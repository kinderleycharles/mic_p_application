/* lab3.asm
 *
 * Modified:	6 October 2015
 * Authors:		Kinderley Charles
 * Section:		6957
 * TA:			Khaled Hassan
 * Description: This program reads a user input from an Input port expansion
 *				output the input to an Output port expansion, shift the input right, 
 *				delay for 2 sec. Then it repeats that for 8 times 
 */



.include "ATxmega128A1Udef.inc"
;******************************INITIALIZATIONS***************************************
.set IN_PORT = 0x8000		; Base address
.set IN_PORT_END = 0x9FFF	; Final address of mirrod port
.equ COUNTER = 8			; Loop 8 times
.equ d250micro = 250		; Counter to delay for 250 microseconds
.equ secCounter = 22		; Counter to achieve 2 seconds delay


.org 0x0000	
	rjmp MAIN

.org 0x200
MAIN:
	ldi R16, 0b10011		; Configure the PORTH bits 4, 2 and 1 as outputs. 
	sts PORTH_DIRSET, R16 	;  These are the CS0(L), RE(L) and WE(L) outputs. 
							;  (CS0 is bit 4; RE is bit 2; WE is bit 1)
							; see 8385, Table 33-7

	ldi R16, 0b10011		; Since RE(L), and CS0(L) are active low signals, we must set  
	sts PORTH_OUTSET, R16	;   the default output to 1 = H = false. See 8331, sec 27.9.
							;   (ALE defaults to 0 = L = false)
	
	ldi R16, 0xFF			; Set all PORTK pins (A15-A0) to be outputs. As requried	
	sts PORTK_DIRSET, R16	; in the data sheet. See 8331, sec 27.9.

	ldi R16, 0xFF			;Set all PORTJ pins (D7-D0) to be outputs. As requried 
	sts PORTJ_DIRSET, R16	;  in the data sheet. See 8331, sec 27.9.
		
	ldi R16, 0x01			;Store 0x01 in EBI_CTRL register to select 3 port EBI(H,J,K) 
	sts EBI_CTRL, R16		;  mode and SRAM ALE1 mode.

;Initialize the Z pointer to point to the base address for chip select 0 (CS0) in memory
	ldi ZH, high(EBI_CS0_BASEADDR)
	ldi ZL, low(EBI_CS0_BASEADDR)
	
;Load the middle byte (A15:8) of the three byte address into a register and store it as the 
;  LOW Byte of the Base Address, BASEADDRL.  This will store only bits A15:A12 and ignore 
;  anything in A11:8 as again, they are assumed to be zero. We increment the Z pointer 
;  so that we can load the upper byte of the base address register.
	ldi R16, byte2(IN_PORT)				
	st Z+, R16							; 

;Load the highest byte (A23:16) of the three byte address into a register and store it as the 
;  HIGH byte of the Base Address, BASEADDRH.
	ldi R16, byte3(IN_PORT)
	st Z, R16

	ldi R16, 0x15						; Set to 8K chip select space and turn on SRAM mode, 0x37 E000 - 0x37 FFFF
	sts EBI_CS0_CTRLA, R16					

	ldi R16, byte3(IN_PORT)				; initalize a pointer to point to the base address of the IN_PORT
	sts CPU_RAMPX, r16					; use the CPU_RAMPX register to set the third byte of the pointer

	ldi XH, high(IN_PORT)				; set the middle (XH) and low (XL) bytes of the pointer as usual
	ldi XL, low(IN_PORT)


; *******************************************************************************************************
PROGRAM:
	; Initialize the program
	ldi R16, COUNTER					; temp = 8
	ld R17, X							; read the input port into R17

LOOP:
	st X, R17							; Output to LED
	lsr R17								; Shift right
	
	ldi R18, secCounter					; Counter for the delay
	rcall DELAYx2s						; Delay for 2 seconds

	dec R16								; counter--
	brne LOOP							; Loop if counter != 0

	rjmp PROGRAM						


/*******************************************************************************************************
                                              SUBROUTINES
 *******************************************************************************************************
 * Name:     DELAYx2s
 * Purpose:  Delay for 2 seconds
 * Inputs:   R18
 * Outputs:  None
 * Affected: R18, R19, R20
 *******************************************************************************************************/
DELAYx2s:		
	ldi r20, d250micro					; R20 = 250
LOOPMICRO1:
	ldi r19, d250micro					; R19 = 250
LOOPMICRO:								; Loop 250 times
	dec r19								; 2 instructions execution = 1 micoSec in total
	brne LOOPMICRO
	dec r20
	brne LOOPMICRO1						; Loop another 250 times
	dec r18
	brne DELAYx2s						; Loop r18 ~ 20 to get 2 seconds
	ret
