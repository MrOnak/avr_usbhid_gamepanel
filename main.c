/**
 * A custom controller USB-Human Interface Device.
 *
 * Author: Dominique Stender <dst@st-webdevelopment.com>
 *
 * This device registers itself on your computer as Keyboard/Joystick
 * combination. The keyboard supports up to 6 simultaneous key presses.
 * At the moment, modifier keys are not supported but might come
 * in a later release, if I need them.
 *
 * The joystick registers itself with 6 analogue axis and 4 buttons.
 * One missing feature is a calibration of the analogue axis. There also
 * seems to be a bit of jitter but I haven't had any problems with it
 * as of now so that will be fixed when it becomes an issue.
 *
 * This work is published under GNU GPL v3 (see License.txt)
 *
 * This project is based on the "AVR ATtiny USB Tutorial"
 *    at http://codeandlife.com/ by Joonas Pihlajamaa, joonas.pihlajamaa@iki.fi
 * which in turn is based on V-USB example code by Christian Starkjohann
 *    Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 *
 * It would not have been possible to complete this work without their
 * previous work or the help from the community on the VUSB forums 
 *    at http://forums.obdev.at
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include "usbdrv.h"
#include "usbkeycodes.h"
#include "gamejoystick.h"
#include "gamekeys.h"

// ************************
// *** USB HID ROUTINES ***
// ************************
PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	//---- keyboard (65 byte)  ------------------------------------------------
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xa1, 0x01, // COLLECTION (Application)
    0x85, 0x01, //   REPORT_ID (1)
    0x75, 0x01, //   REPORT_SIZE (1)
    0x95, 0x08, //   REPORT_COUNT (8)
    0x05, 0x07, //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)(224)
    0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
	0x25, 0x01, //   LOGICAL_MAXIMUM (1)
    0x81, 0x02, //   INPUT (Data,Var,Abs) ; Modifier byte
    0x95, 0x01, //   REPORT_COUNT (1)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x81, 0x03, //   INPUT (Cnst,Var,Abs) ; Reserved byte
	0x95, 0x05, //   REPORT_COUNT (5)
    0x75, 0x01, //   REPORT_SIZE (1)
    0x05, 0x08, //   USAGE_PAGE (LEDs)
    0x19, 0x01, //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05, //   USAGE_MAXIMUM (Kana)
	0x91, 0x02, //   OUTPUT (Data,Var,Abs) ; LED report
    0x95, 0x01, //   REPORT_COUNT (1)
    0x75, 0x03, //   REPORT_SIZE (3)
    0x91, 0x03, //   OUTPUT (Cnst,Var,Abs) ; LED report padding
    0x95, 0x06, //   REPORT_COUNT (6)
	0x75, 0x08, //   REPORT_SIZE (8)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0x65, //   LOGICAL_MAXIMUM (101)
    0x05, 0x07, //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))(0)
	0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)(101)
    0x81, 0x00, //   INPUT (Data,Ary,Abs)
    0xc0, // END_COLLECTION
	//---- joystick (84 byte) ------------------------------------------------
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x02,                    //   REPORT_ID (2)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //     END_COLLECTION
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x09, 0x32,                    //     USAGE (Z)
	0x09, 0x33,                    //     USAGE (Rx)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //     END_COLLECTION
    0x09, 0x34,                    //   USAGE (Ry)
    0x09, 0x35,                    //   USAGE (Rz)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x95, 0x02,                    //   REPORT_COUNT (2)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x04,                    //   USAGE_MAXIMUM (Button 4)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x04,                    //   REPORT_COUNT (4)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x75, 0x04,                    //   REPORT_SIZE (4)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x01,                    //   INPUT (Cnst,Ary,Abs)
    0xc0                           // END_COLLECTION
};

typedef struct {
	uint8_t reportId;
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t rx;
	uint8_t ry;
	uint8_t rz;
	uint8_t buttons;
} joystick_report_t;

typedef struct {
	uint8_t reportId;
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[6];
} keyboard_report_t;

static joystick_report_t joystickReport;
volatile uchar joystickUpdateNeeded 	= 0;// flag whether or not to send the joystick report
volatile uchar analogChannel 			= PIN_ANALOG_X;	// "iterator" for ISP(ADC_vect)

static keyboard_report_t keyboardReport;
static uchar idleRate; // repeat rate
volatile uchar keyboardUpdateNeeded 	= 0;// flag whether or not to send the keyboard report

static unsigned char protocolVersion	= 0; 

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (void *) data;

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        switch (rq->bRequest) {
            case USBRQ_HID_GET_REPORT: 
			// check report ID requested
			if (rq->wValue.bytes[0] == 1) {
				usbMsgPtr = (void *) &keyboardReport;
				keyboardReport.modifier 	= 0;
				keyboardReport.keycode[0] 	= 0;
				return sizeof(keyboardReport);
			} else if (rq->wValue.bytes[0] == 2) {
				usbMsgPtr = (void *) &joystickReport;
				return sizeof(joystickReport);
			}			
			
            case USBRQ_HID_SET_REPORT: // if wLength == 1, should be LED state
                return (rq->wLength.word == 1) ? USB_NO_MSG : 0;
            
			case USBRQ_HID_GET_IDLE: // send idle rate to PC as required by spec
                usbMsgPtr = &idleRate;
                return 1;
            
			case USBRQ_HID_SET_IDLE: // save idle rate as required by spec
                idleRate = rq->wValue.bytes[1];
                return 0;
		
			case USBRQ_HID_GET_PROTOCOL:
				usbMsgPtr = &protocolVersion;
				return 1;
				
			case USBRQ_HID_SET_PROTOCOL:
				protocolVersion = rq->wValue.bytes[1];
				return 0;
        }
    }

    return 0; // by default don't return any data
}

usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len) {
    return len;
}

/**
 * reads four digital inputs and updates the values of the joystick buttons
 *
 * returns 1 if any of the buttons have changed state, 0 otherwise
 */
