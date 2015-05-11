#!/usr/bin/ruby
require 'socket'
require 'mysql'
require 'process'

key = "password"
server = "127.0.0.1"
port = 80
mysql_host = "127.0.0.1"
mysql_user = "root"
mysql_pass = ""
mysql_db = "tracker"
user = 65534
group = 65534

server = TCPServer.new(server, port)

Process::Sys.setuid user
Process::Sys.seteuid group

while (session = server.accept)
    Thread.start do
        input = session.gets

        m = input.match(/^([0-9a-zA-Z]+),([0-9\.\-]+),([0-9\.\-]+),([0-9\.]+),([0-9\.]+),([01]),([0-9]+)/);
        if m[1] == key
            con = Mysql.new mysql_host, mysql_user, mysql_pass, mysql_db

            ts = Time.now.strftime("%Y-%m-%d %H:%M:%S");

            con.query("INSERT into `log` (timestamp,lat,lon,speed,altitude,heading,vehicleBat,engineStatus,runningTime,ip) values ('#{ts}','#{m[2]}','#{m[3]}','#{m[4]}','0','0','#{m[5]}','#{m[6]}','#{m[7]}','#{session.peeraddr[2]}');");
        end
        session.close
    end
end
