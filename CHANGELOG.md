Changelog

v1.0
====

v2.0
====

v2.0.1
======

v3.0.1
======

v3.0.2
======

+ Renamed tracker.h to tracker.h.example so that tracker.h can be .gitignored (as it tends to contain credentials)
+ Added ALWAYS_ON config option to always log and post data even when the ignition is off.
+ Added options to additionally log and post the ignition state, battery level and total engine running time.
+ Added tracker.php - a basic PHP script that will log POSTed data to a MySQL database and forward the data on to Geolink.
+ Added a "locate" SMS command that responds to the sender with a google maps link. By default this is in the comgooglemaps:// format which will open in the google maps iOS app, but it can be changed to normal https://maps.google links by setting LOCATE_COMMAND_FORMAT_IOS = 0 in tracker.h.
+ Added GSM_SEND_FAILURES_REBOOT options, if set to >0 this determines the number of consecutive GSM send failures that will trigger a reboot of the device.

v3.0.3
======

+ added SEND_RAW mode to send the data packet over a raw TCP connection in order to minimise bandwidth
+ added example daemon.rb ruby tcp server to accept and log the raw data packets
+ added boolean flags for all elements in the data packet so they can be turned on and off individually

Raw mode drastically reduces the bandwidth used by the OpenTracker.

Example old data transmission using HTTP:

POST /index.php HTTP/1.0\r\nHost: some.server.com \r\nContent-type: application/x-www-form-urlencoded\r\nContent-length: 42\r\nConnection: close\r\n\r\nimei=8634241016201447&key=xxxxxxxxx&d=15/05/10,09:40:40 0[100515,9404700,12.394991,-4.132110,0.02,18.70,346.90,67,15]12.24,0,2520#eof

New data packet using raw mode and only a few select data items:

xxxxxxxxx,12.345968,-1.234517,0.04,12.42,0,2880

273 bytes down to 47 bytes
