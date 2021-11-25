/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use FMC multiword program function.
	(1)	Using two project to demonstrate.
		1.	First project generates ¡¥.bin¡¦ file for multi word program image. Location: ..StdDriver\FMC_MultiWordProgram\KEIL\multi_word_prog.bin
		2.	Second project will multi word program the APROM.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Open ¡¥multi_word_prog¡¦ project and compile to generate ¡¥multi_word_prog.bin¡¦.
	(3)	Open ¡¥main_loader¡¦ project then compile and execute.
	(4)	Program test procedure ¡V
		1.	First, the program will load ¡¥multi_word_prog.bin¡¦ image to SRAM address 0x4000.
		2.	Next, the program will be running the ¡¥multi_word_prog.bin¡¦ code. It will program address 0x00000 to 0x20000 using multi word function.
		3.	Messages ¡§Multiword program APROM page ¡K(address)¡¨, programing information will be showing.
		4.	When showing " Multi-word program demo done. ", the demo is finished. 
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
