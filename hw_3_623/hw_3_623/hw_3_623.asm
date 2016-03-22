/*
 * hw_3_623.asm
 *
 *  Created: 9/24/2015 12:17:40 AM
 *   Author: Kin
 Description: This program find the smallest number out of from 32 8-bit unsinged numbers
 */ 

 .include "ATxmega128A1Udef.inc"

 .equ counter = 32						; 32 elements in the table to itereate through
 .equ smallestNum = 0					; Set the smallest number to begin with
 .equ ramBegin = 0x2000

 .DSEG
 .ORG 0x2000
 Table: .dw 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,32

 .CSEG
 .ORG 0x0000							; Leave space for interrupt vectors
	RJMP MAIN


 .ORG 0x0100
 MAIN:
	LDI r16, counter					; counter = 32
	RCALL StoreInRam					; Add table to memory
	LDI YL, Low(ramBegin)
	LDI YH, High(ramBegin)
	LD r17, Y							; ans = 0
	LDI r16, counter					; Reset counter = 32

LOOP: 
	CPI r16, 1
	BREQ ANS
	
	; Compare unsigned values
	LD r18, Y+							; r18 = Table[i]
	AND r17, r18 
	
	SUBI r16, 1							; Decrement counter
	RJMP LOOP

ANS:									; Enter answer in following address
	ST Y, r17							; Enter answer in 33th consec mem spot

DONE:									; End of program
	RJMP DONE	
	

;*************************************************************************************
;                                  Subroutine
;*************************************************************************************
; Description: Strore table in location staring at 0x2000
StoreInRam:
	LDI XL, Low(ramBegin)
	LDI XH, High(ramBegin)
	LDI r21, 1							; incrementing values
	LDI r20, 1							; Value to add
LOOP1:
	CPI r16, 0
	BREQ Exit
	ST X+, r20
	ADD r20, r21
	SUBI r16, 1							; Decrement counter
	RJMP LOOP1

Exit: 
	ret