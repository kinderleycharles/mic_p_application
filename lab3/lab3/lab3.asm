/*
 * lab3.asm
 *
 *  Created: 10/6/2015 8:10:09 PM
 *   Author: Kinderley Charles
 *
 * Description: This is a program that expand I/O port
 *				by adding an input and output port that are
 *				both mapped at address $8000 and mirrored to $9FFFs
 */ 


 .include "ATxmega128A1Udef.inc"
 ;******************************INITIALIZATIONS***************************************
 .set IO_PORT = 0x008000		; Beginning of component in memory
 .set IO_PORT_END = 0x009FFF	; End of component in memory

 .org 0x0000					; Beginning of program
	rjmp MAIN					; Leave space for interrupt vectors

 
 .org 0x200
 MAIN:

