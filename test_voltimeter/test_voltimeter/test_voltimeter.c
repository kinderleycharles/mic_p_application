/*
* Lab6_B_RRN.c
*
* LAB 6 Part B
* Name: Ryan Nordman
* Section # 6483
* TA Name: Jacob Panikulam
*
* Description: This program reads from the analog to digital converter
* 1 whenever it changes, and it converts the value to a voltage
* before displaying it to the LCD screen.
*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include "ebi_driver.h"

#define CS1_Start 0x470000
#define	LCD_Start 0x471000
#define	LCD_Start_1 0X471001


void waitLCD(){
	while(__far_mem_read(LCD_Start) & 0X80); //LOOP WHILE BUSY FLAG IS 1
}

/*function to setup EBI CS1*/
void WRITE_TO_LCD(char* str){
	while(*str){
		waitLCD();
		if('/n' == *str) 
		__far_mem_write(LCD_Start, 0xC0);
		else 
		__far_mem_write(LCD_Start_1, *str);
		str++;
	}
}

void INIT_EBI_CS1(void){
	PORTH_DIRSET = 0x37;	//WE, RE, ALE1, CS1
	PORTH_OUTSET = 0x33;	//WE, RE, CS0 and CS1; _ and . are interchangable
	PORTJ_DIRSET = 0xFF;	//Data line is output now
	PORTK_DIRSET = 0xFF;	//Address PORTK lines are outputs
	
	EBI_CTRL = 0x01;	//3port SRAM ALE1 mode (00-00-00-01)
	EBI_CS1_BASEADDR = (uint16_t) (CS1_Start >> 8) & 0xFFFF;	//set the CS1 address
	EBI_CS1_CTRLA = 0x21;	//64k SRAM (0-01000-01)
}



/*function to int LCD*/
void INIT_LCD(void){
	waitLCD();
	__far_mem_write(LCD_Start, 0x38); //2 LINE MODE
	waitLCD();
	__far_mem_write(LCD_Start,0x0F); //TURN ON DISPLAY
	waitLCD();
	__far_mem_write(LCD_Start,0x01); //CLEAR HOME
	waitLCD();
	__far_mem_write(LCD_Start,0X06); //set cursor to inc w/o shifting screen
}


void CONFIGURE_ADC(){
	ADCB.CTRLB = 0x1C;
	ADCB.REFCTRL= 0x10;
	ADCB.EVCTRL = 0x05;
	ADCB.PRESCALER= 0x07;
	
	ADCB.CH0.CTRL= 0x01;
	ADCB.CH0.MUXCTRL=0x01;
	ADCB.CH0.INTCTRL=0x01;
	ADCB.CTRLA=0x05;
	
	
}


void CLEAR_LCD(){
	waitLCD();
	__far_mem_write(LCD_Start, 0x01);
	waitLCD();
	__far_mem_write(LCD_Start, 0x0C);
	
	
}


void CHAR_LCD(char c){
	{
		waitLCD();
		if('/n' == c)
		__far_mem_write(LCD_Start, 0xC0);
		else
		__far_mem_write(LCD_Start_1, c);

	}
}


void _delay(void){
	int s = 0, p = 0;
	for(s=0;s<=30000;s++)
	for(p=0;p<=13;p++){}
}

void OUT_VOLTAGE() {
	_delay();
	uint8_t volatile tmp = ADCB_CH0_RES;
	float volt = tmp/128.0 *5.0;
	// clearLCD();
	float tmp2 = volt;
	//sendChar('0'+(int)tmp2);
	//sendChar('.');							// Add decimal point
	float tmp3 = 10*(tmp2 - (int)tmp2);
	//sendChar('0'+(int)tmp3);
	float tmp4 = 10*(tmp3 - (int)tmp3);
	//sendChar('0'+(int)tmp4);
	//sendChar('V');
}

int main(void)
{
	INIT_EBI_CS1();
	INIT_LCD();
	CONFIGURE_ADC();
	OUT_VOLTAGE();
	// PMIC_CTRL = 0x01;
	// sei();
	
	while(1);
}

ISR(ADCB_CH0_vect){
	_delay();
	uint8_t input = ADCB.CH0.RESL;
	float value=input/128.0 *5.0;
	

	
	CLEAR_LCD();
	CHAR_LCD('0' + (int) value);
	CHAR_LCD('.' );
	CHAR_LCD('0'+ ((int) (value*10))  % 10);
	CHAR_LCD('0' +  ((int) (value*100)) % 10);
	WRITE_TO_LCD(" V (0X");
	CHAR_LCD(((input>>4) > 9)?  'A' + (input >> 4)-10 : '0'+(input>>4));
	CHAR_LCD(((input&15) > 9)?  'A' + (input&15)-10  : '0'+(input&15));
	CHAR_LCD(')');
	/*
	for (int i=0; i<16; i++) {
	CHAR_LCD(' ');
	}
	for (int j=0; j<30000; j++) {
	_delay();
	}
	*/		
		
	
	ADCB.INTFLAGS = 0x01;
}