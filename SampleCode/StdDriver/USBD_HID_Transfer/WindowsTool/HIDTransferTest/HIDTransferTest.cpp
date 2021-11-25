/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) 2010 Nuvoton Technology Corp. All rights reserved.                                        */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "HIDTransferTest.h"
#include "HID.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define USB_VID         0x0416  /* Vendor ID */
#define USB_PID         0x1002  /* Product ID */

#define HID_CMD_SIGNATURE   0x43444948

/* HID Transfer Commands */
#define HID_CMD_NONE     0x00
#define HID_CMD_ERASE    0x71
#define HID_CMD_READ     0xD2
#define HID_CMD_WRITE    0xC3
#define HID_CMD_TEST     0xB4


#define PAGE_SIZE       2048/*256*/
#define SECTOR_SIZE     4096
#define HID_PACKET_SIZE 64


#define USB_TIME_OUT    100

// 僅有的一個應用程式物件

CWinApp theApp;

using namespace std;

int main(void);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    int nRetCode = 0;

    // 初始化 MFC 並於失敗時列印錯誤
    if(!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
    {
        // TODO: 配合您的需要變更錯誤碼
        _tprintf(_T("嚴重錯誤: MFC 初始化失敗\n"));
        nRetCode = 1;
    }
    else
    {
        // TODO: 在此撰寫應用程式行為的程式碼。
        main();




    }

    return nRetCode;
}

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
    unsigned char cmd;
    unsigned char len;
    unsigned int arg1;
    unsigned int arg2;
    unsigned int signature;
    unsigned int checksum;
} CMD_T;

#pragma pack(pop)   /* restore original alignment from stack */



unsigned int CalCheckSum(unsigned char *buf, unsigned int size)
{
    unsigned int sum;
    int i;

    i = 0;
    sum = 0;
    while(size--)
    {
        sum += buf[i++];
    }

    return sum;

}

/*
    This function is used to read data through USB HID.

    pReadBuf - [in ] The read buffer to store the data from USB HID. User must make sure its size is enough.
    startPage- [out] The start page to read. The page size should be dependent on SPI Flash.
    pages    - [out] The number of pages to read.

    return value is the total read bytes.
*/
int ReadPages(unsigned char *pReadBuf, unsigned int startPage, unsigned int pages)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    unsigned long readBytes;
    bool isDeviceOpened;

    readBytes = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf("USB HID Device VID[%04x] PID[%04x] Open Success.\n", USB_VID, USB_PID);
        printf(">>> Read pages: %d - %d\n", startPage, startPage + pages - 1);

        cmd.cmd = HID_CMD_READ;
        cmd.len = sizeof(cmd) - 4; /* Not include checksum */
        cmd.arg1 = startPage;
        cmd.arg2 = pages;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send read command error!\n");
            goto lexit;
        }

        while(1)
        {
            if(readBytes >= cmd.arg2 * PAGE_SIZE)
            {
                break;
            }

            bRet = io.ReadFile(pReadBuf + readBytes, 256, &length, USB_TIME_OUT);
            if(!bRet)
            {
                printf("ERROR: Read fail!\n");
                goto lexit;
            }
            readBytes += length;
        }

    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return readBytes;
}

/*
    This function is used to erase sectors of target device.

    startSector- [out] The start sector to erase. The sector size should be dependent on SPI Flash.
    sectors    - [out] The number of sectors to erase.

    return value is the total erased sec.
*/
int EraseSectors(unsigned int startSector, unsigned int sectors)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    bool isDeviceOpened;
    unsigned int eraseCnt;

    eraseCnt = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf("USB HID Device VID[%04x] PID[%04x] Open Success.\n", USB_VID, USB_PID);
        printf(">>> Erase sectors: %d - %d\n", startSector, startSector + sectors - 1);

        cmd.cmd = HID_CMD_ERASE;
        cmd.len = sizeof(cmd) - 4; /* Not include checksum */
        cmd.arg1 = startSector;
        cmd.arg2 = sectors;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send erase command error!\n");
            goto lexit;
        }
        eraseCnt = sectors;
    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return eraseCnt;
}


/*
    This function is used to program data to target device through USB HID.

    pWriteBuf- [in ] The buffer of programming data. User must make sure its size is enough.
    startPage- [out] The start page to program SPI Flash. The page size should be dependent on SPI Flash.
    pages    - [out] The number of pages to program.

    return value is the total programming bytes.
*/
int WritePages(unsigned char *pWriteBuf, unsigned int startPage, unsigned int pages)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    unsigned long writeBytes;
    bool isDeviceOpened;

    writeBytes = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf("USB HID Device VID[%04x] PID[%04x] Open Success.\n", USB_VID, USB_PID);
        printf(">>> Write pages: %d - %d\n", startPage, startPage + pages - 1);

        cmd.cmd = HID_CMD_WRITE;
        cmd.len = sizeof(cmd) - 4; /* Not include checksum */
        cmd.arg1 = startPage;
        cmd.arg2 = pages;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send read command error!\n");
            goto lexit;
        }

        while(1)
        {
            if(writeBytes >= cmd.arg2 * PAGE_SIZE)
            {
                break;
            }

            bRet = io.WriteFile(pWriteBuf + writeBytes, HID_PACKET_SIZE, &length, USB_TIME_OUT);
            if(!bRet)
            {
                printf("ERROR: Write fail!\n");
                goto lexit;
            }
            writeBytes += length;
        }

    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return writeBytes;
}

/*
    This function is used to be an simple demo of send command. User may use it as an example to add new command.
*/
int SendTestCmd(void)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    bool isDeviceOpened;


    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf("USB HID Device VID[%04x] PID[%04x] Open Success.\n", USB_VID, USB_PID);
        printf(">>> Test command\n");

        cmd.cmd = HID_CMD_TEST;
        cmd.len = sizeof(cmd) - 4; /* Not include checksum */
        cmd.arg1 = 0x12345678;
        cmd.arg2 = 0xabcdef01;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send test command error!\n");
            goto lexit;
        }
    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return 0;
}

int main(void)
{
    int i,iCount;
    unsigned char buf[16 * PAGE_SIZE];

	iCount = 4;
    /* Erase all sectors */
    EraseSectors(0,8);
    /* Blank check 4(2-5) pages */
    ReadPages(buf,2,iCount);

    for(i = 0; i < iCount*PAGE_SIZE; i++)
    {
        if(buf[i] != (unsigned char)0xFF)
        {
            printf("ERROR: Blank test fail!\n");
            return -1;
        }
    }

	iCount = 5;
	/* Setting buffer value */
    for(i = 0; i < iCount*PAGE_SIZE; i++)
        buf[i] = i & 0xFF;
    /* Write buffer data 5(10-14) pages */
    WritePages(buf, 10, iCount);
    /* Clean the buffer */
    memset(buf, 0xCC, iCount*PAGE_SIZE);
    /* Read buffer 5(10-14) pages to compare */
    ReadPages(buf, 10, iCount);

    for(i = 0; i < iCount*PAGE_SIZE; i++)
    {
        if(buf[i] != (i & 0xFF))
        {
            printf("ERROR: Programming test fail!\n");
            return -1;
        }
    }

    printf("\n>>> HID Transfer test ok! \n");
	system("pause");
    return 0;
}