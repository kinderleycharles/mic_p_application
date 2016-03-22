/*
 * lab_1.asm
 *
 *  Created: 9/16/2015 1:46:50 AM
 *   Author: Kinderley Charles
 * 
 *	This program should filter the data in a given table such that only characters with
 *	a hex value less than or equal to 0x21 OR greater than 0x40 are copied to new table
 *	stored at 0x2B10
 */ 

 .include "ATxmega128A1Udef.inc"

; Setting some variables for future reference
.equ lTres = 0x21		; Filtering low treshold (21)
.equ hTres = 0x40		; Filtering high treshold (40)
.equ Table_end = 0x00	; Setting the end of the array to be a Null value.equ upperA = 0x41		; Upper Case A.equ upperZ = 0x5B		; Character following Z.equ UpToLow = 0x20		; Upper to Lowercase converting factor
.org 0x0000		; Placing a jump instruction at address 0x0000 to leave space for vertor interrupts
	rjmp MAIN	; relative jump to start program

/*GIVEN TABLE*/
.org 0x4230		; Place given table in address 0x4230 (should be in data memory)
GivenTable: .db "G2/o+ #4@G/)a4&<t*o(.r?s!",Table_end

/*FILTERED TABLE*/
.dseg
.org 0x2B10
newTable: .byte 1	; Save a byte to start storing the new filtered table

/*PROGRAM*/
.cseg
.org 0x0100		; Placing the Main routine at address 0x100
MAIN:	
	ldi ZL, low(GivenTable << 1)	;load the low byte of the Table address into ZL register
	ldi ZH, high(GivenTable << 1)	;load the high byte of the Table address into ZH register
	ldi YL, low(newTable)			;load the low byte of the filtered Table address into YL register
	ldi YH, high(newTable)			;load the high byte of the filtered Table address into YH register

LOOP:
	ldi r18, 0x61
	ldi r19, 0x5B
	ldi r20, 0x20
	ldi r16, Table_end	; r16 initialize to end of Given Table
						; r16 will be used as temporary variable to load values
	lpm r17, Z+			; Load element from GivenArray (stored in memory) and increment to next element in array
	
	; Test #1: End of Table test
	cp r17, r16			; Check if ending of given table is reached
	brne Test2			; GivenTable[i] != NULL
	st Y+, r17			; Add ending to new filtered table
	breq EXIT			; GivenTable[i] == NULL, Exit loop

	; Test #2: is element == 21
Test2:	ldi r16, lTres	; Load temporary var with 21
	cp r17, r16			
	brne Test3			; GivenTable[i] != 21, proceed to other tests  	 
	st Y+, r17			; GivenTable[i] == 21, FilterTable[i] = GivenTable[i]
	rjmp Iterate

	; Test #3: is element < 21
Test3: cp r17, r16
	brpl Test4			; GivenTable[i] > 21, proceed to other tests
	st Y+, r17			; Element < 21, Add to filtered table, then increment pointer	
	rjmp Iterate

	; Test #4: is element > 40
Test4:	ldi r16, hTres	; Load temporary var with 40
	cp r16, r17
	breq Iterate		; GivenTable[i] == 40
	brpl Iterate		; GivenTable[i] < 40


	cp r17, r18			; is it A or something greater than a
	brge Up
	; Convert to lowercase
	add r17, r20

Up: st Y+, r17			; GivenTable[i] > 40, FilterTable[i] = GivenTable[i]
	
Iterate:
	rjmp LOOP

EXIT:					; Infinite loop ending the program
	rjmp EXIT