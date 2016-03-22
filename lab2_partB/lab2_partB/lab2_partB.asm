/*
 * lab2_partB.asm
 *
 *  Created: 9/23/2015 4:15:13 PM
 *   Author: Kinderley Charles
Description: The following program will blink an LED
		     at 2 Khz
 */ 

 .include "ATxmega128A1Udef.inc"

 .equ portEOut = 0x01			; Set bit 0 of Port E as output
 .equ clrPort = 0xFF			; Clear all bits of port
 .equ test = 0x01				; For debugging purposes
 .equ micro250s = 165			; 250 Loops

 .ORG 0x0000					; Code starts running from address 0x0000
	rjmp MAIN					; Jump to Main routine


 ;*******************************PROGRAM START****************************************
 .ORG 0x0100
 MAIN: 
	
	ldi r16, portEOut			; r16 = 0xFF
	sts PORTE_DIRSET, r16		; Set pin0 of PORT E as output
	
	ldi r16, clrPort
	sts PORTE_OUTCLR, r16		; Initialize port E to 0

	ldi r17, test				; LED to light up
	ldi r18, micro250s			; r18 = 250 microSecCounter

 LOOP:
	sts PORTE_OUT, r17			; Port E = r17
	rcall DELAY_250muSec		; Call Delay subroutine

	sts PORTE_OUTCLR, r16		; Turn off LED
	rcall DELAY_250muSec		; Call Delay subroutine

	rjmp LOOP

 DONE:							; End of program
	rjmp DONE

 ;********************************SUBROUTINES*****************************************
 /************************************************************************************
 * Name:     DELAY
 * Purpose:  Delay execution of PORTE_OUT for 250 microSeconds assuming that each ins.
			 takes 0.5 microSeconds.
 * Inputs:   R18
 * Outputs:  None
 * Affected: R18, R18 reset before returning
 ***********************************************************************************/
 DELAY_250muSec:
	DEC r18							; Loop 250 times 1.0 microSeconds
	BRNE DELAY_250muSec
	LDI r18, micro250s
 RET								; return from subroutine