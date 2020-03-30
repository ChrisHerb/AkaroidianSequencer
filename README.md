# Akaroidian Sequencer
by Christopher Herb

This is a 3 channel algorithmic sequencer inspired by the Euclidean algorithm. 
Each channel will generate a sequence (called shape) which is first defined by the assigned length. 
By default it plays a note in the first position but also according to the other 
shapes' length. For instance, if shape 1 gets assigned the length = 5, 
shape 2 = 7 and shape 3 = 13 it will output the following sequences: 

Shape 1: 1 0 0 0 0
Shape 2: 1 0 0 0 0 1 0
Shape 3: 1 0 0 0 0 1 0 1 0 0 0 0 0

These sequences are now free running polyrhythms with the exception that 
channel 1 resets after it has reached the defined measure length. Channel 2 will
reset after 2 bars and channel 3 will reset after 4 bars of this measure. 
The shapes can also be freely rotated.

In Addition, 3 extra channels will ouput sequences derived through complex logic 
from the first 3 channels.

This sketch is a prototype meant to be run on an Arduino with native USB capabilities
(Arduino Leonardo, Arduino Micro).
It will need a Midi clock and start/stop signals to run and can be controlled 
via Midi CC. It ouputs Midi on the following channels: 

Channel 1 = shape 1
Channel 2 = shape 2
Channel 3 = shape 3
Channel 4 = extra logic output 1
Channel 5 = extra logic output 2
Channel 6 = extra logic output 3

The following CC values can be used to control the sequencer:

CC1 = shape 1 length
CC2 = shape 2 length
CC3 = shape 3 length
CC4 = shape 1 rotation
CC5 = shape 2 rotation
CC6 = shape 3 rotation