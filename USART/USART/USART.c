/*/////////////////////////////////////////////////////////////
 * usart_serial.c
 *
 *  Modified: 28 October 2015
 *  Author:  Dr. Schwartz, Ivan Bukreyev, Josh Weaver
 *  Purpose: To provide an example for the USART in C.  
 *    Initializes USARTC0 and 
 *    performs interrupt driven echo.  Settings at 9600 Baud, 
 *    8 data, 1 stop, no parity.
 *//////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////
#include <avr/io.h>
#include <avr/interrupt.h>

//INITIALIZATIONS/
#define F_CPU 2000000       // ATxmega runs at 2MHz
#define BSCALE9600	0x9	 // -7
#define BSEL9600	1539

// FUNCTIONS///////////////////////////////////////
/**************************************************************
* Name:     RoughDelay1sec
* Purpose:  Simple function to create a 1 second delay using a 
*           for structure
* Inputs:
* Output:
**************************************************************/
void RoughDelay1sec(void)
{
    volatile uint32_t ticks;            // Volatile prevents compiler optimization
    for(ticks=0;ticks<=F_CPU;ticks++);  // increment 2e6 times -> ~ 1 sec
}


int main(void)
{
    //Set I/O direction for TX/RX lines
    //PORTC.DIRSET = PIN3_bm;
    //PORTC.DIRCLR = PIN2_bm;
	
	// Added by Kinderley for TX/RX lines setup for USARTD0
	PORTD.DIRSET = PIN3_bm;		// Set pin 3 for transmitting as output
	PORTD.DIRCLR = PIN2_bm;		// Set pin 2 for receiving as input
	
    //Enable medium level interrupts for receiver 
    PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	
    USARTD0.CTRLA = USART_RXCINTLVL_MED_gc;
    USARTD0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | 
                    USART_PMODE_DISABLED_gc | 
                    USART_CHSIZE_8BIT_gc;

// Typecast: truncates arbitrary size #define to 8 bit unsigned integer
    USARTD0.BAUDCTRLA = (uint8_t)BSEL9600;                    
// Typecast again to set S
    USARTD0.BAUDCTRLB = (uint8_t)
                         ( (BSCALE9600 << 4)|(BSEL9600>>8) );   
// Enable TX and RX
    USARTD0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;              

    sei();  //Enable global interrupts
	
	PORTQ_DIRSET = 0xA;
	PORTQ_OUTCLR = 0xA;
	
	/*
    PORTQ_DIRSET = 0xF;	//Set PORTQ as output
    while(1)
    {
        PORTQ_OUTTGL = 0xF;	//Toggle PORTQ
        RoughDelay1sec();	//Wait 1 second (roughly)
    }
	*/
}
/**********************************************************
* Name:     ISR(USARTC0_RXC_vect)
* Purpose:  Interrupt routine triggered during a receive on USARTC0.  Function is
*           setup to echo the incoming message.
* Inputs:
* Output:
*************************************************************/
ISR(USARTD0_RXC_vect)
{
    USARTD0.DATA = USARTD0.DATA;	//Echo
}
