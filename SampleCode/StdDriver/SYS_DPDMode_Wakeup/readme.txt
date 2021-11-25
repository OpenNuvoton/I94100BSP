/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to configure ¡§DPD Power-down Mode¡¨ and wakeup function.
	(1)	System will enter DPD power down mode, and could wake up the system by two methods.
	(2)	Wake up by GPIO interrupt from PA.15(SW2 USER).
	(3)	Wake up by TIMER time-out interrupt.

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Compile and execute.
	(3)	Program test procedure ¡V
		1.	First, user could choose which wakeup method to demonstrate. Press ¡¥1¡¦ to choose GPIO wakeup. Press ¡¥2¡¦ to choose TIMER time-out wakeup.
		2.	The program enters power-down mode.
		3.	If user pressed ¡¥1¡¦, press the button ¡§SW2 USER¡¨ to wake up system.
		4.	The system will reset and return to procedure ¡¥1.¡¦.
		5.	If user pressed ¡¥2¡¦, wait for the TIMER time-out to wake up system.
		6.	The system will reset and return to procedure ¡¥1.¡¦. 
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
