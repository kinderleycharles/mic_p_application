/*
 * Lab 8 Part C
 * Name:		Kinderley Charles
 * Section:		6957
 * TA:			Khaled Hassan
 * Description:	The following program reads an input from a remote control
 *				and play different song based on the key that is pressed
 */


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "ebi_driver.h"

#define XMEGA_CLK 2000000						// XMEGA clocks at 2MHz
#define LCD_CMD 0x471000						// Command Address
#define LCD_DATA 0x471001
#define CS0_Start 0x8000
#define CS0_End 0x9FFF
#define CS1_Start 0x470000
#define CS1_End 0x47FFFF
#define true 1
#define false 0


// Function Prototypes
void clearLCD();
void LCD_init();
void sendStr(char*);

// Array that will store the width of every pulse and number of pulse
double keyWidth[10][100];						// Store the width of 100 OR all the pulses for each key
int keyNumPulse[10];							// Store the number of pulses that each remote key generates
int tmpKeyWidth[101];							// Temporary array to store the width of each pulse of current key
int tmpCounter;									// Counter to iterate through tmpKeyWidth
int recordKey;									// Variable to hold which key user want to record

int recordingKey = 0;							// Use to determine whether key is being recorded or not
int doneRecording;								// 1 when recording is done, 0 when false
int keyBeingRecorded = 0;						// Tell which key is being recorded
uint16_t recordedKeyWidth[101];
int buttonRecording = 0;						// Global variable to tell how many number of pulse has been recorded



 /************************************************************************************
 * Name:     Wait
 * Purpose:  Function delay until poll indicate that LCD is ready for instruction
 * Inputs:
 * Output:
 ************************************************************************************/
 void Wait() {
	 while ( __far_mem_read(LCD_CMD) & 0x80 );
 }
 

 /************************************************************************************
 * Name:     EBI_init
 * Purpose:  Function to initialize the desired EBI Ports.  Configures to run IO Port,
 *           SRAM, and LCD.  All CSs and other control signals generate appropriate
 *           enables inside CPLD.
 * Inputs:
 * Output:
 ************************************************************************************/
 void EBI_init()
 {
	 PORTH.DIR = 0x37;       // Enable RE, WE, CS0, CS1, ALE1
	 PORTH.OUT = 0x33;		// Defaults: RE=H, WE=H, CS0=H, CS1=H, ALE1=0
	 PORTK.DIR = 0xFF;       // Enable Address 7:0 (outputs)
	 EBI.CTRL = EBI_SRMODE_ALE1_gc | EBI_IFMODE_3PORT_gc;            // ALE1 multiplexing, 3 port configuration

	 EBI.CS0.BASEADDRH = (uint8_t) (CS0_Start>>16) & 0xFF;
	 EBI.CS0.BASEADDRL = (uint8_t) (CS0_Start>>8) & 0xFF;            // Set CS0 range to 0x008000 - 0x009FFF
	 EBI.CS0.CTRLA = EBI_CS_MODE_SRAM_gc | EBI_CS_ASPACE_8KB_gc;	    // SRAM mode, 8k address space

	 // BASEADDR is 16 bit (word) register. C interface allows you to set low and high parts with 1
	 // instruction instead of the previous two
	 EBI.CS1.BASEADDR = (uint16_t) (CS1_Start>>8) & 0xFFFF;          // Set CS1 range to 0x370000 - 0x37FFFF
	 EBI.CS1.CTRLA = EBI_CS_MODE_SRAM_gc | EBI_CS_ASPACE_64KB_gc;	// SRAM mode, 64k address space
 }
 
 
 /************************************************************************************
 * Name:     KEYPAD_INIT
 * Purpose:  Function to initialize PORT E for the keypad
 * Inputs:
 * Output:
 ************************************************************************************/
 void KEYPAD_init() {
	 PORTE.DIR = 0x0F;						// Set least significant nibble as output and most sig as input
	 // Pull up resistors
	 uint8_t volatile a = 0b00011000;
	 PORTE_PIN7CTRL = a;
	 PORTE_PIN6CTRL = a;
	 PORTE_PIN5CTRL = a;
	 PORTE_PIN4CTRL = a;
 }
 
 
