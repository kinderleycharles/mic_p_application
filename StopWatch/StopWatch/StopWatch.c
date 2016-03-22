/*
 * StopWatch.c
 *
 * Created: 12/7/2015 8:27:22 PM
 *  Author: Kin
 * Description: This program is a stopwatch that start at 0 and counts up
 *				if the user press any key in the keyPad, the watch stop
 */ 

#include <stdlib.h>								// Allow in to string casting
#include <stdio.h>								// Allow string concatenation
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ebi_driver.h"

#define F_CPU 2000000							// ATxmega runs at 2MHz on Reset.
#define LCD_CMD 0x471000						// Command Address
#define LCD_DATA 0x471001
#define CS0_Start 0x8000
#define CS0_End 0x9FFF
#define CS1_Start 0x470000
#define CS1_End 0x47FFFF

// Global variables to keep track of which key is pressed
int volatile keyPressed = -1;					// Keep track of which key the user 
												// push in the keypad

int seconds = 00;
int minutes = 00;
int hours = 00;

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
 * Name:     EBI_INIT
 * Purpose:  Function to initialize the desired EBI Ports.  Configures to run IO Port,
 *           SRAM, and LCD.  All CSs and other control signals generate appropriate
 *           enables inside CPLD.
 * Inputs:
 * Output:
 ************************************************************************************/
 void EBI_INIT()
 {
	 PORTH.DIR = 0x37;												// Enable RE, WE, CS0, CS1, ALE1
	 PORTH.OUT = 0x33;												// Defaults: RE=H, WE=H, CS0=H, CS1=H, ALE1=0
	 PORTK.DIR = 0xFF;												// Enable Address 7:0 (outputs)
	 EBI.CTRL = EBI_SRMODE_ALE1_gc | EBI_IFMODE_3PORT_gc;           // ALE1 multiplexing, 3 port configuration

	 EBI.CS0.BASEADDRH = (uint8_t) (CS0_Start>>16) & 0xFF;
	 EBI.CS0.BASEADDRL = (uint8_t) (CS0_Start>>8) & 0xFF;           // Set CS0 range to 0x008000 - 0x009FFF
	 EBI.CS0.CTRLA = EBI_CS_MODE_SRAM_gc | EBI_CS_ASPACE_8KB_gc;	// SRAM mode, 8k address space

	 // BASEADDR is 16 bit (word) register. C interface allows you to set low and high parts with 1
	 // instruction instead of the previous two
	 EBI.CS1.BASEADDR = (uint16_t) (CS1_Start>>8) & 0xFFFF;          // Set CS1 range to 0x370000 - 0x37FFFF
	 EBI.CS1.CTRLA = EBI_CS_MODE_SRAM_gc | EBI_CS_ASPACE_64KB_gc;	 // SRAM mode, 64k address space
 }



/************************************************************************************
 * Name:     TC_INIT()
 * Purpose:  Function to configure timer counter of the XMEGA and the pin to be used
 *			 as output
 * Inputs:
 * Output:
 ************************************************************************************/
void TC_INIT() {
	PORTC_DIR = 0xFF;						// Set all port C pins as output
	PORTC_OUT = 0xFF;						// Output 1's to all port C pins
		
	// Configure Type 1 counter on Port C for counting time elapsed by note
	TCC1_CTRLA = 0x00;						// Set timer counter ON, N = 1
	TCC1_CTRLB = 0x10;						// Set CCA on F1, set timer waveform generation to Normal
	TCC1_CTRLC = 0x00;						// To set compare register if diff. compare value used (UNUSED)
	TCC1_CTRLD = 0xA0;						// No capture is being performed for this lab
	TCC1_CTRLE = 0x00;						// Set timer/counter to normal mode
	TCC1_INTCTRLA = 0x01;					// ERROR interrupt = False, Overflow interrupt = Low priority
	TCC1_INTFLAGS = 0x01;					// Clear interrupt flag	
	TCC1_PER = 9999;						// Set CCA to 567 assuming that prescaler 1 is being used
	TCC1_CNT = 0;							// Set counter to 0
	TCC1_CTRLFCLR = 0x00;					// Set counter to incrementing

	PMIC_CTRL = PMIC_LOLVLEN_bm;			// Step 7: Enable low level in PMIC, global interrupt
	sei();
}



/************************************************************************************
 * Name:     KEYPAD_INIT
 * Purpose:  Function to initialize PORT E for the keypad. Bit(0-3) set as outputs.
 *			 Bit(4-7) set as inputs
 * Inputs:
 * Output:
 ************************************************************************************/
 void KEYPAD_INIT() {
	 PORTE.DIR = 0x0F;						// Set least significant nibble as output 
	 PORTE.DIRCLR = 0xF0;					// Most significant nibble as input
	 PORTE_IN = 0xF0;
	 PORTE_OUT = 0x0F;
	 
	 // Pull up resistors
	 uint8_t volatile a = 0b00011000;
	 PORTE_PIN7CTRL = a;
	 PORTE_PIN6CTRL = a;
	 PORTE_PIN5CTRL = a;
	 PORTE_PIN4CTRL = a;
 }


