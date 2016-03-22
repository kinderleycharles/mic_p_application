/*
 * Lab 7 Part A
 * Name:		Kinderley Charles
 * Section:		6957
 * TA:			Khaled Hassan
 * Description:	The following program send a specific note (A6) which resonates at
 *				a frequency 1760 Hz while a key is pressed in a keypad
 */


#include <avr/io.h>
#include "ebi_driver.h"

#define CS0_Start 0x8000
#define CS0_End 0x9FFF
#define CS1_Start 0x470000
#define CS1_End 0x47FFFF



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
	TCF0_CTRLA = 0x01;						// Set Prescaler OFF; N = 1
	TCF0_CTRLB = 0x11;						// Set CCA on F0, set mode to FRQ
	TCF0_CTRLC = 0x00;						// To set compare register if diff. compare value used (UNUSED)
	TCF0_CTRLD = 0xA0;						// No capture is being performed for this lab
	TCF0_CTRLE = 0x00;						// Set timer/counter to normal mode
	
	TCF0_CCA = 575;							// Set CCA to 567 assuming that prescaler 1 is being used
	TCF0_CNT = 0x0000;						// Set counter to 0
	TCF0_CTRLFCLR = 0x00;					// Set counter to imcrementing
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


int main(void) {
	
	// Initialize all the necessary components
	EBI_init();
	KEYPAD_init();
	TC_init();
    while(1) {
		// if 1 is pressed, output note A6
        if(SCAN_KEYPAD() == 1) {
			TCF0_CTRLB = 0x11;
		}
		else {
			TCF0_CTRLB = 0x01;
		}
    }
}