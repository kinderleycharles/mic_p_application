/*
* Lab6_A_RRN.c
*
* LAB 6 Part A
* Name: Ryan Nordman
* Section # 6483
* TA Name: Jacob Panikulam
*
* Description: This program displays "Ryan Nordman"
* to the screen when initialized.
*/


#include <avr/io.h>
#include "ebi_driver.h"


#define CS1_Start 0x470000
#define LCD_Start 0x471000
#define LCD_Start_1 0x471001

void Wait() {
	while (__far_mem_read(LCD_Start) & 0x80);
}

void LCD_INIT() {
	Wait();
	__far_mem_write(LCD_Start, 0x38); //2 LINE MODE
	Wait();
	__far_mem_write(LCD_Start,0x0F); //TURN ON DISPLAY
	Wait();
	__far_mem_write(LCD_Start,0x01); //CLEAR HOME
	Wait();
	__far_mem_write(LCD_Start,0X06); //set cursor to inc w/o shifting screen
}

void EBI_init(){
	PORTH_DIRSET = 0x37;	//WE, RE, ALE1, CS1
	PORTH_OUTSET = 0x33;	//WE, RE, CS0 and CS1; _ and . are interchangable
	PORTJ_DIRSET = 0xFF;	//Data line is output now
	PORTK_DIRSET = 0xFF;	//Address PORTK lines are outputs
	
	EBI_CTRL = 0x01;	//3port SRAM ALE1 mode (00-00-00-01)
	EBI_CS1_BASEADDR = (uint16_t) (CS1_Start) & 0xFFFF;	//set the CS1 address
	EBI_CS1_CTRLA = 0x21;	//64k SRAM (0-01000-01)
}


void OUT_STRING(char* character2) {
	while(*character2){
		Wait();
		if('/n' == character2)
		__far_mem_write(LCD_Start, 0xC0);
		else
		__far_mem_write(LCD_Start_1, *character2);
		character2++;
	}
	
}

int main(void)
{
	EBI_init();
	LCD_INIT();
	OUT_STRING("Ryan Nordman");
	
}