/**
 * @file      deca_error.h
 *
 * @brief     Header file for errors
 *
 * @author    Decawave Applications
 *
 * @attention Copyright (c) 2021 - 2022, Qorvo US, Inc.
 * All rights reserved
 * Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this
 *  list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 3. You may only use this software, with or without any modification, with an
 *  integrated circuit developed by Qorvo US, Inc. or any of its affiliates
 *  (collectively, "Qorvo"), or any module that contains such integrated circuit.
 * 4. You may not reverse engineer, disassemble, decompile, decode, adapt, or
 *  otherwise attempt to derive or gain access to the source code to any software
 *  distributed under this license in binary or object code form, in whole or in
 *  part.
 * 5. You may not use any Qorvo name, trademarks, service marks, trade dress,
 *  logos, trade names, or other symbols or insignia identifying the source of
 *  Qorvo's products or services, or the names of any of Qorvo's developers to
 *  endorse or promote products derived from this software without specific prior
 *  written permission from Qorvo US, Inc. You must not call products derived from
 *  this software "Qorvo", you must not have "Qorvo" appear in their name, without
 *  the prior permission from Qorvo US, Inc.
 * 6. Qorvo may publish revised or new version of this license from time to time.
 *  No one other than Qorvo US, Inc. has the right to modify the terms applicable
 *  to the software provided under this license.
 * THIS SOFTWARE IS PROVIDED BY QORVO US, INC. "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. NEITHER
 *  QORVO, NOR ANY PERSON ASSOCIATED WITH QORVO MAKES ANY WARRANTY OR
 *  REPRESENTATION WITH RESPECT TO THE COMPLETENESS, SECURITY, RELIABILITY, OR
 *  ACCURACY OF THE SOFTWARE, THAT IT IS ERROR FREE OR THAT ANY DEFECTS WILL BE
 *  CORRECTED, OR THAT THE SOFTWARE WILL OTHERWISE MEET YOUR NEEDS OR EXPECTATIONS.
 * IN NO EVENT SHALL QORVO OR ANYBODY ASSOCIATED WITH QORVO BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *
 */

#ifndef INC_ERROR_H_
#define INC_ERROR_H_

//#include <stm32f4xx.h>

typedef enum
{
    _NO_ERR = 0,
    _ERR,
    _ERR_Busy,
    _ERR_Timeout,
    _ERR_DEVID,
    _ERR_IWDG,
    _ERR_INSTANCE,
    _ERR_INIT,
    _ERR_IMU_INIT,
    _ERR_TxBuf_Overflow,
    _ERR_RxBuf_Overflow, // 10
    _ERR_Usb_Tx,
    _ERR_Flash_Ob,
    _ERR_Flash_Prog,
    _ERR_Flash_Erase,
    _ERR_Flash_Error,
    _ERR_Flash_Verify,
    _ERR_Flash_Protected,
    _ERR_LSM_R,
    _ERR_LSM_W,
    _ERR_SPI, // 20
    _ERR_SPI_RRX,
    _ERR_SPI_WTX,
    _ERR_SPI_DMA,
    _ERR_UART_DMA,
    _ERR_UART_INIT,
    _ERR_UART_RX,
    _ERR_UART_TX,
    _ERR_UART_RxCplt,
    _ERR_UART_RxCplt_Overflow,
    _ERR_USB_UART_RX, // 30
    _ERR_TCFM,
    _ERR_TWR_CANNOT_START,
    _ERR_MEM_CORRUPTED,
    _ERR_Configure_WKUP_Timer,
    _ERR_PLL,
    /*TWR*/
    _ERR_Twr_Bad_State,
    _ERR_Not_Twr_Frame,
    _ERR_Unknown_Tag,
    _ERR_DelayedTX_Late,
    _ERR_Range_Calculation, // 40
    _ERR_Ranging_Config,
    _ERR_RC_Version_Unknown,
    _ERR_Non_Compatible_TWR_Parameters,
    _NO_Err_New_Tag,
    _NO_Err_Tx_Sent,
    _NO_Err_Start_Rx,
    _NO_Err_Final,
    _NO_Err_Ranging_Config,
    _NO_Err_Ranging_Update,
    _NO_Err_Response, // 50
    _NO_Err_Idata,
    _NO_Err_Rdata,
    _NO_Err_Can_Sleep,
    /*USB2SPI*/
    _ERR_Usb2Spi_ptr_busy,
    _ERR_Usb2Spi_ptr_alloc,
    /*RTOS*/
    _ERR_General_Error,
    _ERR_Create_Task_Bad,
    _ERR_Timer_Create_Bad,
    _ERR_Timer_Start_Bad,
    _ERR_Signal_Bad, // 60
    _ERR_Cannot_Delete_Timer,
    _ERR_Cannot_Delete_Task,
    _ERR_Cannot_Delete_usb2spiTask,
    _ERR_Cannot_Delete_tcfmTask,
    _ERR_Cannot_Delete_tcwmTask,
    _ERR_Cannot_Delete_imuTask,
    _ERR_Cannot_Delete_rtlsTask,
    _ERR_Cannot_Delete_rxTask,
    _ERR_Cannot_Delete_calcTask,
    _ERR_Cannot_Delete_twrTask, // 70
    _ERR_Cannot_Delete_commTask,
    _ERR_Cannot_Send_Mail,
    _ERR_Cannot_Alloc_Mail,
    _ERR_Cannot_Alloc_Memory,
    _ERR_Cannot_Alloc_NodeMemory,
    _ERR_Malloc_Failed,
    _ERR_Stack_Overflow,
    _ERR_No_pTwrInfo
} error_e;


#endif /* INC_ERROR_H_ */
