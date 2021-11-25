/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate I2S data transfer.
    (1) Implement how I2S works in Master mode..
    (2) This sample code will transmit data to Slave and receive the data processed by Slave.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect Pins: 
         Master PD6(BCLK) < - > Slave PD6(BCLK)
         Master PD5(DO) < - > Slave PD4(DI)
         Master PD4(DI) < - > Slave PD5(DO)
         Master PD3(LRCK) < - > Slave PD3(LRCK)
    (3) Run the slave demo first to wait for master transmission 
    (4) Compiled to execute.
    (5) Program test procedure -
		1. Press any key to start
		2. The master will transmit data in "ai32OutBuf" to slave, and slave sample code will
		   return the received data from master.
		3. The Master sample code will show data received from slave.
    
      
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    	(1) This sample code needs to work with I2S_Slave.
	(2) If the actual HXT is not 12.288MHz on the board, please change the value of __HXT in "system_I94100.h" 
		and check the PLL setting accroding to "SYSCLK_PLL_CLK" description.
	(3) If the PLL and HIRC setting are changed in "ConfigSysClk.h", please check if the clock source of I2S
		is compliant with the sampling rate.