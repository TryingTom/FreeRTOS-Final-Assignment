# FreeRTOS-Final-Assignment

Simple FreeRTOS application made as a final assigment to real-time programming course.

The program displays time, date and temperature on LCD screen.
It uses FreeRTOS to make everything work real-time and work alongside each other.

The used LCD screen has 5 buttons, four directional and 1 select. 

The main screen shows the time, date and temperature.
If up or down is pressed, the user can move to next screen. The main screen will show again if nothing is pressed for 3 seconds.
There are following screens implemented:
 - screen will show minimum and maximum values and their time
 - screen will show average by the hour, showing also the max and minimum for that hour
 - screen will allow you to set time or set date
	- set time screen
	- set date screen
	
There will be also USB connection to another program on the computer, which shows user interface for the different temperatures according to the time.

## Todo:
- [ ] Fixing screen texts
- [ ] Implement change time
- [ ] Implement display date
- [ ] Implement change date
- [ ] Implement calculate average

- [ ] UI for the the data
