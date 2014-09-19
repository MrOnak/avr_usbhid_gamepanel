// defines the vars for the shift registers which are used
// to read in the keys for the keyboard
#define REGISTER_SHIFTREG	DDRB
#define PORT_SHIFTREG		PORTB
#define PORT_IN_SHIFTREG	PINB
#define PIN_CLK				PB2
#define PIN_SHLD			PB3
#define PIN_QH				PB1

/*
 * number of buttons that you have attached
 *
 * this determines how many bits are read from the 
 * shift-registers
 */
#define NUMBER_OF_KEYS 32	

// not used at the moment
#define NUM_LOCK 1
#define CAPS_LOCK 2
#define SCROLL_LOCK 4

/*
 * this defines the button assignment for each bit in the shift registers.
 * the bit read first from the registers must be at index 0 of the array.
 * 
 * for a list of (almost) all USB keycodes see usbkeycodes.h
 */
uchar keyCodes[NUMBER_OF_KEYS] 	 	 = {KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H,
										KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P,
										KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X,
										KEY_Y, KEY_Z, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6};
