# FreeRTOS-Final-Assignment

Simple FreeRTOS application made as a final assigment to real-time programming course.

Devices used: Arduino Mega 2560 + DFRobot LCD Keypad Shield V1.1 + DS1820.

The program displays time, date and temperature on LCD screen.
It uses FreeRTOS to make everything work real-time and work alongside each other.

Picture of the wiring will be done soon.
Connect GND to ground, VDD to 5 V and DQ to A7 pin. Also connect the DQ pin to VDD using 4,7 kΩ reistor in between (Pull-up resistor). The program uses extrenal power supply from the VDD - it could have used parasite power using only two pins, but for this project, external power supply was chosen due it being more stable.

The used LCD screen has 5 buttons, four directional and 1 select. 

The main screen shows the time, date and temperature.
If up or down is pressed, the user can move to min and max screen. Left and right buttons will show average. The main screen will show again if nothing is pressed for 3 seconds.
There are following screens implemented:
 - screen will show minimum and maximum values and their time
 - screen will show average by the hour, showing also the max and minimum for that hour
 - screen will allow you to set time or set date
	- set time screen
	- set date screen
	
There will be also USB connection to another program on the computer, which shows user interface for the different temperatures according to the time.

## Wiring:

![alt text](https://github.com/TryingTom/FreeRTOS-Final-Assignment/blob/main/Wiring.png?raw=true)

## Todo:
- [x] Fixing screen texts
- [x] Implement change time
- [x] Implement display date
- [x] Implement change date
- [ ] Implement calculate average
- [x] Fix buttons so they have less error

- [x] UI for the the data (The program sends data to Serial, and the user can use whatever UI - like Arduino IDE's plotter.)
- [x] Picture of the wiring
