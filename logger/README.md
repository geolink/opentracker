Basic back-end logging system for OpenTracker by github.com/m4rkw
-----------------------------------------------------------------

Features:

- logs to a mysql database
- also stores events such as "engine started", "engine stopped", etc
- can send alerts to ios devices using Prowl
- detects engine-off movement and alerts (not currently reliable)
- detects engine starts when at home [1]
- detects engine starts overnight

[1] For at-home engine-start detection to work you need to set the contents of a file
to "1" when you are at home and "0" when you are not.  An easy way to do this is to
detect the presence of say, your mobile phone, on the home wifi network and log this
accordingly.
