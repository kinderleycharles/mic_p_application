/*
 * Table_Load_Example.asm
 *
 *  Created: 9/15/2015 7:05:44 PM
 *   Author: Kinderley Charles

 This program will load Table_Size values from a table at location 0x100 
and sum them all together and store the result into the "Total" variable.
 */ 


;Definitions for all the registers in the processor. ALWAYS REQUIRED.
;You can view the contents of this file in the "Solution Explorer" window 
;  under "Dependencies"

.nolist	; This works, but the below file can't be removed for lss file.
.include "ATxmega128A1Udef.inc"
.list 

.equ Table_Size = 10	;set the table size here
.equ Tab_Begin = 0x100	; Dummy practice to see if I can saave address in var
.def counter = r17		;define another name for r17 to hould our current count
.def sum = r0			;define another name for r0 to hold our current sum

.org 0x0000				;Place code at address 0x0000
	rjmp MAIN			;Relative jump to start of program

.org 0x100							;Place table at address 0x100.  This will appear
									;  to be at 0x200 in Atmel Studio memory window
Table: .db 1,2,3,4,5,6,7,8,9,10		;Define the table in bytes
Hello: .dw 0x4744					;Note (in memory window) that this is a little 
									;  endian processor
GoodBye: .db 0x37					;Assembler gives a warnding of misallignment since 
									;  program memory is 16-bits wide.  There will be 
									;  padding of 0x00
		.db 0xAB, 0xCD
		.db 077						;Octal 77 = 0x3F = decimal 63
		.db "hello! 'there'"		;ASCII strings do NOT terminate with null (0)
		.db 'G', 'O', 'O', 'D'		;Single quotes (apostrophes) are for single characters
.dseg		;We can only place variables (defined by .byte) in a DATA segment 
			;   (defined by the .DSEG directive). Everything is .CSEG by default.
			;   .DSEG tells the assembler the following is to be placed into the 
			;   data memory map (where SRAM and registers reside). If we did not 
			;	do this, we would not be able to write to Total (see below) since it 
			;	would be located in .CSEG, which is non-volatile flash memory.
.org 0x2000		;.org at address 0x2000 because that is where the internal SRAM begins. 
Total: .byte 1	;Reserve one variable byte and name it Total
Outs:  .BYTE 3	;Notice that assembly language is case insensitive
	
.cseg			;Everything is .CSEG by default, but since we used .DSEG earlier, 
				;  we must redeclare the following as a code segment
.org 0x200		;Place our program at 0x200 
MAIN:
	;We have to use the Z register to pull values from the Flash (Program Memory)  
	;  section of memory. The first (least significant) bit of the Z register 
	;  is used to tell the processor whether or not to load a lower byte or upper byte 
	;  from program memory. SRAM memory is only 8 bits so we dont normaly run into this 
	;  problem. We have to shift left one bit to preserve the address due to this.
	;  So, for example, address 0xFF = 1111 1111 ==> 1 1111 1110 and 1 1111 1111
	ldi ZL, low(Table << 1)		;load the low byte of the Table address into ZL register
	ldi ZH, high(Table << 1)	;load the high byte of the Table address into ZH register
	;You can add to with an instruction like: adiw Z, 0x25 (number to add must be < 64=0x3F)

	ldi counter, Table_Size		;load the table size into our counter register
; ***NOTE***: You CAN NOT ldi for r0-r15
	clr sum		;Clear our sum register to zero. NEVER make assumptions about 
				;  initial values.
	ldi YL, low(Outs)	;not necessary ##
	ldi YH, high(Outs)	;not necessary ##

LOOP:
	lpm r16, Z+		;load the value from the table (using Z as pointer) into r16 and 
					;  then increment table pointer (Z)
	st  Y+, r16 	;not necessary ##

	add sum, r16
	dec counter		;decrement our counter
	brne LOOP		;if the counter is not equal to zero, branch back to LOOP
	sts Total, sum	;store the sum variable directly to our Total variable
DONE:								
	rjmp DONE						;infinite loop because we are done!