/*
 * milli240.asm
 *
 *  Created: 9/24/2015 12:00:40 PM
 *   Author: Kin
 */ 


  .include "ATxmega128A1Udef.inc"

 .equ portEOut = 0xFF			; Set all 8 bits of Port E as output
 .equ test = 0xFF				; For debugging purposes
 .equ clrPort = 0xFF			; Clear all bits of port
 .equ ms240 = 24				; Converstion factor to 24 millisecond
 .equ ms420 = 60				; Converstion factor to 24 millisecond
 .equ tenmilli = 40				; Conversion factor to 10 milliSeconds
 .equ micro250s = 168			; Conversion factor for 250 microSeconds


 .ORG 0x000
	RJMP MAIN

.ORG 0x0100
MAIN:
	ldi r16, portEOut			; r16 = 0xFF
	sts PORTE_DIRSET, r16		; Set all 8 pins of PORT E as output
	
	ldi r16, clrPort
	sts PORTE_OUTCLR, r16		; Unitialize port F to 0

	ldi r20, ms240
	ldi r21, ms420
	ldi r17, test				; temp value to output
	ldi r18, tenmilli


LOOP:
	sts PORTE_OUT, r17			; Turn on LED
	rcall DELAYx240ms
	
	sts PORTE_OUTCLR, r16		; Turn off LED
	rcall DELAYx420ms

	rjmp LOOP

DONE:							; End of Loop
	RJMP DONE


; Subroutines
KITT_1:




DELAYx240ms:
	ldi r18, tenmilli
LOOPMILLI:		
	ldi r19, micro250s
LOOPMICRO:
	dec r19
	brne LOOPMICRO
	dec r18
	brne LOOPMILLI
	dec r20 
	brne DELAYx240ms
	ldi r20, ms240
	ret

DELAYx420ms:
	ldi r18, tenmilli
LOOPMILLI1:		
	ldi r19, micro250s
	LOOPMICRO1:
	dec r19
	brne LOOPMICRO1
	dec r18
	brne LOOPMILLI1
	dec r21 
	brne DELAYx420ms
	ldi r21, ms420
	ret

