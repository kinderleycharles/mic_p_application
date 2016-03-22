/*
 * Lab4 PART B
 * Name:		Kinderley Charles
 * Section:		6957
 * TA:			Khaled Hassan
 * Description:	This program below write to an external memory that has been added to the board.
 *				It rereads the RAM and copy the address of any memory location that failed to have
 *				a the set values copied over	
 */ 

 .include "ATxmega128A1Udef.inc"

 .SET CS0_BASE = 0x8000						; Base address
 .SET CS1_BASE = 0x470000					; Base address of CS1
 .SET RAM = 0x472000						; External Ram begin
 .set RAM_END = 0X47A000					; End of RAM
 .EQU STACK_ADDR = 0x3FFF					; Address to initialize Stack
 .SET TEST = 0x472000						


 .ORG 0x0000								; Beginning of program
	RJMP MAIN								; Leave space for interrupt vectors

.ORG 0x100
KEYS:		.DB 0x01, 0x04, 0x07, 0x0E, 0x02, 0x05, 0x08, 0x00, 0x03, 0x06, 0x09, 0x0F, 0x0A, 0x0B, 0x0C, 0x0D, 0xFF

.ORG 0x0200
MAIN:
	// INITIALIZE STACK POINTER
	LDI R16, LOW(STACK_ADDR)
	STS CPU_SPL, R16
	LDI R16, HIGH(STACK_ADDR)
	STS CPU_SPH, R16

	// PORT CONFIGURATION FOR KEYPAD
	LDI R16, 0xF0							; Setting MOST sig nibble in port E as input
	STS PORTE_DIRCLR, R16					; PORTE[7:4] set as input
	LDI R16, 0x0F							; Setting LEAST sig nibble in port E as output
	STS PORTE_DIRSET, R16					; PORTE[3:0] set as output

	LDI R16, 0b00011000						; Pull-up resistor configuration
	STS PORTE_PIN7CTRL, R16					; Add pull-up resistor to pin 7
	STS PORTE_PIN6CTRL, R16					; Add pull-up resistor to pin 6
	STS PORTE_PIN5CTRL, R16					; Add pull-up resistor to pin 5
	STS PORTE_PIN4CTRL, R16					; Add pull-up resistor to pin 4

	// EBI PORT CONFIGURATION
	LDI R16, 0b110111						; Configure the PORTH bits 5, 4, 2 and 1 as outputs. 
	STS PORTH_DIRSET, R16 					; These are the CS1(L), CS0(L), ALE(H), RE(L) and WE(L) outputs.

	LDI R16, 0b110011						; Since WE(L), RE(L), CS0(L), and CS1(L) are active low signals, we must set  
	STS PORTH_OUTSET, R16					; the default output to 1 = H = false. ALE defaults to 0 = L = false
	
	LDI R16, 0xFF							; Set all PORTK pins (A15-A0) to be outputs. As requried	
	STS PORTK_DIRSET, R16					; in the data sheet. See 8331, sec 27.9.

	LDI R16, 0xFF							; Set all PORTJ pins (D7-D0) to be outputs. As requried 
	STS PORTJ_DIRSET, R16					; in the data sheet. See 8331, sec 27.9.
		
	LDI R16, 0x01							; Store 0x01 in EBI_CTRL register to select 3 port EBI(H,J,K) 
	STS EBI_CTRL, R16						; mode and SRAM ALE1 mode.

	// CS0 CONFIGURATION
	LDI ZH, high(EBI_CS0_BASEADDR)			; Set base address of Chip Select
	LDI ZL, low(EBI_CS0_BASEADDR)
	LDI R16, byte2(CS0_BASE)				
	ST Z+, R16	
	LDI R16, byte3(CS0_BASE)
	ST Z, R16

	LDI R16, 0x15							; Set Chip Select Size							
	STS EBI_CS0_CTRLA, R16					; Set to 8K chip select space and turn on SRAM mode, 0x8000 - 0x9FFF					
	
	// CS1 CONFIGURATION
	LDI ZH, high(EBI_CS1_BASEADDR)			; Set base address of Chip Select
	LDI ZL, low(EBI_CS1_BASEADDR)
	LDI R16, byte2(CS1_BASE)				
	ST Z+, R16	
	LDI R16, byte3(CS1_BASE)
	ST Z, R16					

	LDI R16, 0x21							; Set Chip Select Size							
	STS EBI_CS1_CTRLA, R16					; Set to 64K chip select space and turn on SRAM mode, 0x47 0000 - 0x47 FFFF	


	LDI R16, byte3(RAM)						; initalize a pointer to point to the base address of the IN_PORT
	STS CPU_RAMPX, r16						; use the CPU_RAMPX register to set the third byte of the pointer
	LDI XH, high(RAM)						; set the middle (XH) and low (XL) bytes of the pointer as usual
	LDI XL, low(RAM)

	STS CPU_RAMPY, R16						; Set end upper range
	LDI YH, HIGH(RAM_END)
	LDI YL, LOW(RAM_END)
	
	// WRITE 0XAA TO ALL 32K ADDRESSES OF THE EXTERNAL RAM
	LDI R16, 0xAA							; Value to write to memory
