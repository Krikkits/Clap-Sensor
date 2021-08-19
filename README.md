# Clap-Sensor
These are the source code for a home made clap sensor using the Arduino IDE. Recommended for Arduino boards, specifically Arduino UNO, since that is the board some of the code is tailored for.

Both files follow the same constraints, but two approaches are available for how it is converted to reflect dB values:

1. direct mapping through calibration, basic arithmetic used to change voltage values to dB
2. using RMS and dividing the sound values with RMS to reflect how many dBs louder a sound is. Unfortunately, this approach is not very functional, the values are rather small, rendering the sensor insensitive
