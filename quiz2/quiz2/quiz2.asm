/*
 * quiz2.asm
 *
 *  Created: 9/24/2015 7:30:33 PM
 *   Author: Kin
 */ 


 .include "ATxmega128A1Udef.inc"


 .equ portFIn = 0x00			; Set all 8 bits of Port F as input
 .equ portEOut = 0xFF			; Set all 8 bits of Port E as output
 .equ clrPort = 0xFF			; Clear all bits of port
 .equ test = 0x03				; For debugging purposes
 .equ micro250s = 164			; 250 Loops
 .equ micro250s1 = 140

 .ORG 0x0000					; Code starts running from address 0x0000
	rjmp MAIN					; Jump to Main routine


 .ORG 0x0100					; Main routine starting address
 MAIN:

	ldi r16, portFIn			; r16 = 0x00
	sts PORTF_DIRSET, r16		; Set all 8 pins of port F as input

	ldi r16, portEOut			; r16 = 0xFF
	sts PORTE_DIRSET, r16		; Set all 8 pins of PORT E as output
	
	ldi r16, clrPort
	sts PORTE_OUTCLR, r16		; Unitialize port F to 0

	LDI r20, 2					; Output bit 1 in LED

LOOP:
	lds r17, PORTF_IN			; r17 = Input at Port F	
	SBRC r17, 0					; if bit0 == 0, Skip next instruction
	RJMP TEST1					; Jump to Test1 since bit == 1
	
	sts PORTE_OUT, r20			; Turn LED on
	rcall DELAY_250muSec		; Delay for 250 microS
	rcall DELAY_250muSec		; Delay for 250 microS

	sts PORTE_OUTCLR, r16		; Turn off LED
	rcall DELAY_250muSec		; Delay for 250 microS
	rcall DELAY_250muSec		; Delay for 250 microS
	RJMP LOOP

TEST1:
	sts PORTE_OUT, r20			; Turn LED on
	rcall DELAY_250muSec1		; Delay for 250 microS

	sts PORTE_OUTCLR, r16		; Turn off LED
	rcall DELAY_250muSec1		; Delay for 250 microS
	rcall DELAY_250muSec1		; Delay for 250 microS
	rcall DELAY_250muSec1		; Delay for 250 microS
	rjmp LOOP

 DONE:							; End of program
	rjmp DONE


DELAY_250muSec1:
	DEC r18							; Loop 250 times 1.0 microSeconds
	BRNE DELAY_250muSec1
	LDI r18, micro250s1
	RET	

DELAY_250muSec:
	DEC r18							; Loop 250 times 1.0 microSeconds
	BRNE DELAY_250muSec
	LDI r18, micro250s
	RET	