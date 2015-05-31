#!/bin/bash
if [ "`whoami`" != "root" ] ; then
    echo "This needs to run as root (but will drop privileges in daemon.rb)"
    exit 0
fi

while :
do
    ./daemon.rb
done