uchar readButtons() {
	uchar retval 	= 0;
	uchar bit 		= 0;
	// Button 1
	if (PORT_IN_JBUTTON_1 & (1 << PIN_JBUTTON_1)) {
		if (!(joystickReport.buttons & (1 << bit))) {
			retval = 1;
		}
		joystickReport.buttons |= (1 << bit);
	} else {
		if (joystickReport.buttons & (1 << bit)) {
			retval = 1;
		}
		joystickReport.buttons &= ~(1 << bit);
	}
	// Button 2
	bit = 1;
	if (PORT_IN_JBUTTON_2 & (1 << PIN_JBUTTON_2)) {
		if (!(joystickReport.buttons & (1 << bit))) {
			retval = 1;
		}
		joystickReport.buttons |= (1 << bit);
	} else {
		if (joystickReport.buttons & (1 << bit)) {
			retval = 1;
		}
		joystickReport.buttons &= ~(1 << bit);
	}
	// Button 3
	bit = 2;
	if (PORT_IN_JBUTTON_3 & (1 << PIN_JBUTTON_3)) {
		if (!(joystickReport.buttons & (1 << bit))) {
			retval = 1;
		}
		joystickReport.buttons |= (1 << bit);
	} else {
		if (joystickReport.buttons & (1 << bit)) {
			retval = 1;
		}
		joystickReport.buttons &= ~(1 << bit);
	}
	// Button 4
	bit = 3;
	if (PORT_IN_JBUTTON_4 & (1 << PIN_JBUTTON_4)) {
		if (!(joystickReport.buttons & (1 << bit))) {
			retval = 1;
		}
		joystickReport.buttons |= (1 << bit);
	} else {
		if (joystickReport.buttons & (1 << bit)) {
			retval = 1;
		}
		joystickReport.buttons &= ~(1 << bit);
	}

	return retval;
}

/**
 * Reads data from 74HCT165N shift-registers to save GPIO pins.
 * 
 * Updates the keyboard key data for the USB report.
 *
 * will return 1 if any of the keys have changed, 0 otherwise.
 */