/************************************************************************************
 * Name:     TC_init()
 * Purpose:  Function to configure timer counter of the XMEGA and the pin to be used
 *			 as output
 * Inputs:
 * Output:
 ************************************************************************************/
void TC_init() {
	PORTF_DIR = 0x10;						// Set port F pin 4 as output
	PORTF_OUTSET = 0x10;					// Output 0 to pin 4 in PORTF
	
	
	// Configure Type 1 counter on Port F pin 4 for Note frequency
	TCF1_CTRLA = 0x00;						// Set Prescaler OFF; N = 1
	TCF1_CTRLB = 0x11;						// Set CCA on F0, set mode to FRQ
	TCF1_CTRLC = 0x00;						// To set compare register if diff. compare value used (UNUSED)
	TCF1_CTRLD = 0xA0;						// No capture is being performed for this lab
	TCF1_CTRLE = 0x00;						// Set timer/counter to normal mode
	TCF1_CCA = 574;							// Set CCA to 567 assuming that prescaler 1 is being used
	TCF1_CNT = 0x0000;						// Set counter to 0
	TCF1_CTRLFCLR = 0x00;					// Set counter to imcrementing
	
	
	// Configure PORT C as type 0 timer for note duration
	TCC0_CTRLA = 0x00;						// Set timer counter ON, N = 1
	TCC0_CTRLB = 0x10;						// Set CCA on F1, set timer waveform generation to Normal
	TCC0_CTRLC = 0x00;						// To set compare register if diff. compare value used (UNUSED)
	TCC0_CTRLD = 0xA0;						// No capture is being performed for this lab
	TCC0_CTRLE = 0x00;						// Set timer/counter to normal mode
	TCC0_INTCTRLA = 0x01;					// ERROR interrupt = False, Overflow interrupt = Low priority
	TCC0_PER = 574;							// Set CCA to 567 assuming that prescaler 1 is being used
	TCC0_CNT = 0x0000;						// Set counter to 0
	TCC0_CTRLFCLR = 0x00;					// Set counter to incrementing
	
	
	// Configure PORT C type 1 timer for end of remote key scanned
	TCC1_CTRLA = 0x00;						// Set timer counter ON, N = 1
	TCC1_CTRLB = 0x10;						// Set CCA on F1, set timer waveform generation to Normal
	TCC1_CTRLC = 0x00;						// To set compare register if diff. compare value used (UNUSED)
	TCC1_CTRLD = 0x00;						// No capture is being performed for this lab
	TCC1_CTRLE = 0x00;						// Set timer/counter to normal mode
	TCC1_INTCTRLA = 0x01;					// ERROR interrupt = False, Overflow interrupt = Low priority
	
	TCC1_PER = 9999;						// Set CCA to 567 assuming that prescaler 1 is being used
	TCC1_CNT = 0x0000;						// Set counter to 0
	TCC1_CTRLFCLR = 0x00;					// Set counter to incrementing
	

	// Input capture configuration, need to be set to input capture
	/*
	PORTF_DIRCLR = 0x01;					// Set port F pin 4 as input
	PORTF_PIN4CTRL = 0x00;					// Set pin 4 enable to BOTH EDGES
	EVSYS_CH0MUX = EVSYS_CHMUX_PORTF_PIN0_gc;
	TCF0_CNT = 0;							// Set counter to 0
	TCF0_CCA = 0;
	TCF0_INTFLAGS = 0x10;					// Clear OVERFLOW INTERRUPT FLAG, and CCAIF flag
	TCF0_CTRLE = 0x00;						// Set Byte Mode to normal
	TCF0_CTRLD = 0x2C;						// Set event capture to INPUT CAPTURE and timer even source selection to channel 0
	TCF0_CTRLC = 0x00;						// The compare register is not being used
	TCF0_CTRLB = 0x10;						// Set Capture on CCA and timer to normal mode	
	TCF0_CTRLA = 0x07;						// Turn ON timer
	cli();
	TCF0_INTCTRLB = 0x11;					// Enable LOW level interrupt on CCA
	
	PMIC_CTRL = 0x01;
	*/
	
	
	PORTF_DIRCLR = PIN0_bm;						// Step 1: Set C0 as Input
	PORTF_PIN0CTRL = PORT_ISC_BOTHEDGES_gc;		// Step 2: Set C0 to be triggered on falling edge
	EVSYS_CH0MUX = EVSYS_CHMUX_PORTF_PIN0_gc;	// Step 3: Set C0 as multiplexer input for event channel 0
	TCF0_CTRLD = TC_EVACT_CAPT_gc | TC_EVSEL_CH0_gc; // Step 4: Define event action the timer will perform. Input Capture = CAPT.
	// Select Ch 0 as control source
	TCF0_CTRLB = 0x10;
	TCF0_CTRLA = TC_CLKSEL_DIV1_gc;				// Step 5: Set Prescaler
	
	cli();
	TCF0_INTCTRLB = TC_CCAINTLVL_LO_gc;			// Step 6: Set Interrupt level as low
	PMIC_CTRL = PMIC_LOLVLEN_bm;				// Step 7: Enable low level in PMIC, global interrupt
	sei();
	
	
	TCF0_CNT = 0;								// Step 8: Initializations
	TCF0_CCA = 0;
	
}


