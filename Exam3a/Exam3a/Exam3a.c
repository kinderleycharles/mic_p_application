/*
 * Exam3a.c
 *
 * Name:	Kinderley Charles
 *
 */ 


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
* Name:     LCD_init
* Purpose:  Function to initialize the LCD screen
* Inputs:
* Output:
************************************************************************************/
void LCD_init() {
	Wait();
	__far_mem_write(LCD_CMD, 0x38);	// Enable 2 line mode (See LCD Manual)
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
	TCF0_CCA = 606070;							// Set CCA to 567 assuming that prescaler 1 is being used
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
* Name:     CONFIG_ADC
* Purpose:  Function to configure ADC appropriately
* Inputs:
* Output:
************************************************************************************/
void CONFIG_ADC() {
	ADCB_CTRLA = 0x01;					// Enable ADC Conversion
	ADCB_CTRLB = 0x1C;					// Set 8 bit signed configuration free-run mode
	ADCB_EVCTRL = 0x05;
	ADCB_REFCTRL = 0x30;				// Set external reference
	ADCB_PRESCALER = 0x07;		
	
	ADCB_CH0_MUXCTRL = 0x20;			// Set which pins you are connected to
	ADCB_CH0_CTRL = 0x81;
}


void delayVRead(void){
	int i;
	for(i=0; i!=5000; ++i);
}

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



/************************************************************************************
* Name:    WAVEFORM
* Purpose:  Function to convert the ADC voltage
* Inputs:
* Output:
************************************************************************************/
void WAVEFORM(float voltage) {
	// Variables declaration
	char* msg[16] = {"A6\n1760.00 Hz","C6\n1046.50 Hz","C#6/Db6\n1108.73 Hz","D6\n1174.66 Hz","D#6/Eb6\n1244.51 Hz","E6\n1318.51 Hz","F6\n1396.91 Hz","F#6/Gb6\n1479.98 Hz","G6\n1567.98 Hz","G#6/Ab6\n1661.22 Hz","A#6/Bb6\n1864.68 Hz","B6\n1975.53 Hz","C7\n2093.00 Hz","C#7/Db7\n2217.46 Hz","Ascending\nScale","C Major\nArpergio"};
	uint16_t per[14] = {575,968,914,862,814,768,725,684,645,609,543,512,483,456};
	uint8_t n[14] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	uint16_t duration[14] = {62500,6047,14578, 15625};
	uint16_t durN[14] = {4,5,5,5};
	
	if(voltage < 3.8){
		playNote(per[1],n[1], duration[0], durN[0]);
	}
	else {
		playNote(per[10],n[10], duration[0], durN[0]);
	}
}


/************************************************************************************
* Name:     OUT_VOLTAGE
* Purpose:  Function to convert the ADC voltage
* Inputs:
* Output:
************************************************************************************/
void OUT_VOLTAGE() {
	
	// Delay to give time for voltage to be computed
	int i;
	for(i=0; i!=5000; ++i);
	
	// Read potentiometer value and perform calculation for Xmega
	uint16_t volatile tmp = ADCB_CH0_RES & 0x00FF;
	float volt = tmp/128.0 *5;
	WAVEFORM(volt);
	
	// Clear LCD and print the message to the screen
	clearLCD();
	
	float tmp2 = volt;
	sendChar('0'+(int)tmp2);
	sendChar('.');							// Add decimal point
	float tmp3 = 10*(tmp2 - (int)tmp2);
	sendChar('0'+(int)tmp3);
	float tmp4 = 10*(tmp3 - (int)tmp3);
	sendChar('0'+(int)tmp4);
	sendChar('V');
}



int main(void) {
	EBI_init();
	LCD_init();
	CONFIG_ADC();
	TC_init();
	sei();
    while(1)
    {
       // Perform a simple test
	   OUT_VOLTAGE(); 
    }
}

// Poll note duration interrupt
ISR(TCF1_OVF_vect) {
	TCF1_CTRLA = 0x00;					// Turn OFF duration timer
	TCF0_CTRLA = 0x00;					// Turn OFF sound timer
	TCF1_INTFLAGS = 0x01;				// Reset interrupt vector
}