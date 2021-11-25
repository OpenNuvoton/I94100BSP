/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use ¡¥DPWM¡¦ to play audio file.
	(1)	Playing audio data from a ¡¥Audio.bin¡¦ file.

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connects to com port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect speaker(s) to DPWM right channel (RP = PC10, RN = PC11) and left channel (LP =PC12, LN = PC13).
	(3)	Compiled to execute.
	(4)	Program test procedure -
		1.	After showing¡¨ Please input any key to play audio.¡¨, input any key.
		2.	Speaker will play audio.
		3.	Message¡¨ Please input any key to play audio.¡¨ will show again, procedure return to ¡¥1.¡¦.

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
