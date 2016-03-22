/*
 * Lab 7 Part B
 * Name:		Kinderley Charles
 * Section:		6957
 * TA:			Khaled Hassan
 * Description:	The following program plays a different songs when a different key
 *				is pressed
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
	PORTF_DIR = 0xFF;						// Set all port F pins as output
	PORTF_OUT = 0xFF;						// Output 0 to all pins in PORTF
	
	// Configure Type 0 counter on Port F
	TCF0_CTRLA = 0x00;						// Set Prescaler OFF; N = 1
	TCF0_CTRLB = 0x11;						// Set CCA on F0, set mode to FRQ
	TCF0_CTRLC = 0x00;						// To set compare register if diff. compare value used (UNUSED)
	TCF0_CTRLD = 0xA0;						// No capture is being performed for this lab
	TCF0_CTRLE = 0x00;						// Set timer/counter to normal mode
	TCF0_CCA = 574;							// Set CCA to 567 assuming that prescaler 1 is being used
	TCF0_CNT = 0x0000;						// Set counter to 0
	TCF0_CTRLFCLR = 0x00;					// Set counter to imcrementing
	
	
	// Configure Type 1 counter on Port F
	TCF1_CTRLA = 0x00;						// Set timer counter ON, N = 1
	TCF1_CTRLB = 0x10;						// Set CCA on F1, set timer waveform generation to Normal
	TCF1_CTRLC = 0x00;						// To set compare register if diff. compare value used (UNUSED)
	TCF1_CTRLD = 0xA0;						// No capture is being performed for this lab
	TCF1_CTRLE = 0x00;						// Set timer/counter to normal mode
	TCF1_INTCTRLA = 0x01;					// ERROR interrupt = False, Overflow interrupt = Low priority
	//TCF1_INTCTRLB = 0x01;					// CCA interrupt level
	
	PMIC_CTRL |= 1;
	
	TCF1_PER = 574;						// Set CCA to 567 assuming that prescaler 1 is being used
	TCF1_CNT = 0x0000;						// Set counter to 0
	TCF1_CTRLFCLR = 0x00;					// Set counter to incrementing
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
	TCF0_CCA = noteFreq;				// Set CCA for requested note
	TCF1_PER = dur;						// Set PER for duration timer
	
	// Init both timer's counter to zero
	TCF0_CNT = 0x00;
	TCF1_CNT = 0x00;
	
	// Start both timer
	TCF1_CTRLA = durN;					// Start duration timer at N = 1 (Change later
	TCF0_CTRLA = n;						// Set N based on frequency trying to achieve
	
	while (TCF1_CNT != TCF1_PER) {}		// Wait for note to be done playing
}


 
void turnOffSound() {
	TCF0_CTRLB = 0x01;	
} 



int main(void) {
	// Variables declaration
	char* msg[16] = {"A6\n1760.00 Hz","C6\n1046.50 Hz","C#6/Db6\n1108.73 Hz","D6\n1174.66 Hz","D#6/Eb6\n1244.51 Hz","E6\n1318.51 Hz","F6\n1396.91 Hz","F#6/Gb6\n1479.98 Hz","G6\n1567.98 Hz","G#6/Ab6\n1661.22 Hz","A#6/Bb6\n1864.68 Hz","B6\n1975.53 Hz","C7\n2093.00 Hz","C#7/Db7\n2217.46 Hz","Ascending\nScale","C Major\nArpergio"};
	uint16_t per[14] = {575,968,914,862,814,768,725,684,645,609,543,512,483,456};
	uint8_t n[14] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	uint16_t duration[14] = {62500,6047,14578, 15625};
	uint16_t durN[14] = {4,5,5,5};
	
	// Initialize all the necessary components
	EBI_init();
	LCD_init();
	KEYPAD_init();
	TC_init();
    
	/*
	while(1) {
		int choice = SCAN_KEYPAD();
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
				clearLCD();								// Clear the LCD
				
				// HOTLINE BLING
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
	*/
	
	while (1)
	{
		playNote(per[0],n[0], duration[0], durN[0]);
	}
}

// Poll the interrupt flag on the duration of the playing note


// Poll note duration interrupt
ISR(TCF1_OVF_vect) {
	TCF1_CTRLA = 0x00;					// Turn OFF duration timer
	TCF0_CTRLA = 0x00;					// Turn OFF sound timer
	TCF1_INTFLAGS = 0x01;				// Reset interrupt vector
}