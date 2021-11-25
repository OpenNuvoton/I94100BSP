/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate enter to power down mode and wake-up via VAD(Voice Active Detection).
    (1)  VAD is to rely on DMIC channel0 to do the detection. 
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Compiled to execute.
    (3) Program test procedure - 
        1. Please press 'Enter' to enter to power-down
	2. user can give DMIC a sound
	3. then system can be wake up via VAD
	4. result show  "System wake-up via VAD and end VAD demo."

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) Need to DMIC board