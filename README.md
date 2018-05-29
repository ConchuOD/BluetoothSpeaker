# BTS - unfinished                 
The v1.0 board has been made to work, albeit with a couple of modifications.
The v2.0 board currently works BT interactions, and the display. Touch isn't working yet, amp is untested however *should* work.
Hopefully 

This repo houses the hardware and code for a bluetooth speaker. It uses an RN52 from Roving Networks on one daughterboard to provide the bluetooth streaming capabilities and a Teensy3.2 from PJRC on another to control operation and power the touchscreen display.
The amplification is provided by a TI 20 Watt Class-D amplifier. An ADI switching controller provides the 3.3 Volt rail and an ON semi "LDO" provides ~10.5 Volts to power the amplifier. The power supply is a 3S LiPo battery I had lying around from a RC project.
