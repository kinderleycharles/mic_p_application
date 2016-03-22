/*
 * Quiz_Lab7.c
 *
 * Created: 11/19/2015 7:46:12 PM
 *  Author: Kin
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

void INIT_TC() {
	PORTD_DIRSET = 0xFF;		// Set all pins in PORTF as output
	PORTD_OUT = 0x00;
	
	TCF0_CTRLA = 0x06;			// Set timer ON
	TCF0_CTRLB = 0x00;			// Set CCA true and mode to NORMAL
	TCF0_CTRLC = 0x00;			// Unused in this example but set to 0 regardless
	TCF0_CTRLD = 0x00;			// No event but set to 0 regardless
	TCF0_CTRLE = 0x00;			// Timer BYMODE = NORMAL
	TCF0_INTCTRLA = 0x01;		// Enable overflow interrupt
	//TCF0_INTFLAGS = 0x01;		// Clear flag
	
	PMIC_CTRL = 0x01;
	
	TCF0_CNT = 0x0000;			// Counter = 0
	TCF0_PER = 31250;			// 
}

int main(void) {
	INIT_TC();
	sei();
    while(1){
		asm volatile("nop");
    }
}

ISR(TCF0_OVF_vect) {
	PORTD_OUT = ~(0x01);		// Print light 
	if (PORTD_DIR == 0xFF) {
		PORTD_DIR == 0x00;
	}
	else {
		PORTD_DIR == 0xFF;
	}
	//TCF0_INTFLAGS = 0x01;
}