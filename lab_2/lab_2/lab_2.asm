/*
 * lab_2.asm
 *
 *  Created: 9/21/2015 11:09:28 AM
 *   Author: Kinderley Charles
Description: This is small program that reads the write a certain output
			 to a define port
 */ 

 // Pseudocode
 // Save registers' content
 //
 // Beginning os Subroutine
 // Clear port content (for possible trash)
 // Set the port as output
 //			Set how many bits you want to be outputted
 // Put value to be outputted in port (Value should be seen now)
 // Repeat the process
 //
 // End of subroutine
 // Restore registers previous content

 .include "ATxmega128A1Udef.inc"


 .equ portFIn = 0x00			; Set all 8 bits of Port F as input
 .equ portEOut = 0xFF			; Set all 8 bits of Port E as output
 .equ clrPort = 0xFF			; Clear all bits of port
 .equ test = 0x03				; For debugging purposes

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

LOOP:
	lds r17, PORTF_IN			; r17 = Input at Port F	
	sts PORTE_OUT, r17			; Port E = r17
	rjmp LOOP

 DONE:							; End of program
	rjmp DONE



;********************************SUBROUTINES*****************************************
 /************************************************************************************
 * Name:     DELAY_240mS
 * Purpose:  Delay execution of an instruction by 240 mS
 * Inputs:   R18
 * Outputs:  None
 * Affected: R18, R18 reset before returning
 ***********************************************************************************/