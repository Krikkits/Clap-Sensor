# Clap-Sensor
Both files follow the same constraints, but two approaches are available for how it is converted to reflect dB values:

1. direct mapping through calibration, basic arithmetic used to change voltage values to dB
2. using RMS and dividing the sound values with RMS to reflect how many dBs louder a sound is. Unfortunately, this approach is not very functional, the values are rather small, rendering the sensor insensitive
