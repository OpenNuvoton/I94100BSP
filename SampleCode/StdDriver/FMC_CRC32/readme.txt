/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use FMC to get CRC32 checksum of LDROM and APROM data.
	(1)	Using FMC to get ¡¥Company ID¡¦, ¡¥Product ID¡¦, ¡¥User Config¡¦ and ¡¥Data Flash Address¡¦.
	(2)	Using FMC to get calculate CRC32 checksum of LDROM and APROM data.

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connects to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Compiled to execute.
	(3)	Program test procedure -
		1.	Message will show ¡§Company ID ¡K¡¨, ¡§Product ID ¡K¡¨, ¡§User Config 0 ¡K¡¨, ¡§User Config 1 ¡K¡¨ and 
			¡§Data Flash Base Address ¡K¡¨.
		2.	Message will show CRC32 checksum result of LDROM, or print out 
			¡§Failed on calculating LDROM CRC32 checksum!¡¨, if action was failed.
		3.	Message will show CRC32 checksum result of APROM bank 0, or print out 
			¡§Failed on calculating APROM bank0 CRC32 checksum!¡¨, if action was failed.
		4.	Message will show CRC32 checksum result of APROM bank 1, or print out 
			¡§Failed on calculating APROM bank1 CRC32 checksum!¡¨, if action was failed.
		5.	When showing "FMC CRC32 checksum test done. ", the demo is finished.

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
