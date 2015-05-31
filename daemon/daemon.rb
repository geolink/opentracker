#!/usr/bin/ruby

require 'socket'
require 'mysql'
require 'process'
require 'time'
require 'cgi'
require 'net/http'
require 'net/https'
require 'uri'

load './daemon_config.rb'

class OpenTrackerDaemon
    def initialize
        @server = TCPServer.new($server, $port)

        Process::Sys.setuid $user
        Process::Sys.seteuid $group

        !$include_key and puts "WARNING: $include_key == false: authentication disabled!"

        if $timestamp_use == 'gsm' and !$include_timestamp
            puts "Error: $timestamp_use == 'gsm' requires $include_timestamp"
            exit
        end
        if $timestamp_use == 'gps' and (!$include_gps_date || !$include_gps_time)
            puts "Error: $timestamp_use == 'gps' requires $include_gps_date and $include_gps_time"
            exit
        end
        if !["server","gsm","gps"].include? $timestamp_use
            puts "Error: invalid setting for $timestamp_use; valid settings are server, gsm or gps"
            exit
        end

        @segments, @attributes = determine_attributes
        @regex = "^" + @segments.join(",")
    end

    def start_server
        while (session = @server.accept)
            unless session.peeraddr.nil?
                Thread.start do
                    input = session.gets

                    $show_received and puts "#{input}"

                    load './daemon_config.rb'

                    handle_request input, session.peeraddr[2]

                    session.close
                end
            end
        end
    end

    def handle_request(req, ipaddr)
        if req == nil
            return
        end

        m = req.match Regexp.new(@regex)

        if m != nil and m[1] and (!$include_key || m[1] == $key)
            $debug and puts "key match"

            if $timestamp_use == 'server'
                ts = Time.now
            elsif $timestamp_use == 'gsm'
                t = m[key_position("timestamp")].match /^([0-9]{2})\/([0-9]{2})\/([0-9]{2}),([0-9]{2}\:[0-9]{2}\:[0-9]{2})$/
                ts = Time.parse "20" + t[1] + "-" + t[2] + "-" + t[3] + " " + t[4]
            elsif $timestamp_use == 'gps'
                t1 = m[key_position("gps_date")].match(/^([0-9]{2})([0-9]{2})([0-9]{2})$/)
                t2 = m[key_position("gps_time")].match(/^([0-9]{1,2})([0-9]{2})([0-9]{2})([0-9]{2})$/)
                ts = Time.parse "20" + t1[3] + "-" + t1[2] + "-" + t1[1] + " " + t2[1].rjust(2,'0') + ":" + t2[2] + ":" + t2[3]
            end

            ts = ts.strftime "%Y-%m-%d %H:%M:%S"

            $debug and puts "timestamp: #{ts}"

            values = []
            attributes = @attributes.clone

            for i in 0...attributes.length
                if attributes[i] == 'speed'
                    values.push (m[key_position("speed")].to_f * 0.621371).round(2)
                elsif attributes[i] != 'key'
                    values.push m[key_position(attributes[i])]
                end
            end

            con = Mysql.new $mysql_host, $mysql_user, $mysql_pass, $mysql_db

            last_row = query(con, "select * from `log` order by id desc limit 1")

            if $detect_engineoff_movement and m[key_position("ignition_state")].to_i == 0 and last_row["ignition_state"].to_i == 0
                distance = sprintf("%.2f",get_distance(last_row['latitude'], last_row['longitude'], m[key_position("latitude")], m[key_position("longitude")]))

                if distance.to_f >= $engineoff_movement_threshold
                    alert('Movement',"Moved #{distance} metres with engine off!")

                    last_event = query(con, "select * from `event` order by id desc limit 1")
                    total_distance = sprintf("%.2f",last_event["total_distance"].to_f + distance.to_f)

                    con.query("insert into `event` (`timestamp`,`event`,`moved`,`moved_total`) values ('#{ts}','engine-off-moved','#{distance}','#{total_distance}');")
                end
            end

            if last_row["ignition_state"].to_i == 0 and m[key_position("ignition_state")].to_i == 1
                n = ts.match(/^[0-9]{4}-[0-9]{2}-[0-9]{2} ([0-9]{2}:[0-9]{2}):[0-9]{2}$/)

                time = n[1]

                if $detect_enginestart_athome and File.exists?($athome_file) and File.open($athome_file,"r").read.strip.to_i == 1
                    alert('Engine','Engine started while at home!')
                end

                if $detect_enginestart_overnight and time >= $enginestart_overnight_from and time <= $enginestart_overnight_to
                    alert('Engine','Engine started overnight!')
                end

                puts "insert into `event` (`timestamp`,`event`,`moved`,`moved_total`) values ('#{ts}','engine-started','#{distance}','#{distance}');"

                con.query("insert into `event` (`timestamp`,`event`,`moved`,`moved_total`) values ('#{ts}','engine-started','#{distance}','#{distance}');")

                if $log_journeys
                    con.query("insert into `journey` (`from_timestamp`,`from_latitude`,`from_longitude`) values ('#{ts}','#{m[key_position("latitude")]}','#{m[key_position("longitude")]}')")
                    journey = query(con, "select * from `journey` order by id desc limit 1")

                    con.query("insert into `journey_step` (`journey_id`,`timestamp`,`latitude`,`longitude`) values ('#{journey["id"]}','#{ts}','#{m[key_position("latitude")]}','#{m[key_position("longitude")]}')")
                end
            end

            if last_row["ignition_state"].to_i == 1 and m[key_position("ignition_state")].to_i == 1
                last_event = query(con, "select * from `event` order by id desc limit 1")
                total_distance = sprintf("%.2f",last_event["moved_total"].to_f + distance.to_f)

                con.query("insert into `event` (`timestamp`,`event`,`moved`,`moved_total`) values ('#{ts}','moved','#{distance}','#{total_distance}');")

                if $log_journeys
                    journey = query(con, "select * from `journey` order by id desc limit 1")

                    con.query("insert into `journey_step` (`journey_id`,`timestamp`,`latitude`,`longitude`) values ('#{journey["id"]}','#{ts}','#{m[key_position("latitude")]}','#{m[key_position("longitude")]}')")
                end
            end

            if last_row["ignition_state"].to_i == 1 and m[key_position("ignition_state")].to_i == 0
                last_event = query(con, "select * from `event` order by id desc limit 1")
                total_distance = sprintf("%.2f",last_event["moved_total"].to_f + distance.to_f)

                con.query("insert into `event` (`timestamp`,`event`,`moved`,`moved_total`) values ('#{ts}','engine-stopped','#{distance}','#{total_distance}');")

                if $log_journeys
                    journey = query(con, "select * from `journey` order by id desc limit 1")

                    con.query("insert into `journey_step` (`journey_id`,`timestamp`,`latitude`,`longitude`) values ('#{journey["id"]}','#{ts}','#{m[key_position("latitude")]}','#{m[key_position("longitude")]}')")
                    con.query("update journey set to_timestamp = '#{ts}', to_latitude = '#{m[key_position("latitude")]}', to_longitude = '#{m[key_position("longitude")]}' where id = #{journey["id"]}")
                end
            end

            $debug and puts values.inspect

            attributes.shift

            $debug and puts attributes.inspect

            query = "INSERT into `log` (timestamp," + attributes.join(",") + ",ip) values ('" + ts + "'," + values.join(",") + ",'#{ipaddr}');"

            $debug and puts "#{query}"

            con.query(query)
            con.close
        else
            $debug and puts "Error: key '#{m[1]}' != #{$key}"
        end

    end

    def query(con, sql)
        res = con.query(sql)

        row = {}

        if res
            rowdata = res.fetch_row
            fields = res.fetch_fields

            for i in 0...fields.length
                row[fields[i].name] = rowdata[i]
            end
        end

        row
    end

    def alert(type, msg)
        if $prowl_api_key
            msg = URI::escape(msg)
            url = URI.parse("https://prowl.weks.net/publicapi/add?apikey=#{$prowl_api_key}&application=Tracker&event=#{type}&description=#{msg}&priority=2")
            http = Net::HTTP.new(url.host, url.port)
            http.use_ssl = true
            request = Net::HTTP::Get.new(url.path)
            response = http.start {|http| http.request(request) }
        end
    end

    def get_distance(latitudeFrom, longitudeFrom, latitudeTo, longitudeTo, earthRadius = 6371000)
        # convert from degrees to radians
        latFrom = deg2rad(latitudeFrom)
        lonFrom = deg2rad(longitudeFrom)
        latTo = deg2rad(latitudeTo)
        lonTo = deg2rad(longitudeTo)

        latDelta = latTo - latFrom
        lonDelta = lonTo - lonFrom

        angle = 2 * Math::asin(Math::sqrt(Math::sin(latDelta / 2) ** 2) + Math::cos(latFrom) * Math::cos(latTo) * (Math::sin(lonDelta / 2) ** 2))

        angle * earthRadius
    end

    def deg2rad(deg)
        deg.to_f * Math::PI / 180
    end

    def determine_attributes
        segments = []
        attributes = []

        if $include_key
            segments.push '([0-9a-zA-Z]+)'
            attributes.push 'key'
        end
        if $include_timestamp
            segments.push '([0-9]{2}\/[0-9]{2}\/[0-9]{2},[0-9]{2}\:[0-9]{2}\:[0-9]{2})'
            attributes.push 'timestamp'
        end
        if $include_gps_date
            segments.push '([0-9]{6})'
            attributes.push 'gps_date'
        end
        if $include_gps_time
            segments.push '([0-9]+)'
            attributes.push 'gps_time'
        end
        if $include_latitude
            segments.push '([0-9\.\-]+)'
            attributes.push 'latitude'
        end
        if $include_longitude
            segments.push '([0-9\.\-]+)'
            attributes.push 'longitude'
        end
        if $include_speed
            segments.push '([0-9\.\-]+)'
            attributes.push 'speed'
        end
        if $include_altitude
            segments.push '([0-9\.\-]+)'
            attributes.push 'altitude'
        end
        if $include_heading
            segments.push '([0-9\.\-]+)'
            attributes.push 'heading'
        end
        if $include_hdop
            segments.push '([0-9]+)'
            attributes.push 'hdop'
        end
        if $include_satellites
            segments.push '([0-9]+)'
            attributes.push 'satellites'
        end
        if $include_battery_level
            segments.push '([0-9\.]+)'
            attributes.push 'battery_level'
        end
        if $include_ignition_state
            segments.push '([01])'
            attributes.push 'ignition_state'
        end
        if $include_engine_running_time
            segments.push '([0-9]+)'
            attributes.push 'running_time'
        end

        return [segments, attributes]
    end

    def key_position(attribute)
        for i in 0...@attributes.length
            if @attributes[i] == attribute
                return i+1
            end
        end
    end
end

ot = OpenTrackerDaemon.new
ot.start_server