WRITE:
	ST X+, R16								; Store to mem location and increment to next location
	CP XH, YH								; Check if end of RAM is reached
	BRNE WRITE
	CP XL, YL
	BRNE WRITE


	// REREAD ALL THE 32K ADDRESS AND COPY LOWEST 2 BYTES OF CORRUPTED ADDRESSES
	// CORRUPT ADDRESS: ADDRESSES WHICH VALUES ARE NOT 0XAA
	LDI XH, high(RAM)						; Set X as beginning of External RAM
	LDI XL, low(RAM)
	LDI ZL, LOW(0x2200)						; Set to storing location for corrupted addresses
	LDI ZH, HIGH(0x2200)
	LDI R18, 100							; Maximum number of addresses to copy
RE_READ:
	LD R17, X								; R17 = value read at mem location in external RAM
	RCALL WRITE2SRAM						; Call subroutine to copy to RAM if possible
	LD R17, X+								; Dummy instruction to increment address
	CP XH, YH								; Check it end of RAM is reached
	BRNE RE_READ
	CP XL, YL
	BRNE RE_READ

	
	// WRITE 0X55 TO ALL 32K ADDRESSES OF THE EXTERNAL RAM
	LDI XH, high(RAM)
	LDI XL, low(RAM)
	LDI R16, 0x55
WRITE0x55:
	ST X+, R16								; Store to mem location and increment to next location
	CP XH, YH								; Check if end of RAM is reached
	BRNE WRITE0x55
	CP XL, YL
	BRNE WRITE0x55


	// REREAD ALL THE 32K ADDRESS AND COPY LOWEST 2 BYTES OF CORRUPTED ADDRESSES
	// CORRUPT ADDRESS: ADDRESSES WHICH VALUES ARE NOT 0X55
	LDI XH, high(RAM)						; Set X as beginning of RAM address
	LDI XL, low(RAM)
	LDI ZL, LOW(0x2400)						; Set Z as storing location to move data
	LDI ZH, HIGH(0x2400)
	LDI R18, 100							; Maximum number of addresses to rewrite
RE_READ1:
	LD R17, X								; Read value at External mem location
	RCALL WRITE2SRAM						; Call subroutine to copy to RAM if possible
	LD R17, X+								; Dummy instruction to increment address
	CP XH, YH								; Check if end of RAM memory is reached
	BRNE RE_READ1
	CP XL, YL
	BRNE RE_READ1


DONE:										; End of program
	RJMP DONE




/*******************************************************************************************************
                                              SUBROUTINES
 *******************************************************************************************************
 * Name:     WRITE2SRAM
 * Purpose:  Scan a keypad to figure which key is pressed
 * Inputs:   R16, R17, R18
 * Outputs:  None
 * Affected: X
 *******************************************************************************************************/
 WRITE2SRAM:
	CPI R18, 0									; Do not add if 100 address already writtent to internal mem
	BREQ EXIT_WRITE2SRAM
	CP R17, R16									; Check if value is same as R16 value
	BREQ EXIT_WRITE2SRAM
	ST Z+, XL									; Add corrupted address (LOW)
	ST Z+, XH									; Add corrupted address (LOW)
	DEC R18										; Decrement # of data copied

EXIT_WRITE2SRAM:
	RET

	