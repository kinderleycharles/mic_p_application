/*
 * new_get.asm
 *
 *  Created: 9/15/2015 6:15:20 PM
 *   Author: Kinderley Charles
 *
 *
 * This program shows how to initialize a GPIO port on the Atmel 
 (Port D for this example) and demonstrates various ways to write to 
 a GPIO port.  The output will blink LEDs at the bottom left of the 
 uPAD, labeled D5.  PortD0, PortD1, and PortD4 are the red, green, 
 and blue LEDs,  respectively.  Note that these LEDs are active-low.
 */

;Definitions for all the registers in the processor. ALWAYS REQUIRED.
;View the contents of this file in the Processor "Solution Explorer" 
;   window under "Dependencies"
.include "ATxmega128A1Udef.inc"

.equ	BIT0 = 0x01
.equ	RED = ~(BIT0)
.equ	BIT1 = 0x02
.equ	GREEN = ~(BIT1)
.equ	BIT4 = 0x10
.equ	BLUE = ~(BIT4)
.equ	BIT410 = 0x13
.equ	WHITE = ~(BIT410)
.equ	BIT40 = 0x11
.equ	PINK = ~(BIT40)
.equ	BLACK = 0xFF

.ORG 0x0000					;Code starts running from address 0x0000.
	rjmp MAIN				;Relative jump to start of program.

.ORG 0x0100					;Start program at 0x0100 so we don't overwrite 
							;  vectors that are at 0x0000-0x00FD 
MAIN:
	ldi R16, BIT410			;load a four bit value (PORTD is only four bits)
	sts PORTD_DIRSET, R16	;set all the GPIO's in the four bit PORTD as outputs
; Notice that the 3 LEDs (RED, GREEN, and BLUE) are all now on, creating white

; The following code shows different ways to write to the GPIO pins.

; Turn on each of the primary colored LEDs in turn, then use some combinations
	; These instructions sends the value in R16 to the PORTD pins. 
	; Since the LEDs are wired as active-low, an R16 = RED = 0xFE = 0b1111 1110 
	; will turn the RED LED on.
	ldi	R16, RED
	sts PORTD_OUT, R16 		;send the value in R16 to the PORTD pins	

	ldi	R16, GREEN
	sts PORTD_OUT, R16
	ldi	R16, BLUE
	sts PORTD_OUT, R16
	ldi	R16, WHITE
	sts PORTD_OUT, R16
	ldi	R16, PINK
	sts PORTD_OUT, R16

	ldi	R16, BLACK
	sts PORTD_OUT, R16
; Why do you think the D5 LED is now dimly on?  See the uPAD schematic!

	ldi	R16, BLUE
	sts PORTD_OUT, R16

; Notice that the OUTSET makes the LED go off, since the LED is active-low
	ldi	R16, BIT4			; BIT4 = ~BLUE
	sts PORTD_OUTSET, R16	; Since active-high LED, this will turn off the LED
; Why do you think the D5 LED is now dimly on?  See the uPAD schematic!

; Notice that the OUTCLR makes the LED go on, since the LED is active-low
	ldi	R16, BIT4			; BIT4 = ~BLUE
	sts PORTD_OUTCLR, R16

; Notice that the OUTTGL toggles the value of a PORT pin (in this case the BLUE LED)
LOOP:
	sts PORTD_OUTTGL, R16

	rjmp LOOP				;repeat forever!