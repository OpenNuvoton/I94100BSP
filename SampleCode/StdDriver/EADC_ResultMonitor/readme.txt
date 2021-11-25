/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use EADC to get conversion result and compared to constant value.
	(1)	Single end input from channel 2.
	(2)	Compare channel 2 conversion data to a constant value which could be modified by user.
	(3)	Using comparator 0 and comparator 1.

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connects to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect PA2 to the node to be compared.
	(3)	Compiled to execute.
	(4)	Program test procedure -
		1.	Message will show which comparator is triggered to identify the conversion value is 
			greater (comparator 1) or lesser (comparator 0) than the constant value.
		2.	When showing " Exit EADC sample code. ", the demo is finished.

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
