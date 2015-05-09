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
