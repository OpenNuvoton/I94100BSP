/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to implement use timer controller
    (1) Implement how to use timer0 to create various delay time.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Compiled to execute.
    (3) Program test procedure -
	1. Timer1 to check Timer0 TIMER_Delay API delay time is reasonable or not.
        2. Delay time includes 100 ms, 200 ms, 300 ms, 400 ms and 500 ms.
        3. finally, show check delay time result is pass or fail and "Check TIMER_Delay API delay time done"
  
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
