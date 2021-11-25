/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to implement use I2C Serial Interface 
    (1) Implement how to wake up MCU from Power-down mode through I2C interface.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect PA9(Master) < -- > PA9(Slave), PA10(Master)< -- > PA10(Slave)      
    (3) The pull up resistance need to connect PA.10, PA.9 and 3.3V.
    (4) Compiled to execute.
    (5) Program test procedure -
          1. Press any key 
          2. send message to wake up slave
          3. Access to the corresponding address Slave
          4. Access Slave with address mask
          5. done
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) This sample code needs to work with I2C_Wakeup_Master.
    (2) This sample need to pull up resistance.
    (3) Slave must running first.