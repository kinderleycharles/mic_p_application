/*
 * Lab 6 Part C
 * Name:		Kinderley Charles
 * Section:		6957
 * TA:			Khaled Hassan
 * Description:	The following program reads the input of a keypad, and display
 *				a different output based on the entry of the keypad. Below is
 *				a list of the keys pressed and their respective outputs
 *				0,1		-		Kinderley Charles
 *				2,3		-		Clear LCD and blink cursor at home
 *				4,5		-		Toggle display on or off
 *				6,7		-		"May the Schwartz be with you"
 *				*,#		-		Continuously display voltage of pot pin
 *				Others	-		Extra credits
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
 void KEYPAD_INIT() {
	 PORTE.DIR = 0x0F;						// Set least significant nibble as output and most sig as input
	 
	 // Pull up resistors
	 uint8_t volatile a = 0b00011000;
	 PORTE_PIN7CTRL = a;
	 PORTE_PIN6CTRL = a;
	 PORTE_PIN5CTRL = a;
	 PORTE_PIN4CTRL = a;
 }
 
 
 
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
 
 void Reset_LCDHOME() {
	 Wait();
	 __far_mem_write(LCD_CMD,0x01);	// Clear Home
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
			 uint8_t volatile outting = out[j];
			 if(tmp == out[j]) {
				 return keys[4*i+j];
			 }
		  }
	  }
	  return -1;
  }
 
 /************************************************************************************
 * Name:     OUT_STRING
 * Purpose:  Function to display a C-string on an LCD.
 * Inputs:	 char* str
 * Output:
 ************************************************************************************/
 void OUT_STRING(char* str) {
	 Reset_LCDHOME();
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
 * Name:     Function_2
 * Purpose:  Clear LCD and blink cursor
 * Inputs:	 
 * Output:
 ************************************************************************************/
 void Function_2() {
	 Wait();
	 __far_mem_write(LCD_CMD,0x0F);	// Display cursor blink
	 Wait();
	 __far_mem_write(LCD_CMD,0x01);	// Clear Home
 }
 
 
 /************************************************************************************
 * Name:     TOGGLE_DISPLAY
 * Purpose:  Toggle display on or off. If on, turn off. If off, turn on
 * Inputs:	 d
 * Output:	 int
 ************************************************************************************/
 uint8_t TOGGLE_DISPLAY(uint8_t volatile d) {
	 Wait();
	 if(d) {
		 // if not 0. if display is ON, turn it off
		 __far_mem_write(LCD_CMD, 0x08);		// Turn OFF display
	 }
	 else {
		 // Display is off, turn it on
		 __far_mem_write(LCD_CMD, 0x0F);		// Turn ON display
	 }
	 return !d;
 }
 
 // ADDED DURING LAB
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
	 float volt = tmp/128.0 *5.0;
	 
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
	uint8_t volatile display = 1;
    EBI_init();
	LCD_init();
	CONFIG_ADC();		// Added in Lab
	KEYPAD_INIT();
	while(1) {
		uint8_t volatile k = KEY_PRESSED();
		switch( k ) {
			case 0:
			case 1: OUT_STRING("Kinderley\nCharles");
				break;
			case 2:
			case 3: Function_2();
				break;
			case 4:
			case 5: display = TOGGLE_DISPLAY(display);
				break;
			case 6:
			case 7:	OUT_STRING("May the Shwartz\nbe with you!");
				break;
			case 14:
			case 15: Wait();
				while( KEY_PRESSED() == -1) {
					OUT_VOLTAGE();
				}
				break;
			case 8: // Do nothing if no keys are pressed
			case 9: // Do nothing if no keys are pressed
			case 10: // Do nothing if no keys are pressed
			case 11: // Do nothing if no keys are pressed
			case 12: // Do nothing if no keys are pressed
			case 13: // Do nothing if no keys are pressed
				OUT_STRING("Be Creative!");
				break;
			default: // Do nothing
				break;
		}	
	}
}