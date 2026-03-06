/* ========================================
 *
 * Copyright SUNTECH, 2018-2026
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF SUNTECH.
 *
 * ========================================
*/

/*
 * 18-03-12 : Bootloader
 * 18-05-14 : 2D Scanner + Touch OP
 * 18-05-14 : Tablet > "Monitoring Port"
 * 18-05-14 : Touch OP (With Scanner) > "USB_OP Port"
 * 18-05-25 : Scanner Mode Enable
 * 19-04-20 : USED PWJ
 * 26-03-04 : 코드 품질 개선 (uint8_t, NULL 체크, 상수화, 버퍼 방어, 미사용 변수 제거)
*/

#include "main.h"

unsigned long lastReceivedTime;
char          receivedFromBarcode[UART_BUF_SIZE];
uint8_t       receivedFromBarcodeCount = 0;

char scanTrigerStart[3] = { 0x02, 0xf4, 0x03 };    // 2D SCANNER scanTrigerStart

char         receivedScanTrigerOrder[UART_BUF_SIZE];
unsigned int receivedScanTrigerOrderCount = 0;


void process_BARCODE(void)
{
    while (BARCODE_SpiUartGetRxBufferSize() > 0)
    {
        char c = BARCODE_UartGetChar();     // 항상 먼저 소비 (무한루프 방지)

        if (receivedFromBarcodeCount == 0)
        {
            timerCount = 0;
        }
        lastReceivedTime = timerCount;

        if (receivedFromBarcodeCount < UART_BUF_SIZE - 1)  // 버퍼 오버플로우 방어
        {
            receivedFromBarcode[receivedFromBarcodeCount++] = c;
        }
    }
}


void process_USB_OP(void)
{
    while (USB_OP_SpiUartGetRxBufferSize() > 0)
    {
        char c = USB_OP_UartGetChar();      // 항상 먼저 소비 (무한루프 방지)

        if (receivedScanTrigerOrderCount == 0)
        {
            timerCount = 0;
        }
        lastReceivedTime = timerCount;

        if (receivedScanTrigerOrderCount < UART_BUF_SIZE - 1)  // 버퍼 오버플로우 방어
        {
            receivedScanTrigerOrder[receivedScanTrigerOrderCount++] = c;
        }

        if (receivedScanTrigerOrderCount == (unsigned int)strlen(SCAN_TRIGER_ORDER_STR))
        {
            receivedScanTrigerOrder[receivedScanTrigerOrderCount] = '\0';

            if (strcmp(receivedScanTrigerOrder, SCAN_TRIGER_ORDER_STR) == 0)
            {
                for (int i = 0; i < 3; i++)
                {
                    BARCODE_UartPutChar(scanTrigerStart[i]);    // 2D Scanner 트리거
                }
            }
            if (strcmp(receivedScanTrigerOrder, SCAN_MODE_CMD_STR) == 0)
            {
                USB_OP_UartPutString("@@@@!99900304;&{FM100-M-R|DD03AB95}^^^^");  // Scan Mode Enable
            }

            receivedScanTrigerOrderCount = 0;
        }
    }
}


int main(void)
{
    CyGlobalIntEnable;

    init();

    char oldCounterStat = Counter_Read();

    for (;;)
    {
        char currentCounterStat = Counter_Read();

        if (oldCounterStat != currentCounterStat && currentCounterStat == 0)
        {
            timerCount = 0;
            lastReceivedTime = 0;
        }
        if (oldCounterStat != currentCounterStat && currentCounterStat == 1)
        {
            unsigned int dTime = timerCount - lastReceivedTime;

            if (dTime > COUNTER_MIN_MS && dTime < COUNTER_MAX_MS)
            {
                MONITORING_UartPutString("{\"cmd\" : \"count\", \"value\" : 1}");
            }
        }
        oldCounterStat = currentCounterStat;

        // 장시간 데이터가 들어오지 않으면 노이즈 데이터로 처리
        if (timerCount - lastReceivedTime > TIMEOUT_MS_UART)
        {
            receivedScanTrigerOrderCount = 0;

            if (receivedFromBarcodeCount > 0)
            {
                receivedFromBarcode[receivedFromBarcodeCount] = '\0';

                if (strstr(receivedFromBarcode, "SUNTECH") != NULL)
                {
                    BootloaderStart();
                }

                char *ptrFirst, *ptrSecond, *ptrThird, msg[UART_BUF_SIZE];

                ptrFirst  = strtok(receivedFromBarcode, "/\r\n");   // 2D Scanner
                ptrSecond = strtok(NULL, "/\r\n");
                ptrThird  = strtok(NULL, "/\r\n");

                if (ptrFirst != NULL)   // NULL 포인터 방어
                {
                    sprintf(msg, "@@@@!99900035;^^^^*P%s*\r\n", ptrFirst);
                    USB_OP_UartPutString(msg);      // DesignNo → Touch OP 전송
                }

                if (ptrSecond != NULL && ptrThird != NULL)  // NULL 포인터 방어
                {
                    sprintf(msg, "{\"cmd\" : \"barcode\", \"value\" : [\"%s\",\"%s\"]}", ptrSecond, ptrThird);
                    MONITORING_UartPutString(msg);  // 태블릿 JSON 전송
                }
                else if (ptrSecond != NULL)
                {
                    sprintf(msg, "{\"cmd\" : \"barcode\", \"value\" : [\"%s\"]}", ptrSecond);
                    MONITORING_UartPutString(msg);  // 태블릿 JSON 전송
                }

                receivedFromBarcodeCount = 0;
            }
        }

        process_USB_OP();
        process_BARCODE();

        if (Pin_StartBootloader_Read() == 0)
        {
            BootloaderStart();
        }
    }
}

/* [] END OF FILE */
