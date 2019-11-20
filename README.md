# BNO55-RPi4
Minimal example code for reading Euler angles from Adafruit BNO55 Breakout Board using Raspberry Pi 4.
I have basically "converted" (euphemism for largely-copy-pasted) the [existing library for Arduino](https://github.com/adafruit/Adafruit_BNO055) to Raspberry Pi.  
## Why BNO55? 
Because it has a small processor (Cortex-M0) that takes care of sensor fusion for you. Bosch calls this System in Package. Inside this "package", the sensors transfer data to the microconroller in a SPI bus. (See Figure 1 of the datasheet).  
The downside of this setup: The processor, being a relatively slow one, means that Euler angle output is limited to 100 Hz.  
## Why RPi4? 
BNO55 uses I2C clock stretching, and RPi4 is the first board in the family to properly support it. 
