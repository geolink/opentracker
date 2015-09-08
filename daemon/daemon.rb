#!/usr/bin/ruby

require 'socket'
require 'mysql'
require 'process'
require 'time'
require 'sequel'
require 'prowl'

$data_keys = {
  'key' => '([0-9a-zA-Z]+)',
  'timestamp' => '([0-9]{2}\/[0-9]{2}\/[0-9]{2},[0-9]{2}\:[0-9]{2}\:[0-9]{2})',
  'gps_date' => '([0-9]{6})',
  'gps_time' => '([0-9]+)',
  'latitude' => '([0-9\.\-]+)',
  'longitude' => '([0-9\.\-]+)',
  'speed' => '([0-9\.\-]+)',
  'altitude' => '([0-9\.\-]+)',
  'heading' => '([0-9\.\-]+)',
  'hdop' => '([0-9]+)',
  'satellites' => '([0-9]+)',
  'battery_level' => '([0-9\.]+)',
  'ignition_state' => '([01])',
  'engine_running_time' => '([0-9]+)'
}

class OpenTrackerDaemon
  def initialize
    config_file = File.dirname(__FILE__) + "/config/config.rb"

    if !File.exist? config_file
      raise "Config file not found: #{config_file}"
    end

    config = {}
    instance_eval(File.read(config_file)).each do |key, value|
      config[key] = value
    end

    if !config[:db]
      raise "Database connection is not defined in config.rb"
    end

    @db = Sequel.connect(config[:db])

    load_config

    @server = TCPServer.new(@config[:server], @config[:port])

    Process::Sys.setuid @config[:uid]
    Process::Sys.seteuid @config[:gid]

    !@config[:include_key] and puts "WARNING: $include_key == false: authentication disabled!"

    if @config[:timestamp_use] == 'gsm' and !@config[:include_timestamp]
      raise "Error: $timestamp_use == 'gsm' requires $include_timestamp"
    end

    if @config[:timestamp_use] == 'gps' and (!@config[:include_gps_date] || !@config[:include_gps_time])
      raise "Error: $timestamp_use == 'gps' requires $include_gps_date and $include_gps_time"
    end

    if !["server","gsm","gps"].include? @config[:timestamp_use]
      raise "Error: invalid setting for $timestamp_use; valid settings are server, gsm or gps"
    end
  end

  def load_config
    @config = @db[:config].first
  end

  def start_server
    while (session = @server.accept)
      unless session.peeraddr.nil?
        Thread.start do
          input = session.gets

          @config[:show_received] and puts "#{input}"

          load_config

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

    data = parse_data(req)

    if data[:key] and data[:key] == @config[:key]
      @config[:debug] and puts "key match"

      data.delete(:key)

      if @config[:timestamp_use] == 'server'
        ts = Time.now
      elsif @config[:timestamp_use] == 'gsm'
        t = data[:timestamp].match /^([0-9]{2})\/([0-9]{2})\/([0-9]{2}),([0-9]{2}\:[0-9]{2}\:[0-9]{2})$/
        ts = Time.parse "20" + t[1] + "-" + t[2] + "-" + t[3] + " " + t[4]
      elsif @config[:timestamp_use] == 'gps'
        t1 = data[:gps_date].match(/^([0-9]{2})([0-9]{2})([0-9]{2})$/)
        t2 = data[:gps_time].match(/^([0-9]{1,2})([0-9]{2})([0-9]{2})([0-9]{2})$/)
        ts = Time.parse "20" + t1[3] + "-" + t1[2] + "-" + t1[1] + " " + t2[1].rjust(2,'0') + ":" + t2[2] + ":" + t2[3]
      end

      ts = ts.strftime "%Y-%m-%d %H:%M:%S"

      @config[:debug] and puts "timestamp: #{ts}"

      last_row = @db[:log].order(Sequel.desc(:id)).limit(1).first

      distance = sprintf("%.2f",get_distance(last_row[:latitude], last_row[:longitude], data[:latitude], data[:longitude]))

      if @config[:detect_engineoff_movement] and !data[:ignition_state] and !last_row[:ignition_state]
        if distance.to_f >= @config[:engineoff_movement_threshold]
          alert 'Movement',"Moved #{distance} metres with engine off!"

          last_event = @db[:event].order(Sequel(:id)).limit(1).first
          total_distance = sprintf("%.2f",last_event[:total_distance].to_f + distance.to_f)

          @db[:event].insert(:timestamp => ts, :event => 'engine-off-moved', :moved => distance, :moved_total => total_distance)
        end
      end

      if !last_row[:ignition_state] and data[:ignition_state]
        if @config[:event_alerts]
          alert 'Engine','Engine started'
        end

        n = ts.match(/^[0-9]{4}-[0-9]{2}-[0-9]{2} ([0-9]{2}:[0-9]{2}):[0-9]{2}$/)

        time = n[1]

        if @config[:detect_enginestart_athome] and File.exists?(@config[:athome_file]) and File.open(@config[:athome_file],"r").read.strip.to_i == 1
          alert 'Engine','Engine started while at home!'
        end

        if @config[:detect_enginestart_overnight] and time >= @config[:enginestart_overnight_from] and time <= @config[:enginestart_overnight_to]
          alert 'Engine','Engine started overnight!'
        end

        @db[:event].insert(:timestamp => ts, :event => 'engine-started', :moved => distance, :moved_total => distance)

        if @config[:log_journeys]
          @db[:journey].insert(:from_timestamp => ts, :from_latitude => data[:latitude], :from_longitude => data[:longitude])

          journey = @db[:journey].order(Sequel.desc(:id)).limit(1).first

          @db[:journey_step].insert(:journey_id => journey[:id], :timestamp => ts, :latitude => data[:latitude], :longitude => data[:longitude])
        end
      end

      if last_row[:ignition_state] and data[:ignition_state]
        last_event = @db[:event].order(Sequel.desc(:id)).limit(1).first
        total_distance = sprintf("%.2f",last_event[:moved_total].to_f + distance.to_f)

        if @config[:event_alerts]
          alert 'Moving', "Moved #{total_distance}m"
        end

        @db[:event].insert(:timestamp => ts, :event => 'moved', :moved => distance, :moved_total => total_distance)

        if @config[:log_journeys]
          journey = @db[:journey].order(Sequel.desc(:id)).limit(1).first

          @db[:journey_step].insert(:journey_id => journey[:id], :timestamp => ts, :latitude => data[:latitude], :longitude => data[:longitude])
        end
      end

      if last_row[:ignition_state] and !data[:ignition_state]
        last_event = @db[:event].order(Sequel.desc(:id)).limit(1).first
        total_distance = sprintf("%.2f",last_event[:moved_total].to_f + distance.to_f)

        if @config[:event_alerts]
          alert 'Engine', 'Engine stopped'
        end

        @db[:event].insert(:timestamp => ts, :event => 'engine-stopped', :moved => distance, :moved_total => total_distance)

        if @config[:log_journeys]
          journey = @db[:journey].order(Sequel.desc(:id)).limit(1).first

          @db[:journey_step].insert(:journey_id => journey[:id], :timestamp => ts, :latitude => data[:latitude], :longitude => data[:longitude])
          @db[:journey].where('id = ?',journey[:id]).update(:to_timestamp => ts, :to_latitude => data[:latitude], :to_longitude => data[:longitude])
        end
      end

      data[:timestamp] = ts
      data[:ip] = ipaddr

      @db[:log].insert(data)
    else
      @config[:debug] and puts "Error: key '#{m[1]}' != #{@config[:key]}"
    end

  end

  def alert(type, msg)
    if @config[:prowl_api_key]
      Prowl.add({
        :apikey => @config[:prowl_api_key],
        :application => 'Tracker',
        :event => type,
        :description => msg
      })
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

  def parse_data(request)
    keys = []
    regex_seg = []

    $data_keys.each do |key, regex|
      if @config["include_#{key}".to_sym]
        keys.push key
        regex_seg.push regex
      end
    end

    @regex = '\A' + regex_seg.join(',')

    m = request.match Regexp.new(@regex)

    data = {}

    for i in 0...keys.length
      if m[i+1].match /\A[0-9]+\z/
        data[keys[i].to_sym] = m[i+1].to_i
      elsif m[i+1].match /\A[0-9\.]+\z/
        data[keys[i].to_sym] = m[i+1].to_f
      else
        data[keys[i].to_sym] = m[i+1]
      end

      if keys[i] == 'speed'
        data[:speed] = (data[:speed] * 0.621371).round(2)
      elsif keys[i] == 'ignition_state'
        data[:ignition_state] = (data[:ignition_state] == 1)
      end
    end

    data
  end
end

ot = OpenTrackerDaemon.new
ot.start_server
