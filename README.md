This is a project to implement a USB-HID combo device with an Atmel ATMega328p
microcontroller. It should be fairly easy to adapt it to other microcontrollers
from Atmel as well.

## GOAL 
The goal behind the project was to create a custom game panel for flight 
simulators and similar games where you need a lot of buttons which in turn are 
mapped to keyboard keys.

The device is recognized as USB combo device consisting of a keyboard and a 
joystick. The joystick has 6 analog axis and 4 buttons. The keyboard supports
the default 6 simultaneous button presses. I haven't implemented keyboard
modifiers as I'm not using them (yet) but that is a fairly easy exercise to do.

## ACKNOWLEDGEMENTS 
The code is based on the USB HID keyboard tutorial by Joonas Pihlajamaa, 
published at the Code and Life blog, http://codeandlife.com

The usbdrv subfolder contain parts of V-USB library available at 
http://www.obdev.at/avrusb/ and its contents are copyrighted by their 
respective authors. 

Without their combined work and the incredibly helpful people in the V-USB 
forums at http://forums.obdev.at/viewforum.php?f=8 this project would 
probably not have taken flight (excuse the pun). 

## LICENSE 
This code is released under the GNU General Public License, version 3. See 
the License.txt file for details.

## CONFIGURATION 
For a suggestion on a circuit layout, see the avr_gamepanel.png or the 
eagle file in the eagle subdirectory. Also, please read the "A word on the 
eagle file" section further below.

### Circuitry
As you can see in the circuitry, the device that I have built consists
of 36 hardware-debounced buttons (or rather jumpers to connect the switches
to), piped into the ATMega through 74165 shift-registers. You can easily
change that to a more simple design, do the debouncing in software and/or
add more switches by extending the chain of shift-registers.

### Assigning keyboard keys to each switch
See the gamekeys.h file for the pin configuration for the shift registers.

In gamekeys.h you will also find the array `keyCodes` which holds the
mapping of buttons to USB keycodes. 

The `keyCodes` array ordered such that `keyCodes[0]` will be mapped to the
bit that is shifted in first. 

For a list of available USB keycodes, see usbkeycodes.h. Note that the list
is not 100% complete. But since I'm remapping keys in my simulators anyway,
I haven't found it lacking.

### Analogue axis
All analogue input pins of the ATMega328p are assigned to be analogue axis
of the Joystick. If you open the gamejoystick.h file you find all defines 
to configure which input does what.

The inputs are read by an interrupt routine in a round-robin fashion. I'm
sure the code could be shorter / faster but as it is it should be very easy
to modify to your own needs.

One word of advise though: If not all six analogue axis are used, I'd strongly
recommend to either remove code for any axis / any input you're not using 
from the switch-statement in the interrupt routine `ISR(ADC_vect)` or routing
the ADC input pin to ground. I've had extreme floating inputs of axis when 
not all six are connected.

### USB circuitry 
In case you want / have to establish USB connectivity on ports other than 
PD2 and PD3, you will have to change the values for `USB_CFG_IOPORTNAME`,
`USB_CFG_DMINUS_BIT` and `USB_CFG_DPLUS_BIT` in usbconfig.h inside the usbdrv 
subdirectory.

Make sure you change the values in the Makefile for the microcontroller in 
use, your programmer and the frequency that your microcontroller runs at. Make 
sure you double check with the VUSB website at http://www.obdev.at/avrusb/ 
which clock rates are supported (12.000MHz and 16.000MHz are fine, but there 
are others).

## A WORD ON THE EAGLE FILE 
The eagle file was made for clarification purposes. I did NOT build my own
circuit from the eagle file. Instead, the eagle file was done after I had 
already made my device.

**THIS ESSENTIALLY MEANS THAT THE EAGLE FILE IS UNTESTED**

I did my best to verify the schematic and I'm fairly confident that the 
circuitry itself and the values for all parts are correct. But the fact remains
that the schematic hasn't been built as-is. 

**Make sure you do sanity checks on everything during your work.**

Also, if you find issues with the schematic, don't hesitate to contact me
and I'll update the schematic.

## DESIGN DECISIONS 
### Hardware Debouncing
Instead of debouncing the buttons in software, I opted for hardware
debouncing based on the SN74HCT14N inverting Schmitt-Trigger ICs and a RC
low-pass filter for each button. The required components are available 
cheaply and perform very reliably.

That said you can easily implement software debouncing and safe yourself a 
lot of soldering.

### Shift Registers
I wanted the software to be flexible and the hardware expandable. Because of 
that I'm reading the button states through 74HC165 shift registers. You can 
chain any number of shift registers in series to extend the number of buttons 
that your device supports.

### No spamming of the USB port
A bit of logic was implemented to keep the number of USB messages down to
a minimum. The device will send an USB message only when a button is pressed
or released, not when buttons are held down.

