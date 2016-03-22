/* Input_Port.asm
 *
 * Modified: 6 October 2015
 * Authors:  Dr. Schwartz, Colin Watson, Rachel Johnson
 *
 * This is the minimum code required to setup an input port with
 * address decoding utilizing both the CS of the XMEGA and the CPLD. This 
 * solution assumes the user is providing their own output enable, OE(L), 
 * to the input port's tri-state buffer chip using CS0(L) and RE(L).
 */
.include "ATxmega128A1Udef.inc"
;******************************INITIALIZATIONS***************************************
.set IN_PORT = 0x37E000		; Could use something as simple as 0x6000	
.set IN_PORT_END = 0x37FFFF	; Something like this might be useful (could be a comment)

.org 0x0000	
	rjmp MAIN

.org 0x200
MAIN:
	ldi R16, 0b10110		;Configure the PORTH bits 4, 2 and 1 as outputs. 
	sts PORTH_DIRSET, R16 	;  These are the CS0(L), ALE1(H), and RE(L) outputs. 
							;  (CS0 is bit 4; ALE1 is bit 2; RE is bit 1)
							; see 8385, Table 33-7

	ldi R16, 0b10100		; Since RE(L), and CS0(L) are active low signals, we must set  
	sts PORTH_OUTSET, R16	;   the default output to 1 = H = false. See 8331, sec 27.9.
							;   (ALE defaults to 0 = L = false)
	
	ldi R16, 0xFF			; Set all PORTK pins (A15-A0) to be outputs. As requried	
	sts PORTK_DIRSET, R16	; in the data sheet. See 8331, sec 27.9.

	ldi R16, 0xFF			;Set all PORTJ pins (D7-D0) to be outputs. As requried 
	sts PORTJ_DIRSET, R16	;  in the data sheet. See 8331, sec 27.9.
		
	ldi R16, 0x01			;Store 0x01 in EBI_CTRL register to select 3 port EBI(H,J,K) 
	sts EBI_CTRL, R16		;  mode and SRAM ALE1 mode.

;Reserve a chip-select zone for our input port. The base address register is made up of
;  12 bits for the address (A23:A12). The lower 12 bits of the address (A11-A0) are 
;  assumed to be zero. This limits our choice of the base addresses.

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

	ldi R16, 0x15			; Set to 8K chip select space and turn on SRAM mode, 0x37 E000 - 0x37 FFFF
	sts EBI_CS0_CTRLA, R16					

	ldi R16, byte3(IN_PORT)				; initalize a pointer to point to the base address of the IN_PORT
	sts CPU_RAMPX, r16					; use the CPU_RAMPX register to set the third byte of the pointer

	ldi XH, high(IN_PORT)				; set the middle (XH) and low (XL) bytes of the pointer as usual
	ldi XL, low(IN_PORT)

TEST:
	ld R16, X							; read the input port into R16
	rjmp TEST							; put a breakpoint on me and check R16!