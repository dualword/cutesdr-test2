#
# Example script for test the 'Bent pipe' concept.
# Alas it is not fully working because the I/Q data messages coming out from the radio
# have to be redirected into a separate UDP pipe.
#
# 
#

interface="/dev/ttyUSB0"

echo "Setting the $interface to 230400 b/s......"
stty -F $interface 230400 cs8 -cstopb -parity -icanon min 1 time 1

echo "Listening on tcp/50000, please start CuteSDR......"
socat tcp-l:50000,reuseaddr,fork file:$interface,nonblock,waitlock=./tty0.lock