uchar readKeys() {
    uchar i;
	uchar offset = 0;
	uchar retval = 0;
	
    // Trigger a parallel load to latch the state of the data lines
    PORT_SHIFTREG &= ~(1 << PIN_SHLD);
	// the SN74HCT165N is fast enough so we don't need a delay, this saves a few bytes
    //_delay_us(PULSE_WIDTH_USEC);
    PORT_SHIFTREG |= (1 << PIN_SHLD);

    // Loop to read each bit value from the serial out line of the shift register
	// we forge the keyboard report directly, to safe some memory
    for (i = 0; i < NUMBER_OF_KEYS; i++) {
		
		if (PORT_IN_SHIFTREG & (1 << PIN_QH)) {
			// button is pressed
			if (keyboardReport.keycode[offset] != keyCodes[i]) {
				// button wasn't pressed before or another button was pressed at this offset
				keyboardReport.keycode[offset] = keyCodes[i];
				offset++; 	
				retval = 1;
			} else {
				// same button is still pressed, don't trigger an USB update but increase the offset
				offset++;
			}
		} else {
			// button is not pressed
			if (keyboardReport.keycode[offset] == keyCodes[i]) {
				// previously there was THIS button pressed at this offset, unset
				keyboardReport.keycode[offset] = KEY_NONE;
				// no need to increase the offset, this key-slot can be re-used
				retval = 1;
			}
		}

        // Pulse the clock (rising edge shifts the next bit).
        PORT_SHIFTREG |= (1 << PIN_CLK);
		// the SN74HCT165N is fast enough so we don't need a delay, this saves a few bytes
        //_delay_us(PULSE_WIDTH_USEC);
        PORT_SHIFTREG &= ~(1 << PIN_CLK);
		
		if (offset >= 6) {
			break;
		}
    }
	
	/*
		we've gone through all shift registers. make sure all leftover key-slots are reset
		
		this is necessary for key sequences like this: none -> A -> A && B -> B -> none
		the resulting keycode sequences would be:
			00 00 00 00 00 00 00 00			none
			00 00 04 00 00 00 00 00			A
			00 00 04 05 00 00 00 00			A && B
			00 00 05 05 00 00 00 00 		B 		note the stale B on 2nd position.
			00 00 00 05 00 00 00 00			none	stale B is still there
			
			so this loop cleans that up
	*/
	for (i = offset; i < 6; i++) {
		keyboardReport.keycode[i] = KEY_NONE;
	}
	
	return retval;	
}

void init(void) {
    //---- clear reports initially --------------------------------------------
	keyboardReport.reportId = 1;
	keyboardReport.modifier = 0;
	keyboardReport.reserved = 0;
	keyboardReport.keycode[0] = KEY_NONE;
	keyboardReport.keycode[1] = KEY_NONE;
	keyboardReport.keycode[2] = KEY_NONE;
	keyboardReport.keycode[3] = KEY_NONE;
	keyboardReport.keycode[4] = KEY_NONE;
	keyboardReport.keycode[5] = KEY_NONE;

	joystickReport.reportId = 2;
	joystickReport.x 		= 127;
	joystickReport.y 		= 127;
	joystickReport.z 		= 127;
	joystickReport.rx 		= 127;
	joystickReport.ry 		= 127;
	joystickReport.rz 		= 127;
	joystickReport.buttons 	= 0;

	//---- digital joystick buttons -------------------------------------------
	REGISTER_JBUTTON_1 &= ~(1 << PIN_JBUTTON_1);
	REGISTER_JBUTTON_2 &= ~(1 << PIN_JBUTTON_2);
	REGISTER_JBUTTON_3 &= ~(1 << PIN_JBUTTON_3);
	REGISTER_JBUTTON_4 &= ~(1 << PIN_JBUTTON_4);
	
	//---- configure the ADC for analog joystick axis -------------------------
	// configure analog inputs
	REGISTER_ANALOG &= ~(1 << PIN_ANALOG_X); 
	REGISTER_ANALOG &= ~(1 << PIN_ANALOG_Y); 
	REGISTER_ANALOG &= ~(1 << PIN_ANALOG_Z);
	REGISTER_ANALOG &= ~(1 << PIN_ANALOG_RX); 
	REGISTER_ANALOG &= ~(1 << PIN_ANALOG_RY); 
	REGISTER_ANALOG &= ~(1 << PIN_ANALOG_RZ);
	// select ADC0 channel as first one to read
	ADMUX = 0x00;	// select ADC0;
	ADMUX |= (1 << REFS0);	// voltage reference to AREF pin
	ADMUX |= (1 << ADLAR);	// left align the ADC value - have the 8 highest bits in a single register 
	// prescaling to 128 - 125kHz at 16MHz clock speed
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	// free-running mode
	ADCSRA |= (1 << ADATE);
	ADCSRB = 0;
	
	ADCSRA |= (1 << ADEN);	// enable the ADC
	ADCSRA |= (1 << ADIE);	// enable ADC interrupt
	ADCSRA |= (1 << ADSC);	// start ADC conversions 

    //---- keyboard -----------------------------------------------------------
	// configure output of 74HTC165N shift register as input
    REGISTER_SHIFTREG &= ~(1 << PIN_QH);
    // clock and shift/load pins as output
    REGISTER_SHIFTREG |= (1 << PIN_CLK);
    REGISTER_SHIFTREG |= (1 << PIN_SHLD);

    // set shift/load high to prevent reading of data
    PORT_SHIFTREG |= (1 << PIN_SHLD);
    // set clock pin low
    PORT_SHIFTREG &= ~(1 << PIN_CLK);
	
	// enable watchdog - 1 sec
    wdt_enable(WDTO_1S);
}

