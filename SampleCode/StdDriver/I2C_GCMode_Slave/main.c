/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/21 10:04a $
 * @brief
 *           Show a Slave how to receive data from Master in GC (General Call) mode.
 *           This sample code needs to work with I2C_GCMode_Master.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

volatile uint32_t slave_buff_addr;
volatile uint8_t g_au8SlvData[256];
volatile uint8_t g_au8SlvRxData[3];
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint8_t g_u8DeviceAddr;
volatile uint8_t g_au8SlvTxData[3];
volatile uint8_t g_u8SlvDataLen;
volatile uint8_t g_u8SlvEndFlag = 0;

typedef void (*I2C_FUNC)(uint32_t u32Status);

static I2C_FUNC s_I2C0HandlerFn = NULL;

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_IRQHandler(void)
{
    uint32_t u32Status;

    u32Status = I2C_GET_STATUS(I2C0);

    if(I2C_GET_TIMEOUT_FLAG(I2C0))
    {
        /* Clear I2C0 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C0);
    }
    else
    {
        if(s_I2C0HandlerFn != NULL)
            s_I2C0HandlerFn(u32Status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C GC mode Rx Callback Function                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_GCSlaveRx(uint32_t u32Status)
{
  uint8_t u8Temp;
  
    if(u32Status == 0x70)                      /* Reception of the general call address and one more data byte;
                                                                        ACK has been return */
    {
        g_u8SlvDataLen = 0;
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
    }
    else if(u32Status == 0x90)                 /* Previously addressed with General Call; Data has been received
                                                   ACK has been returned */
    {
      u8Temp = I2C_GET_DATA(I2C0);
        g_au8SlvRxData[g_u8SlvDataLen] = u8Temp;
        g_u8SlvDataLen++;

        if(g_u8SlvDataLen == 2)
        {
          u8Temp = (g_au8SlvRxData[0] << 8);
            slave_buff_addr = u8Temp + g_au8SlvRxData[1];
        }
        if(g_u8SlvDataLen == 3)
        {
          u8Temp = g_au8SlvRxData[2];
            g_au8SlvData[slave_buff_addr] = u8Temp;
            g_u8SlvDataLen = 0;
        }
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
    }
    else if(u32Status == 0x98)                 /* Previously addressed with General Call; Data byte has been
                                                   received; NOT ACK has been returned */
    {
        g_u8SlvDataLen = 0;
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
    }
    else if(u32Status == 0xA0)                 /* A STOP or repeated START has been received while still addressed
                                                   as SLV receiver */
    {
        g_u8SlvDataLen = 0;
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
        if(slave_buff_addr == 0xFF)
        {
            g_u8SlvEndFlag = 1;
        }
    }
    else
    {
        /* TO DO */
        printf("Status 0x%x is NOT processed\n", u32Status);
    }
}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
	 /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);
	 /* Select UART module clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	  /* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

    /* Reset IP */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
}

void I2C0_Init(void)
{
	/* Enable I2C0 module clock */
	CLK_EnableModuleClock(I2C0_MODULE);

	CLK_SetModuleClock(I2C0_MODULE, 0, 0);

	/* Set GPA multi-function pins for I2C0 SDA and SCL */
	SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA9MFP_Msk | SYS_GPA_MFPH_PA10MFP_Msk)) | (SYS_GPA_MFPH_PA9MFP_I2C0_SCL | SYS_GPA_MFPH_PA10MFP_I2C0_SDA);

	/* Reset module */
	SYS_ResetModule(I2C0_RST);

	/* Open I2C module and set bus clock */
	I2C_Open(I2C0, 100000);

	/* Get I2C0 Bus Clock */
	printf("I2C clock %d Hz\n", I2C_GetBusClockFreq(I2C0));

	/* Set I2C 4 Slave Addresses and Enable GC Mode */
	I2C_SetSlaveAddr(I2C0, 0, 0x15, 1);   /* Slave Address : 0x15 */
	I2C_SetSlaveAddr(I2C0, 1, 0x35, 1);   /* Slave Address : 0x35 */
	I2C_SetSlaveAddr(I2C0, 2, 0x55, 1);   /* Slave Address : 0x55 */
	I2C_SetSlaveAddr(I2C0, 3, 0x75, 1);   /* Slave Address : 0x75 */

	/* Set I2C 4 Slave Addresses Mask */
	I2C_SetSlaveAddrMask(I2C0, 0, 0x01);
	I2C_SetSlaveAddrMask(I2C0, 1, 0x04);
	I2C_SetSlaveAddrMask(I2C0, 2, 0x01);
	I2C_SetSlaveAddrMask(I2C0, 3, 0x04);

	/* Enable I2C interrupt */
	I2C_EnableInt(I2C0);
	NVIC_EnableIRQ(I2C0_IRQn);
}

void I2C0_Close(void)
{
    /* Disable I2C0 interrupt and clear corresponding NVIC bit */
    I2C_DisableInt(I2C0);
    NVIC_DisableIRQ(I2C0_IRQn);

    /* Disable I2C0 and close I2C0 clock */
    I2C_Close(I2C0);
    CLK_DisableModuleClock(I2C0_MODULE);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    uint32_t i;

    SYSCLK_INITIATE();
	
    /* Init UART0 for printf */
    UART0_Init();

    /*
        This sample code sets I2C bus clock to 100kHz. Then, accesses Slave (GC Mode) with Byte Write
        and Byte Read operations, and check if the read data is equal to the programmed data.
    */

    printf("\n");
    printf("+------------------------------------------------------------------+\n");
    printf("| I2C Driver Sample Code (Slave) for access Slave (GC Mode)        |\n");
    printf("|                                                                  |\n");
    printf("| I2C Master (I2C0) <---> I2C Slave(I2C0)(Address: 0x00)           |\n");
    printf("+------------------------------------------------------------------+\n");

    printf("Configure I2C0 as a slave.\n");
    printf("The I/O connection for I2C0:\n");
    printf("I2C0_SDA(PA.10), I2C0_SCL(PA.9)\n");

    /* Init I2C0 */
    I2C0_Init();

    /* I2C enter no address SLV mode */
    I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);

    /* Clear receive buffer */
    for(i = 0; i < 0x100; i++)
    {
        g_au8SlvData[i] = 0;
    }

    g_u8SlvEndFlag = 0;

    /* I2C function to Slave receive data */
    s_I2C0HandlerFn = I2C_GCSlaveRx;

    printf("\n");
    printf("Slave(GC Mode) waiting for receiving data.\n");
    while(g_u8SlvEndFlag == 0);

    /* Check receive data correct or not */
    for(i = 0; i < 0x100; i++)
    {
      uint8_t u8Temp;
      
        g_au8SlvTxData[0] = (uint8_t)((i & 0xFF00) >> 8);
        g_au8SlvTxData[1] = (uint8_t)(i & 0x00FF);
        g_au8SlvTxData[2] = (uint8_t)(g_au8SlvTxData[1] + 3);
        
        u8Temp = g_au8SlvTxData[2];
        if(g_au8SlvData[i] != u8Temp)
        {
            printf("GC Mode Receive data fail.\n");
            while(1);
        }
    }

    printf("GC Mode receive data OK.\n");

    s_I2C0HandlerFn = NULL;

    /* Close I2C0 */
    I2C0_Close();

    while(1);
}
