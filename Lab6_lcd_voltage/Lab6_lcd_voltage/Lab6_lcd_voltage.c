/*
 * Lab 6 Part B
 * Name:		Kinderley Charles
 * Section:		6957
 * TA:			Khaled Hassan
 * Description:	The following program reads the a voltage and display the voltage read
 *				on the LCD. It is essentially a voltimeter.
 */ 


#include <avr/io.h>
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

/************************************************************************************
* Name:     OUT_VOLTAGE
* Purpose:  Function to convert the ADC voltage
* Inputs:
* Output:
************************************************************************************/
void OUT_VOLTAGE() {
	int i;
	for(i=0; i!=5000; ++i);
	
	uint16_t volatile tmp = ADCB_CH0_RES & 0x00FF;
	float volt = tmp/128.0 *2.5;
	
	clearLCD();
	
	float tmp2 = volt;
	sendChar('0'+(int)tmp2);
	sendChar('.');							// Add decimal point
	float tmp3 = 10*(tmp2 - (int)tmp2);
	sendChar('0'+(int)tmp3);
	float tmp4 = 10*(tmp3 - (int)tmp3);
	sendChar('0'+(int)tmp4);
	sendChar('V');
	
	// Added during lab
	sendStr("  (0x");
	sendChar(')');						
}

int main(void)
{	
	EBI_init();
	LCD_init();
	CONFIG_ADC();
	while(1) {
		OUT_VOLTAGE();
	}
}