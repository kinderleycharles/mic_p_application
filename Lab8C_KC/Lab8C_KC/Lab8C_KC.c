/*
 * Lab 8 Part B
 * Name:		Kinderley Charles
 * Section:		6957
 * TA:			Khaled Hassan
 * Description:	The following program reads an input from a remote control
 *				and display the key pressed
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include "ebi_driver.h"

#define XMEGA_CLK 2000000						// XMEGA clocks at 2MHz
#define LCD_CMD 0x471000						// Command Address
#define LCD_DATA 0x471001
#define CS0_Start 0x8000
#define CS0_End 0x9FFF
#define CS1_Start 0x470000
#define CS1_End 0x47FFFF

// Function Prototypes
void clearLCD();
void LCD_init();
void sendStr(char*);


// Array that will store the width of every pulse and number of pulse
double keyWidth[10][100];
int keyNumPulse[10];
int recordingKey = 0;							// Use to determine whether key is being recorded or not
int doneRecording;								// 1 when recording is done, 0 when false
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
	PORTF_DIR = 0x01;						// Set port F pin 0 as output
	PORTF_DIRCLR = 0x10;					// Set port F pin 4 as input
	PORTF_OUT = 0xFF;						// Output 0 to all pins in PORTF
	
	// Configure Type 0 counter on Port F pin 0 to generate the frequency of every key pressed
	TCF0_CTRLA = 0x00;						// Set Prescaler OFF; N = 1
	TCF0_CTRLB = 0x11;						// Set CCA on F0, set mode to FRQ
	TCF0_CTRLC = 0x00;						// To set compare register if diff. compare value used (UNUSED)
	TCF0_CTRLD = 0xA0;						// FRQ capture is being performed for this lab
	TCF0_CTRLE = 0x00;						// Set timer/counter to normal mode
	TCF0_CCA = 574;							// Set CCA to 567 assuming that prescaler 1 is being used
	TCF0_CNT = 0x0000;						// Set counter to 0
	TCF0_CTRLFCLR = 0x00;					// Set counter to imcrementing
	
	
	// Configure Type 1 counter on Port F Pin 1 to keep track of how long song has been playing
	TCF1_CTRLA = 0x00;						// Set timer counter ON, N = 1
	TCF1_CTRLB = 0x10;						// Set CCA on F1, set timer waveform generation to Normal
	TCF1_CTRLC = 0x00;						// To set compare register if diff. compare value used (UNUSED)
	TCF1_CTRLD = 0x00;						// No capture is being performed for this lab
	TCF1_CTRLE = 0x00;						// Set timer/counter to normal mode
	TCF1_INTCTRLA = 0x01;					// ERROR interrupt = False, Overflow interrupt = Low priority	
	TCF1_PER = 574;							// Set CCA to 567 assuming that prescaler 1 is being used
	TCF1_CNT = 0x0000;						// Set counter to 0
	TCF1_CTRLFCLR = 0x00;					// Set counter to incrementing
	
	
	// REMOTE TIMER CONFIGURATIONS
	// Input capture configuration, need to be set to input capture
	TCE0_CTRLA = 0x00;						// Set timer OFF initially
	TCE0_CTRLB = 0x10;						// Set Capture on CCA and timer to normal mode
	TCE0_CTRLC = 0x00;						// The compare register is not being used
	TCE0_CTRLD = 0x28;						// Set event capture to INPUT CAPTURE and timer even source selection to channel 0
	TCE0_CTRLE = 0x00;						// Set Byte Mode to normal
	TCE0_INTCTRLA = 0x01;					// Set ERROR interrupt OFF, and OVERFLOW interrupt to LOW priority
	TCE0_INTCTRLB = 0x11;					// Enable LOW level interrupt on CCA
	TCE0_INTFLAGS = 0x11;					// Clear OVERFLOW INTERRUPT FLAG, and CCAIF flag
	TCE0_CNT = 0x0000;						// Set counter to 0
	PORTE_PIN0CTRL = 0x00;					// Set pin 0 enable to BOTH EDGES
	//PORTE_INTCTRL = 0x05;
	
	
	
	// Timer configurations, need to be set to normal mode
	TCE1_CTRLA = 0x00;						// Set timer OFF initially
	TCE1_CTRLB = 0x10;						// Set Capture on CCA and timer to normal mode
	TCE1_CTRLC = 0x00;						// The compare register is not being used
	TCE1_CTRLD = 0x00;						// No capture is being done
	TCE1_CTRLE = 0x00;						// Set Byte Mode to normal
	TCE1_INTCTRLA = 0x01;					// Set ERROR interrupt OFF, and OVERFLOW interrupt to LOW priority
	TCE1_INTFLAGS = 0x01;					// Clear OVERFLOW INTERRUPT FLAG
	TCE1_CNT = 0x0000;						// Set counter to 0
	TCE1_PER = 9999;						// Set PER to 10 ms
	
	PMIC_CTRL = 0x01;						// Set Global interrupt ON
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
 * Name:     READ_KEY(int key)
 * Purpose:  Function determines which key in a keypad was pressed
 * Inputs:	 int
 * Output:
 ************************************************************************************/
 void READ_KEY() {
	 TCE0_CTRLA = 0x01;				// Turn ON timer, N=1
	 TCE1_CTRLA = 0x01;
	 while(doneRecording == 0){}		// Wait until done recording flag is set
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

int main(void) {
		
	// Initialize all the necessary components
	EBI_init();
	LCD_init();
	TC_init();
	for(int i = 0; i != 10; ++i) {
		keyNumPulse[i] = 0;
		for(int j = 0; j != 100; ++j) {
			keyWidth[i][j] = 0;
			recordedKeyWidth[j] = 0;
		}
	}
	
	doneRecording = 0;					// Set done recording to False
    recordingKey = 1;					// Set key to start recording key
	
	while(1) {
		doneRecording = 0;				// Set done recording flag to false
		
		clearLCD();						// Clear LCD screen
		sendStr("Reading key");			// Printing message that key is being read
		
		sei();							// Turn ON global interrupt
		READ_KEY();						// Read key pressed
		cli();							// Disable interrupt
										
		clearLCD();						// Clear LCD screen
		sendStr("Done recording");		// Print message of key done recording
	}
}

// Poll to check if data is read completely
ISR(TCE1_OVF_vect) {				
	if(doneRecording == 0) {
		doneRecording = 1;
		TCE0_CTRLA = 0x00;			// Turn off timer
		TCE1_CTRLA = 0x00;			// Turn off timer
	}
	else {
		doneRecording = 0;
	}
	TCE1_INTFLAGS = 0x11;			// Restore interrupt flags
	TCE0_INTFLAGS = 0x11;								// Reset interrupt flags
}

// Checking pulse
ISR(TCE0_CCA_vect) {
	TCE1_CTRLA = 0x00;									// Turn off timer
	if(buttonRecording < 101) {
		recordedKeyWidth[buttonRecording] = TCE0_CNT;
		buttonRecording++;
	}
	else {
		doneRecording = 1;								// Set flag to state that you are done recording
	}
	TCE0_INTFLAGS = 0x11;								// Reset interrupt flags
	TCE1_CNT = 0x00;									// Reset the counter to 0
	TCE1_CTRLA = 0x01;									// Turn timer back ON
}

// Poll note duration interrupt
ISR(TCF1_OVF_vect) {
	TCF1_CTRLA = 0x00;											// Turn OFF duration timer
	TCF0_CTRLA = 0x00;											// Turn OFF sound timer
	TCF1_INTFLAGS = 0x01;										// Reset interrupt vector
}