# Moscow-80 CanSat 2018 source code

Raspberry Pi v.3
GY-80
u-blox NEO7-M

## Installation

 make receiver80 - for receiver station
 sudo make install -- to use autorun feature on satellite
 (make install low - for 433 MHz)
 make cansat80 - to run manually

## Usage
./cansat80 [h] -- 866 MHz band
./cansat80 l -- 433 MHz band

./receiver80 [b] -- both bands
h, l -- cf. above

## Files
cansat18.sh -- init.d script
install.sh -- installing init.d script
