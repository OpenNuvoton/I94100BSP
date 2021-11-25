;/******************************************************************************
; * @file     startup_I94100.s
; * @version  V1.0
; * $Revision: 1 $
; * $Date: 17/11/22 02:20p $ 
; * @brief    CMSIS Cortex-M4 Core Device Startup File for I94100 MCU
; *
; * @note
; * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
;*****************************************************************************/

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        EXTERN  SystemInit
        PUBLIC  __vector_table
        PUBLIC  __vector_table_0x1c
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

        DATA

__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler

        DCD     NMI_Handler
        DCD     HardFault_Handler
        DCD     MemManage_Handler
        DCD     BusFault_Handler
        DCD     UsageFault_Handler
__vector_table_0x1c
        DCD     0
        DCD     0
        DCD     0
        DCD     0
        DCD     SVC_Handler
        DCD     DebugMon_Handler
        DCD     0
        DCD     PendSV_Handler
        DCD     SysTick_Handler

        ; External Interrupts
        DCD     BOD_IRQHandler            ; 0: Brown Out detection
        DCD     IRC_IRQHandler            ; 1: Internal RC
        DCD     PWRWU_IRQHandler          ; 2: Power Down Wake Up 
        DCD     SRAMF_IRQHandler		  ; 3: SRAM parity check error.
        DCD     CLKF_IRQHandler   		  ;	4: Clock fail detected 
        DCD     Default_Handler		      ;	5: Reserved.
        DCD     RTC_IRQHandler            ; 6: Real Time Clock 
        DCD     Default_Handler           ; 7: Reserved
        DCD     WDT_IRQHandler            ; 8: Watchdog timer
        DCD     WWDT_IRQHandler           ; 9: Window watchdog timer
        DCD     EINT0_IRQHandler          ; 10: External Input 0
        DCD     EINT1_IRQHandler          ; 11: External Input 1
        DCD     EINT2_IRQHandler          ; 12: External Input 2
        DCD     EINT3_IRQHandler          ; 13: External Input 3
        DCD     EINT4_IRQHandler          ; 14: External Input 4
        DCD     EINT5_IRQHandler          ; 15: External Input 5
        DCD     GPA_IRQHandler            ; 16: GPIO Port A
        DCD     GPB_IRQHandler            ; 17: GPIO Port B
        DCD     GPC_IRQHandler            ; 18: GPIO Port C
        DCD     GPD_IRQHandler            ; 19: GPIO Port D
        DCD     Default_Handler           ; 20: Reserved
        DCD     Default_Handler           ; 21: Reserved
        DCD     SPI0_IRQHandler           ; 22: SPI0
        DCD     SPI1_IRQHandler           ; 23: SPI1
        DCD     Default_Handler           ; 24: Reserved
        DCD     PWM0P0_IRQHandler         ; 25: PWM0 pair 0 interrupt
        DCD     PWM0P1_IRQHandler         ; 26: PWM0 pair 1 interrupt
        DCD     PWM0P2_IRQHandler         ; 27: PWM0 pair 2 interrupt
        DCD     Default_Handler           ; 28: Reserved
        DCD     Default_Handler           ; 29: Reserved
        DCD     Default_Handler           ; 30: Reserved
        DCD     Default_Handler           ; 31: Reserved
        DCD     TMR0_IRQHandler           ; 32: Timer 0
        DCD     TMR1_IRQHandler           ; 33: Timer 1
        DCD     TMR2_IRQHandler           ; 34: Timer 2
        DCD     TMR3_IRQHandler           ; 35: Timer 3
        DCD     UART0_IRQHandler          ; 36: UART0
        DCD     Default_Handler           ; 37: Reserved
        DCD     I2C0_IRQHandler           ; 38: I2C0
        DCD     I2C1_IRQHandler           ; 39: I2C1
        DCD     PDMA_IRQHandler           ; 40: Peripheral DMA
        DCD     Default_Handler           ; 41: Reserved
        DCD     EADC0_IRQHandler          ; 42: EADC interrupt source 0
        DCD     EADC1_IRQHandler          ; 43: EADC interrupt source 1
        DCD     Default_Handler           ; 44: Reserved
        DCD     Default_Handler           ; 45: Reserved
        DCD     EADC2_IRQHandler          ; 46: EADC interrupt source 2
        DCD     EADC3_IRQHandler          ; 47: EAD interrupt source 3
        DCD     Default_Handler           ; 48: Reserved
        DCD     Default_Handler           ; 49: Reserved
        DCD     Default_Handler           ; 50: Reserved
        DCD     SPI2_IRQHandler           ; 51: SPI2
        DCD     Default_Handler           ; 52: 
        DCD     Default_Handler           ; 53:
        DCD     Default_Handler           ; 54: 
        DCD     Default_Handler           ; 55: 
        DCD     Default_Handler           ; 56:
        DCD     Default_Handler           ; 57: 
        DCD     Default_Handler           ; 58: 
        DCD     Default_Handler           ; 59: 
        DCD     Default_Handler           ; 60:
        DCD     Default_Handler           ; 61:
        DCD     DPWM_IRQHandler           ; 62: DPWM
        DCD     Default_Handler           ; 63:
        DCD     Default_Handler           ; 64: 
        DCD     Default_Handler           ; 65: 
        DCD     Default_Handler           ; 66: 
        DCD     Default_Handler           ; 67:
        DCD     I2S0_IRQHandler           ; 68: I2S0
                
__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB

        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER(2)
