/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to implement use timer controller
    (1) Use timer0 periodic time-out interrupt event to wake up system.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Compiled to execute.
    (3) Program test procedure -
         1. system enter to power down mode
         2. Timer0 interrupt counts is reaching 3.
         3. System will be wake-up while Timer0 interrupt counts is reaching 4.
    

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) Don't use debug mode.otherwise, The system does not work properly.