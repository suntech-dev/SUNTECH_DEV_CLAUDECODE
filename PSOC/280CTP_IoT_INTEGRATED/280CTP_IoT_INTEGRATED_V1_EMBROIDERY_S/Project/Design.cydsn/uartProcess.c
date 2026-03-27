/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "main.h"
#include "uartProcess.h"

/*
#define UART_BUFFER_SIZE 1000

uint8_t uartRxBuffer[UART_BUFFER_SIZE];
uint16 g_bufferIndex = 0;
// UART 인터럽트 서비스 루틴
CY_ISR(UART_ISR_Handler)
{
    // RX FIFO에 데이터가 있는지 확인
    while (UART_SpiUartGetRxBufferSize() > 0)
    {
        // RX FIFO에서 데이터 읽기
        uint8_t receivedData = UART_UartGetChar();
        
        // 데이터 저장 (버퍼 오버플로 방지)
        if (g_bufferIndex < UART_BUFFER_SIZE)
        {
            uartRxBuffer[g_bufferIndex++] = receivedData;
        }
        else
        {
            // 버퍼 오버플로 처리 (필요하면 추가 로직 작성)
        }
    }

    // Clear the RX interrupt
    UART_ClearRxInterruptSource(UART_INTR_RX_NOT_EMPTY);
}

*/
void initUartProcess()
{
    UART_Start();
}
/* [] END OF FILE */