/************************************************************************************
 * Name:     LCD_INIT
 * Purpose:  Function to initialize the LCD screen before the program
 * Inputs:
 * Output:
 ************************************************************************************/
 void LCD_INIT() {
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
 * Name:     KEYPAD_INTERRUPT_INIT
 * Purpose:  Function to configure interrupt on the keypad
 * Inputs:
 * Output:
 ************************************************************************************/
void KEYPAD_INTERRUPT_INIT() {
	PORTE_INT0MASK = 0xF0;						// Set pins 4-7 as interrupt source
	PORTE_OUT = 0x0F;							// Good practice to set output to 1
												// but may not be necessary
	PORTE_DIRCLR = 0xF0;						// Set pin 4-7 as input
	PORTE_INTCTRL = 0x01;						// Set INT0LVL to low priority
	
	// Set pins (4-7) to Falling edge due to Keypad configuration
	PORTE_PIN4CTRL = 0x02;
	PORTE_PIN5CTRL = 0x02;
	PORTE_PIN6CTRL = 0x02;
	PORTE_PIN7CTRL = 0x02;
	
	PORTE_INTFLAGS = 0x01;
	PMIC_CTRL = 0x01;							// Turn on low level interrupt	
	sei();					
}


 /************************************************************************************
 * Name:     KEY_PRESSED
 * Purpose:  Function determines which key in a keypad was pressed
 * Inputs:	 int
 * Output:
 ************************************************************************************/
  int KEY_PRESSED() {
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
 
 void updateTime() {
	char str[80];
	char hr[10];
	char min[10];
	char sec[10];
	
	// Converting integers to C-String
	itoa(hours, hr, 10);
	itoa(minutes, min, 10);
	itoa(seconds, sec, 10);
	
	// Concatenating String to format the time on the LCD
	strcat(str,hr);
	strcat(str,":");
	strcat(str,min);
	strcat(str,":");
	strcat(str,sec);
	
	// Print time on screen
	sendStr(str); 
 }
 

int main(void) {
    
	// Initialize TC, EBI, and Keypad
	EBI_INIT();
	LCD_INIT();
	KEYPAD_INIT();
	TC_INIT();
	
	// Turn on time counter and run a simple loop
	TCC1_CTRLA = 0x01;
	int stop = 0;						// Flag to indicate a stop for stop watch
	while(1) {
		/*
		int volatile key = KEY_PRESSED();
		while(key == -1) {	// User doe not press any key, keep looping
			key = KEY_PRESSED();
		}
		if(stop == 0) {					// Stop watch was not stop, stop because user request so
			TCC1_CTRLA = 0x00;			// Pause timer
			stop = 1;					// Set stop flag to true
			updateTime();				// Display time to the user
		}
		else {							// User request to unpause the timer
			stop = 0;					// Set stop flag false
			TCC1_CTRLA = 0x01;			// Resume timer
		}
		*/
	}
}

/************************************************************************************
 * Name:     ISR(TCC1_OVF_vect)
 * Purpose:  Interrupt subroutine to count when 1 second has elapsed
 * Inputs:
 * Output:
 ************************************************************************************/
ISR(TCC1_OVF_vect){
	if(seconds == 59) {
		seconds = 0;
		minutes = minutes + 1;
		// Check if minutes is equal to 59
		if(minutes == 59) {
			minutes = 0;
			hours = hours + 1;
		}
		clearLCD();
		__far_mem_write(LCD_CMD,0x02);
		updateTime();
	}
	seconds = seconds + 1;
	TCC1_INTFLAGS = 0x01;							// Reset interrupt flag
}


/************************************************************************************
 * Name:     ISR(PORTE_INT0_vect)
 * Purpose:  Interrupt subroutine to handle which key is pressed. Triggered on falling
 *			 edge
 * Inputs:
 * Output:
 ************************************************************************************/
ISR(PORTE_INT0_vect){
	// Due to bouncing, make sure that either one of the pins(4-7) is set to 0
	uint8_t volatile tmp = (PORTE_IN && 0xF0);
	if( (tmp == 0x70)||(tmp == 0xB0)||(tmp == 0xD0)||(tmp == 0xE0) ){
		keyPressed = KEY_PRESSED();
	}
	PORTE_IN = 0xF0;
	PORTE_INTFLAGS = 0x01;						// Reset INT0 interrupt flag
}
