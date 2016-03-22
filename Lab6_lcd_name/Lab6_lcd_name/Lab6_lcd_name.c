/*
 * Lab 6 Part A
 * Name:		Kinderley Charles
 * Section:		6957
 * TA:			Khaled Hassan
 * Description:	The following program display a user name to and LCD. 
 *				For simplicity of the program, a specific name (Kinderley Charles) 
 *				was printed on the LCD screen
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
	// Do not need to set PortJ to outputs
	
	EBI.CTRL = EBI_SRMODE_ALE1_gc | EBI_IFMODE_3PORT_gc;            // ALE1 multiplexing, 3 port configuration

	EBI.CS0.BASEADDRH = (uint8_t) (CS0_Start>>16) & 0xFF;
	EBI.CS0.BASEADDRL = (uint8_t) (CS0_Start>>8) & 0xFF;            // Set CS0 range to 0x008000 - 0x009FFF
	EBI.CS0.CTRLA = EBI_CS_MODE_SRAM_gc | EBI_CS_ASPACE_8KB_gc;	    // SRAM mode, 8k address space

	// BASEADDR is 16 bit (word) register. C interface allows you to set low and high parts with 1
	// instruction instead of the previous two
	EBI.CS1.BASEADDR = (uint16_t) (CS1_Start>>8) & 0xFFFF;          // Set CS1 range to 0x370000 - 0x37FFFF
	EBI.CS1.CTRLA = EBI_CS_MODE_SRAM_gc | EBI_CS_ASPACE_64KB_gc;	// SRAM mode, 64k address space
}

int main(void)
{	
	EBI_init();
	LCD_init();
	sendStr("Kinderley\nCharles");
}