/************************************************************************************
 * Name:     LCD_init()
 * Purpose:  Function to configure the LCD
 * Inputs:
 * Output:
 ************************************************************************************/
void LCD_init() {
	Wait();
	__far_mem_write(LCD_CMD, 0x38);// Enable 2 line mode (See LCD Manual)
	Wait();
	__far_mem_write(LCD_CMD,0x0F);	// Display cursor blink
	Wait();
	__far_mem_write(LCD_CMD,0x01);	// Clear Home
	Wait();
	__far_mem_write(LCD_CMD,0x06);	// Turn off shifting in LCD just case name is too long
}

/************************************************************************************
 * Name:     SCAN_KEYPAD()
 * Purpose:  Function determines which key in a keypad was pressed
 * Inputs:	 int
 * Output:
 ************************************************************************************/
 int SCAN_KEYPAD() {
	 uint8_t volatile keys[16] = {1,4,7,14,2,5,8,0,3,6,9,15,10,11,12,13};
	 uint8_t volatile out[4] = {7,11,13,14};
	 
	 uint8_t volatile i;
	 for (i = 0; i != 4; ++i) {
		 PORTE.OUT = out[i];
		 uint8_t volatile tmp = PORTE.IN >> 4;
		 
		 uint8_t volatile j;
		 for(j = 0; j!=4; ++j) {
			 if(tmp == out[j]) {
				 return keys[4*i+j];
			 }
		 }
	 }
	 return -1;
 }
 
 
 /************************************************************************************
 * Name:     sendChar
 * Purpose:  Function to display a character on an LCD.
 * Inputs:	char a
 * Output:
 ************************************************************************************/
 void sendChar(char a) {
	 Wait();
	 __far_mem_write(LCD_DATA, a);
 }

 /************************************************************************************
 * Name:     senStr
 * Purpose:  Function to display a C-string on an LCD.
 * Inputs:	char* a
 * Output:
 ************************************************************************************/
 void sendStr(char* str) {
	 while(*str != 0) {
		 Wait();
		 if(*str == 10) {
			 __far_mem_write(LCD_CMD, 0xC0);		// Move to new line in LCD
			 str++;
		 }
		 else {
			 __far_mem_write(LCD_DATA, *str);	// Print character to LCD
			 str++;								// Point to next character in string
		 }
	 }
 }
 
 /************************************************************************************
 * Name:     initTableStorage
 * Purpose:  Initialize the array that will store pulse of every remote key pressed
 * Inputs:	 
 * Output:
 ************************************************************************************/
 void initTableStorage() {
	 for(int i = 0; i != 10; ++i) {
		 keyNumPulse[i] = 0;
		 for(int j = 0; j != 100; ++j) {
			 keyWidth[i][j] = 0;
			 tmpKeyWidth[j] = 0;
		 }
	 }
	 tmpKeyWidth[100] = 0;
 }
 
 
 /************************************************************************************
 * Name:     clearLCD
 * Purpose:  Clear LCD and blink cursor
 * Inputs:	 
 * Output:
 ************************************************************************************/
 void clearLCD() {
	 Wait();
	 __far_mem_write(LCD_CMD,0x0F);	// Display cursor blink
	 Wait();
	 __far_mem_write(LCD_CMD,0x01);	// Clear Home
 }
 
 
 /************************************************************************************
 * Name:     playNote
 * Purpose:  Clear LCD and blink cursor
 * Inputs:	 CCA for note, N for note counter, PER for duration, N for duration counter
 * Output:
 ************************************************************************************/
