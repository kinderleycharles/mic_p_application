/*
 * Lab 2 Part C
 * Name:		Kinderley Charles
 * Section:		6957
 * TA:			Khaled Hassan
 * Description: The following program displays a different output at a
 *				different frequency based on the sixth bit of an input
 *				entry. If the sixth bit is a one, the output is as 
 *				follow: 0xE7, 0xC3, 0x81, 0xC3 where each output is 
 *				succeeded by a delay of 420ms. If the sixth bit 0, the 
 *				output if a rotated entry with 240ms delay between each
 *				output
 */ 


 .include "ATxmega128A1Udef.inc"


 .equ portFIn = 0x00			; Set all 8 bits of Port F as input
 .equ portEOut = 0xFF			; Set all 8 bits of Port E as output
 .equ clrPort = 0xFF			; Clear all bits of port
 .equ ms240 = 24				; Converstion factor to 24 millisecond
 .equ ms420 = 60				; Converstion factor to 24 millisecond
 .equ tenmilli = 40				; Conversion factor to 10 milliSeconds
 .equ micro250s = 168			; Conversion factor for 250 microSeconds
 .equ test = 0X03				; For debugging purposes
 .equ part1Out = 0x03
 .equ part2Out = 0xE7
 .equ counter = 4				; Number of digit to swap for part 2



 .ORG 0x0000					; Code starts running from address 0x0000
	rjmp MAIN					; Jump to Main routine


 .ORG 0x100						
 Table: .db 0xE7, 0xC3, 0x81, 0xC3

 ;*******************************PROGRAM START****************************************
 .ORG 0x0200
 MAIN: 

	; PORT F set as Input
	LDI r16, portFIn			; r16 = 0x00
	STS PORTF_DIRSET, r16		; Set all 8 pins of port F as input

	; PORT E set as Output
	LDI r16, portEOut			; r16 = 0xFF
	STS PORTE_DIRSET, r16		; Set all 8 pins of PORT E as output
	
	; Initialize PORT E
	LDI r16, clrPort
	STS PORTE_OUTCLR, r16		; Unitialize port E to 0

	ldi r20, ms240				; Delay for 240ms
	ldi r21, ms420				; Delay for 420ms


RESET:
	; Read PORT F value
	LDS r17, PORTF_IN			; r17 = PORT F
	

LOOP:
	CLC							; Clear carry flag for possible trash
	LDI r22, part1Out			; Init port output to 0x03
TEST1:
	LDS r17, PORTF_IN			; r17 = PORT F
	SBRC r17, 6					; if bit6 == 0, Skip next instruction
	RJMP TEST2					; bit6 == 1, Proceed to TEST2
	RCALL KITT_1				; Rotate Output
	RJMP TEST1

TEST2:
	LDI ZL, Low(Table << 1)
	LDI ZH, High(Table << 1)
	CLC	
	CLR r22						; Clear carry flag for possible trash
	LPM r22, Z+					; Init port output to D7
	LDI r23, counter
LTEST2:
	LDS r17, PORTF_IN			; r17 = PORT F
	SBRS r17, 6					; if bit6 == 0, Skip next instruction
	RJMP LOOP					; If bit6 was changed
	RCALL KITT_2
	RJMP LTEST2

DONE:							; End of program
	RJMP DONE




;********************************SUBROUTINES*****************************************
/************************************************************************************
 * Name:     KITT_1
 * Purpose:  Shift the content of R16 left until to make the output rotates
 * Inputs:   R22
 * Outputs:  R22
 * Affected: R22
 ***********************************************************************************/
KITT_1:
	STS PORTE_OUT, r22			; Turn on Light
	RCALL DELAYx240ms			; Delay before the next output
	; Rotate output left
	LSL r22						; r18 = r18*2
	BRCC SKIPA
	SBR r22, 1					; Wrap around if most sig fig was 1
SKIPA:
	RET


/************************************************************************************
 * Name:     KITT_2
 * Purpose:  Shift the content of R16 left until to make the output rotates
 * Inputs:   R22 is the value to output to PORT E
 * Outputs:  R22, R23
 * Affected: R22
 ***********************************************************************************/
KITT_2:
	STS PORTE_OUT, r22			; Turn on Light
	RCALL DELAYx420ms			; Delay before the next output
	DEC r23
	BRNE EXIT_KITT_2
	LDI r23, counter
	LDI ZL, Low(Table << 1)
	LDI ZH, High(Table << 1)
EXIT_KITT_2:
	LPM r22, Z+					; Init port output to D7
	RET

/************************************************************************************
 * Name:     DELAYx240ms
 * Purpose:  Delay for 240 ms
 * Inputs:   R20 is the counter to make this delay subroutine last 240ms
 * Outputs:  None
 * Affected: R18, R19, R20
 ***********************************************************************************/
DELAYx240ms:
	ldi r18, tenmilli
LOOPMILLI:						; Delay for 10 milliSeconds
	ldi r19, micro250s
LOOPMICRO:						; Delay for 250 microSeconds
	dec r19
	brne LOOPMICRO				
	dec r18
	brne LOOPMILLI
	dec r20 
	brne DELAYx240ms			; Delay for 420ms
	ldi r20, ms240				; Reset value of r20
	ret

/************************************************************************************
 * Name:     DELAYx420ms
 * Purpose:  Delay for 420 ms
 * Inputs:   R21 is the counter to make this delay subroutine last 420ms
 * Outputs:  None
 * Affected: R18, R19, R21
 ***********************************************************************************/
DELAYx420ms:
	ldi r18, tenmilli
LOOPMILLI1:						; Delay for 10 milliSeconds
	ldi r19, micro250s
LOOPMICRO1:						; Delay for 250 microSeconds
	dec r19
	brne LOOPMICRO1
	dec r18
	brne LOOPMILLI1
	dec r21 
	brne DELAYx420ms			; Delay for 420ms
	ldi r21, ms420				; Reset value of r20
	ret