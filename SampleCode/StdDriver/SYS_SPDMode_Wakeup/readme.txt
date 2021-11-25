/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to configure two SPD (Standby Power Down) mode, and two methods to wake up from power down.
	(1)	Using GPIO PA.15 (SW2 USER) rising edge interrupt as wakeup source.
	2)	Using wakeup TIMER time-out interrupt as wakeup source.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Compile and execute.
	(3)	Program test procedure ¡V
		1.	User could choose which SPD mode and which wakeup methods to demonstrate.
		2.	If input is ¡¥0¡¦ and ¡¥1¡¦, program will configure to SPD0 mode. If input is ¡¥2¡¦ and ¡¥3¡¦, program will configure to SPD1 mode.
		3.	If input is ¡¥0¡¦ and ¡¥2¡¦, the wakeup source will set to GPIO rising edge interrupt, by pressing button ¡§SW2 USER¡¨. If input is ¡¥1¡¦ and ¡¥3¡¦, the wakeup source will set to TIMER time-out interrupt.
		4.	If user chooses SPD0 mode, after the wakeup, the system will reset and show ¡§Press any key to continue ...¡¨. Press any key to start the demo again.
		5.	If user chooses SPD1 mode, after the wakeup, the system will reset and return to procedure ¡¥1.¡¦.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