void playNote(uint16_t noteFreq, uint8_t n, uint16_t dur, uint16_t durN) {
	TCF1_CCA = noteFreq;				// Set CCA for requested note
	TCC0_PER = dur;						// Set PER for duration timer
	
	// Init both timer's counter to zero
	TCF1_CNT = 0x00;
	TCC0_CNT = 0x00;
	
	// Start both timer
	TCC0_CTRLA = durN;					// Start duration timer at N = 1 (Change later
	TCF1_CTRLA = n;						// Set N based on frequency trying to achieve
	
	while (TCC0_CNT != TCC0_PER) {}		// Wait for note to be done playing
}


 /************************************************************************************
 * Name:     PLAY(int)
 * Purpose:  Play music that correspond to a received key
 * Inputs:	 key
 * Output:
 ************************************************************************************/
void PLAY(int choice) {
	// Variables declaration
	char* msg[16] = {"A6\n1760.00 Hz","C6\n1046.50 Hz","C#6/Db6\n1108.73 Hz","D6\n1174.66 Hz","D#6/Eb6\n1244.51 Hz","E6\n1318.51 Hz","F6\n1396.91 Hz","F#6/Gb6\n1479.98 Hz","G6\n1567.98 Hz","G#6/Ab6\n1661.22 Hz","A#6/Bb6\n1864.68 Hz","B6\n1975.53 Hz","C7\n2093.00 Hz","C#7/Db7\n2217.46 Hz","Ascending\nScale","C Major\nArpergio"};
	uint16_t per[14] = {575,968,914,862,814,768,725,684,645,609,543,512,483,456};
	uint8_t n[14] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	uint16_t duration[14] = {62500,6047,14578, 15625};
	uint16_t durN[14] = {4,5,5,5};
	
	// Play song based on choice passed	
	switch(choice) {
		case -1: // Turn OFF sounds
		while(SCAN_KEYPAD() == -1) {}			// Infinitely keep scanning for keypad
		break;
		case 14: // Play ascending scale or song 1
		sei();
		clearLCD();								// Clear the LCD
		sendStr(msg[choice]);					// Send message to LCD
		playNote(per[12],n[12], duration[2], durN[2]);
		playNote(per[3],n[3], duration[2], durN[2]);
		playNote(per[5],n[5], duration[2], durN[2]);
		playNote(per[6],n[6], duration[2], durN[2]);
		playNote(per[8],n[8], duration[2], durN[2]);
		playNote(per[0],n[0], duration[2], durN[2]);
		playNote(per[11],n[11], duration[2], durN[2]);
		playNote(per[12],n[12], duration[2], durN[2]);
		while(SCAN_KEYPAD() != -1) {}
		break;
		case 15: // Play C major Arpergio or song 2
		sei();
		clearLCD();
		sendStr("Hotline Bling\nby Drake");
		playNote(per[3],n[3], duration[0], durN[0]);
		playNote(per[3],n[3], duration[0], durN[0]);
		playNote(per[3],n[3], duration[0], durN[0]);
		playNote(per[6],n[6], duration[0], durN[0]);
		playNote(per[5],n[5], duration[0], durN[0]);
		playNote(per[3],n[3], duration[0], durN[0]);
		playNote(per[1],n[1], duration[0], durN[0]);
		playNote(per[5],n[5], duration[2], durN[2]);
		playNote(per[1],n[1], duration[2], durN[2]);
		
		// Delay for 1 second
		PORTF_DIRTGL = 0xFF;						// Turn OFF sounds
		playNote(per[1],n[1], duration[3], durN[3]);
		playNote(per[1],n[1], duration[3], durN[3]);
		playNote(per[1],n[1], duration[3], durN[3]);
		playNote(per[1],n[1], duration[3], durN[3]);
		PORTF_DIRTGL = 0xFF;						// Reset sound setting
		
		playNote(per[6],n[6], duration[0], durN[0]);
		playNote(per[5],n[5], duration[0], durN[0]);
		playNote(per[3],n[3], duration[0], durN[0]);
		playNote(per[1],n[1], duration[0], durN[0]);
		playNote(per[5],n[5], duration[3], durN[3]);
		playNote(per[1],n[1], duration[3], durN[3]);
		playNote(per[0],n[0], duration[3], durN[3]);
		
		// Delay for 1 second
		PORTF_DIRTGL = 0xFF;						// Turn OFF sounds
		playNote(per[1],n[1], duration[3], durN[3]);
		playNote(per[1],n[1], duration[3], durN[3]);
		playNote(per[1],n[1], duration[3], durN[3]);
		playNote(per[1],n[1], duration[3], durN[3]);
		PORTF_DIRTGL = 0xFF;						// Reset sound setting
		
		playNote(per[6],n[6], duration[0], durN[0]);
		playNote(per[12],n[12], duration[1], durN[1]);
		playNote(per[12],n[12], duration[0], durN[0]);
		playNote(per[12],n[12], duration[0], durN[0]);
		playNote(per[0],n[0], duration[0], durN[0]);
		playNote(per[12],n[12], duration[0], durN[0]);
		playNote(per[12],n[12], duration[0], durN[0]);
		playNote(per[13],n[13], duration[2], durN[2]);
		
		
		playNote(per[6],n[6], duration[0], durN[0]);
		playNote(per[0],n[0], duration[0], durN[0]);
		playNote(per[8],n[8], duration[0], durN[0]);
		playNote(per[6],n[6], duration[0], durN[0]);
		playNote(per[8],n[8], duration[0], durN[0]);
		playNote(per[8],n[8], duration[0], durN[0]);
		playNote(per[0],n[0], duration[2], durN[2]);
		
		
		playNote(per[12],n[12], duration[0], durN[0]);
		playNote(per[0],n[0], duration[0], durN[0]);
		playNote(per[12],n[12], duration[0], durN[0]);
		playNote(per[0],n[0], duration[0], durN[0]);
		playNote(per[12],n[12], duration[0], durN[0]);
		playNote(per[0],n[0], duration[0], durN[0]);
		playNote(per[12],n[12], duration[0], durN[0]);
		playNote(per[0],n[0], duration[0], durN[0]);
		playNote(per[13],n[13], duration[2], durN[2]);
		
		while(SCAN_KEYPAD() != -1) {}
		break;
		default: // Play Note
		sei();									// Enable global interrupt
		clearLCD();								// Clear the LCD
		sendStr(msg[choice]);					// Send message to LCD
		playNote(per[choice],n[choice], duration[0], durN[0]);
		while(SCAN_KEYPAD() != -1) {}
		break;
	}
}


