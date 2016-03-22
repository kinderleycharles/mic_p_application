 /*
 * SCI_Polling.asm
 *
 * Modified: 8 June 2015
 * Authors: Dr. Schwartz & Daniel Frank
 *
 * USART polling example that will Prompt the user with a ? and
 * proceed to echo all characters back to the user.
 * Program will poll the USART rx flag, and when a byte is recieved, 
 * it will store it into the ByteRecieved register
 
  NOTE:
  If you keep the JTAG programmer connected, you will sometimes see an 
  extra prompt and/or extra characters after the ! (or even before the !, 
  and sometimes no ! will be displayed).  If you download the program and 
  then disconnect the JTAG programmer USB cable, then you can run the program
  with no problems by just hitting the reset switch on your board!

*/ 
.include "ATxmega128A1Udef.inc"	
;******************************INITIALIZATIONS***************************************
; Equate statements so we can easily change the BSel and BScale 
;    this will make it very easy to change the baud rate of the USAR
;	 current configuration is a baud rate of 115200 Hz
.EQU BSel = 5
;.EQU BScale = -6	; 115200 Hz
.EQU BScale = -2	; 56000 Hz

.EQU Prompt = '?'
.EQU StringLength = 10
.dseg
.org 0x2000					;where test data will be placed
STRING:		.byte	StringLength

.cseg
.org 0x0000
	rjmp	MAIN	

.org 0x200
MAIN:
	ldi YL, 0xFF	;initialize low byte of stack pointer
	out CPU_SPL, YL
	ldi YL, 0x3F
	out CPU_SPH, YL	

REPEAT:
	ldi	R17, StringLength
	ldi XL, low(STRING)	
	ldi XH, high(STRING)

;	rcall DELAY_500ms	; This can help with extra characters being displayed
	rcall INIT_GPIO
	rcall INIT_USART
	ldi R16, Prompt		;load our Prompt character
						;OUT_CHAR expects the arguments on the stack
LOOP:
	rcall OUT_CHAR		;echo character

	; save value and increment pointer
	rcall IN_CHAR		;read in character

	st X+, R16			;store the last value
	dec	R17
	brne LOOP
	rcall OUT_CHAR		;echo character

	ldi	R16,'!'
	rcall OUT_CHAR		;echo character

;	rcall IN_CHAR		;read in character GARBAGE LINE

;HERE:
;	rjmp HERE

	ldi R16, 'W'		;space
	rcall OUT_CHAR		;echo character
	ldi R16, 'X'		
	rcall OUT_CHAR		;echo character
	ldi R16, 'Y'		;space
	rcall OUT_CHAR		;echo character
	ldi R16, 'Z'		
	rcall OUT_CHAR		;echo character

	ldi R16, 0x20		;space
	rcall OUT_CHAR		;echo character
	rjmp REPEAT			;repeat

; *********************************************
; OUT_CHAR receives a character via R16 and will
;   poll the DREIF (Data register empty flag) until it true,
;   when the character will then be sent to the USART data register.
; SUBROUTINE:   OUT_CHAR
; FUNCTION:     Outputs the character in register R16 to the SCI Tx pin 
;               after checking if the DREIF (Data register empty flag)   
;     			is empty.  The PC terminal program will take this 
;               received data and  put it on the computer screen.
; INPUT:        Data to be transmitted is in register R16.
; OUTPUT:       Transmit the data.
; DESTROYS:     None.
; REGS USED:	USARTD0_STATUS, USARTD0_DATA
; CALLS:        None.

OUT_CHAR:
	push R17

TX_POLL:
	lds R17, USARTD0_STATUS		;load status register
	sbrs R17, 5					;proceed to writing out the char if
								;  the DREIF flag is set
	rjmp TX_POLL				;else go back to polling
	sts USARTD0_DATA, R16		;send the character out over the USART
	pop R17

	ret
; *********************************************
; IN_CHAR polls the receive complete flag and will
;   pass the received character pack to the calling routine in R16.
; SUBROUTINE:   IN_CHAR
; FUNCTION:     Receives typed character (sent by the PC terminal 
;               program through the PC to the PortD0 USART Rx pin) 
;               into register R16.
; INPUT:        None.
; OUTPUT:       Register R16 = input from SCI
; DESTROYS:     R16 (result is transferred in this register)
; REGS USED:	USARTD0_STATUS, USARTD0_DATA
; CALLS:        None
IN_CHAR:

RX_POLL:
	lds R16, USARTD0_STATUS		;load the status register
	sbrs R16, 7					;proceed to reading in a char if
								;  the receive flag is set
	rjmp RX_POLL				;else continue polling
	lds R16, USARTD0_DATA		;read the character into R16

	ret
; *********************************************
; INIT_USART initializes UART 0 on PortD (PortD0)
; SUBROUTINE:   INIT_USART
; FUNCTION:     Initializes the USARTDO's TX and Rx, 
;               56000 (115200) BAUD, 8 data bits, 1 stop bit.
; INPUT:        None.
; OUTPUT:       None
; DESTROYS:     R16
; REGS USED:	USARTD0_CTRLB, USARTD0_CTRLC, USARTD0_BAUDCTRLA,
;               USARTD0_BAUDCTRLB
; CALLS:        None.

INIT_USART:
	ldi R16, 0x18				
	sts USARTD0_CTRLB, R16		;turn on TXEN, RXEN lines

	ldi R16, 0x03
	sts USARTD0_CTRLC, R16		;Set Parity to none, 8 bit frame, 1 stop bit

	ldi R16, (BSel & 0xFF)		;select only the lower 8 bits of BSel
	sts USARTD0_BAUDCTRLA, R16	;set baudctrla to lower 8 bites of BSel 

	ldi R16, ((BScale << 4) & 0xF0) | ((BSel >> 8) & 0x0F)							
	sts USARTD0_BAUDCTRLB, R16	;set baudctrlb to BScale | BSel. Lower 
								;  4 bits are upper 4 bits of BSel 
								;  and upper 4 bits are the BScale. 
	ret
;************************************************
; INIT_GPIO PortD Pin3 for output (PortD0 TX pin) amd
;                 Pin2 for input (PortD0 Rx pin)
; SUBROUTINE:   INIT_GPIO
; FUNCTION:     Must set PortD_PIN3 as output for TX pin 
;               of USARTD0 and initial output value to 1.
;				Also must select PortD bits 2 and 3 to be connected to
;               the USB lines by writing 0 to PortQ bits 1 and 3.
; INPUT:        None.
; OUTPUT:       None
; DESTROYS:     R16
; REGS USED:	PortD_DIR, PortD_OUT, PORTQ_DIR, PORTQ_OUT
; CALLS:        None.
INIT_GPIO:
	ldi R16, 0x08	
	sts PortD_DIRSET, R16	;Must set PortD_PIN3 as output for TX pin 
							;  of USARTD0					
	sts PortD_OUTSET, R16	;set the TX line to default to '1' as 
							;  described in the documentation
	ldi R16, 0x04
	sts PortD_DIRCLR, R16	;Set RX pin for input
	
	ldi R16, 0xA			; PortQ bits 1 and 3 enable and select
	sts PORTQ_DIRSET, R16	;   the PortD bits 2 and 3 serial pins 
	sts PORTQ_OUTCLR, R16   ;   to be connected to the USB lines
	ret
