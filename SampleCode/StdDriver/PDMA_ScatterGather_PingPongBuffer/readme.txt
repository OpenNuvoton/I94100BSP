/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to configure PDMA in Scatter-Gather mode to transfer data from memory to memory (RAM to RAM) and using Ping-Pong buffer.
	(1)	This sample will transfer data by looped around two descriptor tables from two different source to the same destination buffer in sequence.
	(2)	Sequence will be table 1 -> table 2-> table 1 -> table 2 -> table 1 -> ... -> until PDMA configuration doesn't be reloaded.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Compile and execute.
	(3)	Program test procedure ¡V
		1.	When showing " test done... ", the PDMA action is success and demo is finished.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
