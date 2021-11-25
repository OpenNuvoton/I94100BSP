/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 19/11/26 1:14p $
 * @brief    I2C Driver Sample Code
 *           This is a I2C slave mode demo and need to be tested with a master device.
 *
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

uint32_t slave_buff_addr;
uint8_t g_u8SlvData[256];
uint8_t g_au8RxData[3];

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t g_u8DeviceAddr;
uint8_t g_au8TxData[3];
uint8_t g_u8RxData;
uint8_t g_u8DataLen;

typedef void (*I2C_FUNC)(uint32_t u32Status);

static I2C_FUNC s_I2C0HandlerFn = NULL;

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_IRQHandler(void)
{
    uint32_t u32Status;

    u32Status = I2C_GET_STATUS(I2C0);

    if (I2C_GET_TIMEOUT_FLAG(I2C0)) {
        /* Clear I2C0 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C0);
    } else {
        if (s_I2C0HandlerFn != NULL)
            s_I2C0HandlerFn(u32Status);
        CLK_SysTickDelay(1);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C TRx Callback Function                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_SlaveTRx(uint32_t u32Status)
{
	if (u32Status == 0x60)                    /* Own SLA+W has been receive; ACK has been return */
	{
		g_u8DataLen = 0;
		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI | I2C_CTL_AA);
	} 
	else if (u32Status == 0x80)                 /* Previously address with own SLA address
																								 Data has been received; ACK has been returned*/
	{
		g_au8RxData[g_u8DataLen] = I2C_GET_DATA(I2C0);
		g_u8DataLen++;

		if (g_u8DataLen == 2) 
			slave_buff_addr = (g_au8RxData[0] << 8) + g_au8RxData[1];
		
		if (g_u8DataLen == 3)
		{
			g_u8SlvData[slave_buff_addr] = g_au8RxData[2];
			g_u8DataLen = 0;
		}

		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI | I2C_CTL_AA);
	} 
	else if(u32Status == 0xA8)              /* Own SLA+R has been receive; ACK has been return */
	{
		I2C_SET_DATA(I2C0, g_u8SlvData[slave_buff_addr]);
		slave_buff_addr++;
		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI | I2C_CTL_AA);
	} 
	else if (u32Status == 0xB8)                 /* Data byte in I2CDAT has been transmitted
																								 ACK has been received */
	{
		I2C_SET_DATA(I2C0, g_u8SlvData[slave_buff_addr]);
		slave_buff_addr++;
		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI | I2C_CTL_AA);
	} 
	else if (u32Status == 0xC0)                 /* Data byte or last data in I2CDAT has been transmitted
																								 Not ACK has been received */
	{
		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI | I2C_CTL_AA);
	}	
	else if (u32Status == 0xC8)                 /* Data byte or last data in I2CDAT has been transmitted
																								 ACK has been received */
	{
		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI | I2C_CTL_AA);
	}	
	else if (u32Status == 0x88)                 /* Previously addressed with own SLA address; NOT ACK has
																								 been returned */
	{
		g_u8DataLen = 0;
		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI | I2C_CTL_AA);
	} 
	else if (u32Status == 0xA0)                 /* A STOP or repeated START has been received while still
																								 addressed as Slave/Receiver*/
	{
		g_u8DataLen = 0;
		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI | I2C_CTL_AA);
	} 
	else 
	{
			/* TO DO */
			printf("Status 0x%x is NOT processed\n", u32Status);
	}
}

void I2C0_Init(void)
{
	/* Enable I2C0 clock */
	CLK_EnableModuleClock(I2C0_MODULE);

	/* Set GPA multi-function pins for I2C0 SDA and SCL */
	SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA9MFP_Msk | SYS_GPA_MFPH_PA10MFP_Msk)) | (SYS_GPA_MFPH_PA9MFP_I2C0_SCL | SYS_GPA_MFPH_PA10MFP_I2C0_SDA);

	/* Reset module */
	SYS_ResetModule(I2C0_RST);

	/* Open I2C0 and set clock to 100k */
	I2C_Open(I2C0, 10000);

	/* Get I2C0 Bus Clock */
	printf("I2C clock %d Hz\n", I2C_GetBusClockFreq(I2C0));

	/* Set I2C0 4 Slave Addresses */
	I2C_SetSlaveAddr(I2C0, 0, 0x15, I2C_GCMODE_DISABLE);   /* Slave Address : 0x15 */
	I2C_SetSlaveAddr(I2C0, 1, 0x35, I2C_GCMODE_DISABLE);   /* Slave Address : 0x35 */
	I2C_SetSlaveAddr(I2C0, 2, 0x55, I2C_GCMODE_DISABLE);   /* Slave Address : 0x55 */
	I2C_SetSlaveAddr(I2C0, 3, 0x75, I2C_GCMODE_DISABLE);   /* Slave Address : 0x75 */

	I2C_SetSlaveAddrMask(I2C0, 0, 0x01);
	I2C_SetSlaveAddrMask(I2C0, 1, 0x04);
	I2C_SetSlaveAddrMask(I2C0, 2, 0x01);
	I2C_SetSlaveAddrMask(I2C0, 3, 0x04);

	I2C_EnableInt(I2C0);
	NVIC_EnableIRQ(I2C0_IRQn);
}
void UART0_Init()
{
	/* Enable UART clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Select UART clock source from HXT */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));

	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Reset IP */
	SYS_ResetModule(UART0_RST);

	/* Init UART to 115200-8n1 for print message */
	UART_Open(UART0, 115200);
}
/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
	uint32_t i;

	SYSCLK_INITIATE();

	/* Init UART0 for printf */
	UART0_Init();

	/*
			This sample code sets I2C bus clock to 100kHz. Then, Master accesses Slave with Byte Write
			and Byte Read operations, and check if the read data is equal to the programmed data.
	*/

	printf("+-------------------------------------------------------+\n");
	printf("|               I2C Driver Sample Code(Slave)           |\n");
	printf("+-------------------------------------------------------+\n");

	/* Init I2C0 */
	I2C0_Init();

	/* I2C enter no address SLV mode */
	I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI | I2C_CTL_AA);

	for (i = 0; i < 0x100; i++) {
			g_u8SlvData[i] = i;
	}

	/* I2C function to Slave receive/transmit data */
	s_I2C0HandlerFn=I2C_SlaveTRx;

	printf("\n");
	printf("I2C Slave Mode is Running.\n");

	while(1);
}
