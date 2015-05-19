#!/usr/bin/ruby
require 'socket'
require 'mysql'
require 'process'

load './daemon_config.rb'

server = TCPServer.new($server, $port)

Process::Sys.setuid $user
Process::Sys.seteuid $group

while (session = server.accept)
    Thread.start do
        input = session.gets

        puts "#{input}"

        m = input.match(/^([0-9a-zA-Z]+),([0-9\.\-]+),([0-9\.\-]+),([0-9\.\-]+),([0-9\.\-]+),([0-9\.\-]+),([0-9]+),([0-9]+),([0-9\.]+),([01]),([0-9]+)/);
        if m[1] == $key
            con = Mysql.new $mysql_host, $mysql_user, $mysql_pass, $mysql_db

            ts = Time.now.strftime("%Y-%m-%d %H:%M:%S");

            con.query("INSERT into `log` (timestamp,lat,lon,speed,altitude,heading,hdop,satellites,vehicleBat,engineStatus,runningTime,ip) values ('#{ts}','#{m[2]}','#{m[3]}','#{m[4]}','#{m[5]}','#{m[6]}','#{m[7]}','#{m[8]}','#{m[9]}','#{m[10]}','#{m[11]}','#{session.peeraddr[2]}');");
        end
        session.close
    end
end
