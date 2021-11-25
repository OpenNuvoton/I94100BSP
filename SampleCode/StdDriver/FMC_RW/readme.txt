/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use FMC read and write function to write data and then read back to verify.
	(1)	R/W LDROM, APROM and Data Flash in this demo sample.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Compile and execute.
	(3)	Program test procedure ¡V
		1.	First, the program will set data flash base address.
		2.	Next, the program will show basic chip information, Company ID, Product ID, User Config0,1 and Data Flash Base Address.
		3.	Program starts to write data LDROM and then read back to verify.
		4.	Program starts to write data APROM and then read back to verify.
		5.	Program starts to write data flash and then read back to verify.
		6.	When showing " FMC Sample Code Completed. ", the demo is finished.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
