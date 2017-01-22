# atmega328p-timer
The hardware timer for UV exposition.
Timer setpoint can be in the range 1 second to 999 minutes and 59 seconds.

You need the [Arduino](https://www.arduino.cc/) to build and upload this firmware.
This sketch uses the [TimerOne](http://playground.arduino.cc/Code/Timer1) library.

![The electrical circuit](https://github.com/kolod/atmega328p-timer/blob/master/hardware/timer.dch.png)
The electrical circuit of the timer.

You need the [DipTrace](http://diptrace.com/) to edit schematics or PCB designing.

## Buttons:

* S1 - Minus. Decrement value in the cursor position.
* S2 - Plus. Increment value in the cursor position.
* S3 - Start/Resume.
* S4 - Move the cursor left.
* S5 - Move the cursor right.
* S6 - Stop/pause timer. When timer stopped long press of this button saves current time setpoint.