void READ_KEY() {	
	sei();										// Enable Global interrupt
	while(doneRecording == 0) {}				// Wait for remote signal to be done processed
	cli();										// Disable Global interrupt
}

void storeKeyToTable() {
	keyNumPulse[recordKey] = buttonRecording;
	for(int i = 1; i != buttonRecording; ++i) {
		// Copy value and reset recordedKeyWidth
		keyWidth[recordKey][i] = recordedKeyWidth[i];
		recordedKeyWidth[i] = 0;
	}
}

int COMPARE_KEY() {
	// Loop through all the 10 possible keys
	for(int i = 0; i != 10; ++i) {
		
	}
}

int main(void) {
	// Initialize all the necessary components
	EBI_init();
	LCD_init();
	KEYPAD_init();
	TC_init();
	initTableStorage();
	
	// Other variables
    keyBeingRecorded = 0;
	doneRecording = 1;										// Set = 1 denote that nothing is being recorded
	
	// TESTING
	//int menu = 1;
	
	while(1) {
		
		// Display Menu to user
		sendStr("1)Record a key\n2)Play Something");
		int menu = SCAN_KEYPAD();							// Read user selected menu
		switch(menu) {
			case 1: // RECORDING A KEY
	
				while(menu == 1){ menu = SCAN_KEYPAD();}// Wait until user release key						
				clearLCD();									// Clear LCD screen
				sendStr("Press a key\nin keypad");			// Tell user to press a key in keypad
				do {
					recordKey = SCAN_KEYPAD();				// Read key from the user
					int tmp = recordKey;
					while(recordKey == tmp) {				// Wait until user release key
						recordKey = SCAN_KEYPAD(); 
					}
				} while( recordKey < 0 && recordKey > 9);	// Loop until user enter a valid key (0-9)}
				_delay_ms(2000);							// Delay for 2 second so user have time
				clearLCD();									// Clear LCD screen  
				sendStr("Press same key\nin remote");		// Let user know that key is recording
				_delay_ms(2000);							// Delay for 2 second so user have time to push button
		
				// READ KEY PRESSED IN REMOTE
				READ_KEY();
				// storeKeyToTable();						// Set key
				buttonRecording = 0;						// Reset counter
				
				
				clearLCD();									// Clear LCD screen
				sendStr("Key recorded");					// Let user know that key is recorded
				_delay_ms(5000);							// Wait for 2 second, so user can see screen
				clearLCD();									// Clear LCD screen
				break;
				
			// PLAY KEY SONG
			case 2: while(menu == 2){menu = SCAN_KEYPAD();}	// Wait until user release key
				READ_KEY();									// Read key from remote
															// Find out which key was pressed in remote
				PLAY(15);									// Call play method to play the music corresponding to key
				clearLCD();									// Clear LCD screen
				break;
				
			// DO NOTHING IF NO MENU KEY PRESSED
			default:
				break;										
		}
    }
}

