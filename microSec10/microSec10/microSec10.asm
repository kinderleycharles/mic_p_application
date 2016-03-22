/*
 * microSec10.asm
 *
 *  Created: 9/24/2015 10:11:47 AM
 *   Author: Kin
 Descrition: This program delays and output by 10s
 */ 

 .include "ATxmega128A1Udef.inc"

 .equ portEOut = 0xFF			; Set all 8 bits of Port E as output
 .equ test = 0xFF				; For debugging purposes
 .equ clrPort = 0xFF			; Clear all bits of port
 .equ ms240 = 240
 .equ tenmilli = 40				; Conversion factor to 10 milliSeconds
 .equ micro250s = 167			; Conversion factor for 250 microSeconds


 .ORG 0x000
	RJMP MAIN

.ORG 0x0100
MAIN:
	ldi r16, portEOut			; r16 = 0xFF
	sts PORTE_DIRSET, r16		; Set all 8 pins of PORT E as output
	
	ldi r16, clrPort
	sts PORTE_OUTCLR, r16		; Unitialize port F to 0

	ldi r18, tenmilli
	ldi r17, test				; temp value to output

LOOP:
	sts PORTE_OUT, r17			; Turn on LED
	rcall DELAYx240ms
	
	sts PORTE_OUTCLR, r16		; Turn off LED
	rcall DELAYx240ms

	rjmp LOOP

DONE:							; End of Loop
	RJMP DONE


; Subroutines
DELAYx240ms:		
	ldi r19, micro250s

LOOPMICRO:
	dec r19
	brne LOOPMICRO
	
	dec r18
	brne DELAYx240ms
	ldi r18, tenmilli
	ret