int main() {
	// configure pins, set up ADC ...
	init();
	usbInit();
	
    usbDeviceDisconnect(); // enforce re-enumeration
	// wait 500 ms, watchdog is set to 1 sec, so no wdt_reset() calls are required
	_delay_ms(500);	
    usbDeviceConnect();

    sei(); // Enable interrupts after re-enumeration

    while (1) {
        usbPoll(); 
		wdt_reset(); // keep the watchdog happy
		
		keyboardUpdateNeeded |= readKeys();			// keyboard

        if (keyboardUpdateNeeded) {
			while (!usbInterruptIsReady()) {usbPoll();}	
            usbSetInterrupt((void *) &keyboardReport, 8);
			while ( !usbInterruptIsReady() ) {usbPoll();}
            usbSetInterrupt((void *) &keyboardReport + 8, sizeof(keyboardReport) - 8);
			
			keyboardUpdateNeeded = 0;
        }
        joystickUpdateNeeded |= readButtons();		// joystick buttons
													// joystick analog axis are being read in ISR(ADC_vect)
        if (joystickUpdateNeeded) {
			while (!usbInterruptIsReady()) {usbPoll();}	
            usbSetInterrupt((void *) &joystickReport, sizeof(joystickReport));
			/**
			 * no idea why I need to send this byte	but if I don't,
			 * the OS won't register the device.
			 */
			while (!usbInterruptIsReady()) {usbPoll();}	
			usbSetInterrupt(0x00, 1);	
			
			joystickUpdateNeeded = 0;
        }
    }

    return 0;
}

/**
 * Analog-Digital converter interrupt. 
 *
 * Used for the six analog axis of the joystick.
 *
 */
ISR(ADC_vect) {
	// back up the interrupt register
	uchar cSREG = SREG;
	// disable interrupts for the time being
	cli();
	
	uchar ADCval = ADCH;
	// assign ADC value to correct channel
	switch (analogChannel) {
		case PIN_ANALOG_X:
			if (joystickReport.x != ADCval) {joystickUpdateNeeded = 1;}
			joystickReport.x = ADCval;
			ADMUX++; 						// switch to next ADC channel
			analogChannel = PIN_ANALOG_Y;	// select next channel
			break;
			
		case PIN_ANALOG_Y:
			if (joystickReport.y != ADCval) {joystickUpdateNeeded = 1;}
			joystickReport.y = ADCval;
			ADMUX++; 						// switch to next ADC channel
			analogChannel = PIN_ANALOG_Z;	// select next channel
			break;
			
		case PIN_ANALOG_Z:
			if (joystickReport.z != ADCval) {joystickUpdateNeeded = 1;}
			joystickReport.z = ADCval;
			ADMUX++; 						// switch to next ADC channel
			analogChannel = PIN_ANALOG_RX;	// select next channel
			break;
			
		case PIN_ANALOG_RX:
			if (joystickReport.rx != ADCval) {joystickUpdateNeeded = 1;}
			joystickReport.rx = ADCval;
			ADMUX++; 						// switch to next ADC channel
			analogChannel = PIN_ANALOG_RY;	// select next channel
			break;
			
		case PIN_ANALOG_RY:
			if (joystickReport.ry != ADCval) {joystickUpdateNeeded = 1;}
			joystickReport.ry = ADCval;
			ADMUX++; 						// switch to next ADC channel
			analogChannel = PIN_ANALOG_RZ;	// select next channel
			break;
			
		case PIN_ANALOG_RZ:
			if (joystickReport.rz != ADCval) {joystickUpdateNeeded = 1;}
			joystickReport.rz = ADCval;
			ADMUX &= 0xf8; 					// reset to ADC channel 0
			analogChannel = PIN_ANALOG_X;	// select first channel
			break;
			
		default:
			ADMUX &= 0xf8; 					// reset to ADC channel 0
			analogChannel = PIN_ANALOG_X;	// select first channel
	}	
	// restore interrupt register
	SREG = cSREG;
}

