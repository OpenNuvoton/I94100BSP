/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use FMC IAP function.
(1)	Using two project to demonstrate IAP.
1.	First project generates ¡¥.bin¡¦ file for LDROM. Location: ..\StdDriver\FMC_IAP\KEIL\fmc_ld_code.bin
2.	Second project will demonstrate IAP instruction to program LDROM from APROM.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
(2)	Open ¡¥fmc_ld_code¡¦ project and compile to generate ¡¥fmc_ld_code.bin¡¦.
(3)	Open ¡¥fmc_ap_main¡¦ project then compile and execute.
(4)	Program test procedure ¡V
1.	First, the sample program will assert that it runs in APROM.
2.	Next, user needs to select which action to do. Input ¡§0¡¨ to load IAP code LDROM.
3.	After loading image to LDROM, input ¡§1¡¨ to change VECMAP and branch to LDROM (running in LDROM).
4.	After showing ¡§Press any key to branch to APROM¡K¡¨, input any key to return to APROM.
5.	Program will go back to procedure ¡¥2¡¦, and the demonstration is finished at this point. User can repeat the demonstration.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