Reset_Handler
        LDR     R0, =0x40000100
        ; Unlock Register                
        LDR     R1, =0x59
        STR     R1, [R0]
        LDR     R1, =0x16
        STR     R1, [R0]
        LDR     R1, =0x88
        STR     R1, [R0]
        
        ; Init POR
        LDR     R2, =0x40000024
        LDR     R1, =0x00005AA5
        STR     R1, [R2]
        
        ; Select INV Type
        LDR     R2, =0x40000200
        LDR     R1, [R2]
        BIC     R1, R1, #0x1000
        STR     R1, [R2]
        
        ; Lock register
        MOVS    R1, #0
        STR     R1, [R0]                

        LDR     R0, =SystemInit
        BLX     R0
        LDR     R0, =__iar_program_start
        BX      R0

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        
#ifdef DEBUG_ENABLE_SEMIHOST
        SECTION .text:CODE:REORDER(2)
HardFault_Handler


                MOV     R0, LR
                LSLS    R0,R0, #29            ; Check bit 2
                BMI     SP_is_PSP             ; previous stack is PSP
                MRS     R0, MSP               ; previous stack is MSP, read MSP
                B       SP_Read_Ready
SP_is_PSP
                MRS     R0, PSP               ; Read PSP
SP_Read_Ready
                LDR     R1, [R13, #24]         ; Get previous PC
                LDRH    R3, [R1]              ; Get instruction
				LDR    R2, =0xBEAB           ; The sepcial BKPT instruction
                CMP     R3, R2                ; Test if the instruction at previous PC is BKPT
				BNE    HardFault_Handler_Ret ; Not BKPT

                ADDS    R1, #4                ; Skip BKPT and next line
                STR     R1, [R13, #24]         ; Save previous PC

				BX     LR
HardFault_Handler_Ret

#else

        SECTION .text:CODE:REORDER(1)
HardFault_Handler

#endif

        B HardFault_Handler

        PUBWEAK MemManage_Handler
        SECTION .text:CODE:REORDER(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:REORDER(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:REORDER(1)
UsageFault_Handler
        B UsageFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER(1)
SVC_Handler
        B SVC_Handler

        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:REORDER(1)
DebugMon_Handler
        B DebugMon_Handler

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER(1)
PendSV_Handler
        B PendSV_Handler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER(1)
SysTick_Handler
        B SysTick_Handler

        PUBWEAK BOD_IRQHandler
        PUBWEAK IRC_IRQHandler
        PUBWEAK PWRWU_IRQHandler
        PUBWEAK SRAMF_IRQHandler
        PUBWEAK CLKF_IRQHandler
        PUBWEAK RTC_IRQHandler
        PUBWEAK WDT_IRQHandler
        PUBWEAK WWDT_IRQHandler
        PUBWEAK EINT0_IRQHandler
        PUBWEAK EINT1_IRQHandler
        PUBWEAK EINT2_IRQHandler
        PUBWEAK EINT3_IRQHandler
        PUBWEAK EINT4_IRQHandler
        PUBWEAK EINT5_IRQHandler
        PUBWEAK GPA_IRQHandler
        PUBWEAK GPB_IRQHandler
        PUBWEAK GPC_IRQHandler
        PUBWEAK GPD_IRQHandler
        PUBWEAK SPI0_IRQHandler
        PUBWEAK SPI1_IRQHandler
        PUBWEAK PWM0P0_IRQHandler
        PUBWEAK PWM0P1_IRQHandler
        PUBWEAK PWM0P2_IRQHandler
        PUBWEAK TMR0_IRQHandler
        PUBWEAK TMR1_IRQHandler
        PUBWEAK TMR2_IRQHandler
        PUBWEAK TMR3_IRQHandler
        PUBWEAK UART0_IRQHandler
        PUBWEAK I2C0_IRQHandler
        PUBWEAK I2C1_IRQHandler
        PUBWEAK PDMA_IRQHandler
        PUBWEAK EADC0_IRQHandler
        PUBWEAK EADC1_IRQHandler
        PUBWEAK EADC2_IRQHandler
        PUBWEAK EADC3_IRQHandler
        PUBWEAK SPI2_IRQHandler
        PUBWEAK DPWM_IRQHandler
        PUBWEAK I2S0_IRQHandler
        
        SECTION .text:CODE:REORDER(1)
        
BOD_IRQHandler
IRC_IRQHandler
PWRWU_IRQHandler
SRAMF_IRQHandler
CLKF_IRQHandler
RTC_IRQHandler
WDT_IRQHandler
WWDT_IRQHandler
EINT0_IRQHandler
EINT1_IRQHandler
EINT2_IRQHandler
EINT3_IRQHandler
EINT4_IRQHandler
EINT5_IRQHandler
GPA_IRQHandler
GPB_IRQHandler
GPC_IRQHandler
GPD_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
PWM0P0_IRQHandler
PWM0P1_IRQHandler
PWM0P2_IRQHandler
TMR0_IRQHandler
TMR1_IRQHandler
TMR2_IRQHandler
TMR3_IRQHandler
UART0_IRQHandler 
I2C0_IRQHandler 
I2C1_IRQHandler
PDMA_IRQHandler
EADC0_IRQHandler
EADC1_IRQHandler
EADC2_IRQHandler
EADC3_IRQHandler
SPI2_IRQHandler
DPWM_IRQHandler
I2S0_IRQHandler
Default_Handler
        B   .    

    END        
;/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
