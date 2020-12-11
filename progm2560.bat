@REM -------------- Arduino mega 2560 programming-----------------------------------------------
@REM 
@REM Makvä 7.2.2013
@REM 
@REM This script programs Arduino with avrdude .
@REM 
@REM 1. Plug Arduino's usb cable to PC. Use the control panel to see COM port number.
@REM 2. The programmin command: (if you use COM8 port and your program file is  demo.hex)
@REM 
@REM       progm2560 COM8 demo.hex
@REM 
@REM 
@REM 
@REM --------------------------------------------------------------------------------------------

@SET ComPort=%1%
@SET file=%2% 

@REM set arduino mega2560 to programmin state with  dtr signal
mode %ComPort%: baud=115200 parity=n data=8 stop=1 to=on xon=off odsr=off octs=off dtr=on rts=off idsr=off

@REM now start to program
avrdude.exe -p m2560 -P %ComPort% -b 115200 -c stk500v2 -U flash:w:%file% -D
@IF %ERRORLEVEL% NEQ 0 GOTO DUDEError

@ECHO.
@ECHO Programming successful!
@PAUSE
@EXIT 0

:DUDEError
@ECHO.
@ECHO Error returned from [avrdude]
@PAUSE
@EXIT 1
