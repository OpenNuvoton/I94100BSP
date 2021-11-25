/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate transmit and receive data from PC terminal through RS232 interface.
    (1) The sample code will print input char on terminal
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Compiled to execute.
    (3) Program test procedure -
         1. Please enter any Character to start 
	 2. When inputting char to terminal screen, RDA interrupt will happen and
            UART0 will print the received char on screen.
         3. Press '0' to exit
    

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    