// Poll to check if data is read completely
ISR(TCC1_OVF_vect) {
	TCC1_CTRLA = 0x00;									// Turn off timer for 10 ms delay
	TCC1_CNT = 0;										// Reset 10 ms counter to 0
	if(doneRecording == 0) {							// If there was a recording, signal that end of recording is reached
		doneRecording = 1;
		TCC1_CTRLA = 0x00;								// Turn off timer
	}
	TCC1_INTFLAGS = 0x01;								// Restore interrupt flags
	TCF0_INTFLAGS = 0x11;								// Reset INPUT CAPTURE INTERRUPT flag
}

// Checking pulse
ISR(TCF0_CCA_vect) {
	TCC1_CTRLA = 0x00;									// Turn off timer for 10 ms delay
	TCC1_INTFLAGS = 0x01;								// Reset interrupt flag on 10 ms timer
	TCF0_INTFLAGS = 0x11;								// Reset INPUT CAPTURE INTERRUPT flag
	if(buttonRecording < 101) {
		recordedKeyWidth[buttonRecording] = TCC1_CNT;
		buttonRecording++;
		TCC1_CNT = 0x00;								// Reset the counter to 0
		TCC1_CTRLA = 0x01;								// Turn timer back ON
	}
	else {
		doneRecording = 1;								// Set flag to state that you are done recording
		TCC1_CTRLA = 0x00;								// Turn off timer
		TCC1_CNT = 0x00;								// Reset the counter to 0
	}
}

// Poll note duration interrupt
ISR(TCC0_OVF_vect) {
	TCC0_CTRLA = 0x00;					// Turn OFF duration timer
	TCF0_CTRLA = 0x00;					// Turn OFF sound timer
	TCC0_INTFLAGS = 0x01;				// Reset interrupt vector
}
