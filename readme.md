# erosduino
a wifi/osc interface to the Erostek 312B utilizing a wifi enabled arduino

it is meant to be paired with an osc host, TouchOSC is what it was tested with.  A TouchOSC template is included

while functional and power efficient the lag inherent with a tiny embedded device was limiting and I eventually moved the project to a RaspberryPi/python setup.

the arduino code is a rewrite of the venerate project which has disappeared from github.  it was a port of the perl project erosoutsider