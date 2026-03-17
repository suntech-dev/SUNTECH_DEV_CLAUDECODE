/*
 * ST-500 Touch OP Version
 *
 * 20-03-10 : Bootloader. 2D Scanner + Touch OP.
 * 20-03-10 : USED HWI. Scanner Mode Enable.
 * 20-03-10 : Count > 2Pin. Power > 8Pin. IoT 보드 전원을 CPU 보드와 연결해야 함.
 * 20-03-10 : HWI Downtime End OK. Foot or Hand Switch Cable 에 연결.
 * 20-03-10 : Tablet > "OP Port". Touch OP (With Scanner) > "Monitoring Port". Scanner Test OK.
 * 20-03-10 : HWI 전체 적용.
 *
 * 21-05-28 : 2021 New version
 * 21-05-28 : VJ, SCI, PWJ version
 * 21-05-28 : old button op unused (only ST-500 version)
 * 21-05-28 : ST-500 is Monitoring port
 * 21-05-28 : Tablet is OP port
 *
 * 26-03-03 : 코드 개선 - 타입 안전성 향상, NULL 포인터 검사 추가,
 *            상수 정의 적용, 버퍼 오버플로우 방어, 미사용 변수 제거
 *
 * 26-03-16 : [TESTER] 스캔 트리거 비활성화, 스캔 값 원본 그대로 port_MONITORING 송신
 */

#include "main.h"

/* ---- 전역 변수 ---- */
unsigned long  lastReceivedTime            = 0;
char           receivedFromBarcode[UART_BUF_SIZE];
uint8_t        receivedFromBarcodeCount    = 0;   /* FIX: char → uint8_t (배열 인덱스 타입 안전성) */

/* [TESTER] 스캔 트리거 미사용 — 추후 활성화 시 아래 주석 해제 */
/* char  scanTrigerStart[3] = { 0x02, 0xF4, 0x03 }; */

char           receivedScanTrigerOrder[UART_BUF_SIZE];
unsigned int   receivedScanTrigerOrderCount = 0;


/* ------------------------------------------------------------------ */
/*  process_BARCODE()                                                 */
/*  스캐너 UART RX 버퍼에서 데이터를 읽어 receivedFromBarcode 에 누적  */
/* ------------------------------------------------------------------ */
void process_BARCODE(void)
{
    while (BARCODE_SpiUartGetRxBufferSize() > 0)
    {
        char c = BARCODE_UartGetChar();  /* 항상 버퍼에서 소비 */

        if (receivedFromBarcodeCount == 0)
        {
            timerCount = 0;
        }

        lastReceivedTime = timerCount;

        if (receivedFromBarcodeCount < (uint8_t)(UART_BUF_SIZE - 1))
        {
            receivedFromBarcode[receivedFromBarcodeCount++] = c;
        }
    }
}


/* ------------------------------------------------------------------ */
/*  process_MONITORING()                                              */
/*  Touch OP 로부터 받은 명령을 파싱하여 스캐너 또는 태블릿으로 전달  */
/* ------------------------------------------------------------------ */
void process_MONITORING(void)
{
    while (port_MONITORING_SpiUartGetRxBufferSize() > 0)
    {
        char ch = port_MONITORING_UartGetChar();  /* 항상 버퍼에서 소비 */

        if (receivedScanTrigerOrderCount == 0)
        {
            timerCount = 0;
        }

        lastReceivedTime = timerCount;

        if (receivedScanTrigerOrderCount < UART_BUF_SIZE - 1)
        {
            receivedScanTrigerOrder[receivedScanTrigerOrderCount++] = ch;
        }

        if (receivedScanTrigerOrderCount == strlen(SCAN_TRIGER_ORDER_STR))
        {
            receivedScanTrigerOrder[receivedScanTrigerOrderCount] = '\0';

            /* [TESTER] 스캔 트리거 전송 비활성화 — 추후 필요 시 아래 주석 해제
            if (strcmp(receivedScanTrigerOrder, SCAN_TRIGER_ORDER_STR) == 0)
            {
                int i;
                for (i = 0; i < 3; i++)
                {
                    BARCODE_UartPutChar(scanTrigerStart[i]);
                }
            }
            */

            if (strcmp(receivedScanTrigerOrder, SCAN_MODE_CMD_STR) == 0)
            {
                port_MONITORING_UartPutString("@@@@!99900304;&{FM100-M-R|DD03AB95}^^^^");
            }

            receivedScanTrigerOrderCount = 0;
        }
    }
}


/* ------------------------------------------------------------------ */
/*  main()                                                            */
/* ------------------------------------------------------------------ */
int main(void)
{
    CyGlobalIntEnable;
    init();

    port_MONITORING_UartPutString("SunTech CTP280 BARCODE SCANNER TEST Start port_MONITORING...\r\n");
    port_OP_UartPutString("SunTech CTP280 BARCODE SCANNER TEST Start port_OP...\r\n");

    char oldCounterStat = Counter_Read();

    

    for (;;)
    {
        char currentCounterStat = Counter_Read();

        /* Counter 하강 엣지 (1→0): 타이머 초기화 */
        if (oldCounterStat != currentCounterStat && currentCounterStat == 0)
        {
            timerCount       = 0;
            lastReceivedTime = 0;
        }

        /* Counter 상승 엣지 (0→1): 유효 범위 내면 태블릿에 카운트 이벤트 전송 */
        if (oldCounterStat != currentCounterStat && currentCounterStat == 1)
        {
            unsigned int dTime = timerCount - lastReceivedTime;

            if (dTime > COUNTER_MIN_MS && dTime < COUNTER_MAX_MS)
            {
                port_OP_UartPutString("{\"cmd\" : \"count\", \"value\" : 1}");
            }
        }

        oldCounterStat = currentCounterStat;

        /* 타임아웃: 수신 완료로 판단하고 버퍼 처리 */
        if (timerCount - lastReceivedTime > TIMEOUT_MS_UART)
        {
            receivedScanTrigerOrderCount = 0;

            if (receivedFromBarcodeCount > 0)
            {
                receivedFromBarcode[receivedFromBarcodeCount] = '\0';

                /* [TESTER] 스캔 값 원본 그대로 port_MONITORING 으로 송신 */
                port_MONITORING_UartPutString(receivedFromBarcode);
                port_OP_UartPutString(receivedFromBarcode);

                /* [TESTER] 기존 파싱/포맷 로직 비활성화 — 추후 활성화 시 아래 주석 해제
                char  *ptrFirst;
                char  *ptrSecond;
                char  *ptrThird;
                char   msg[UART_BUF_SIZE];

                ptrFirst  = strtok(receivedFromBarcode, "/\r\n");
                ptrSecond = strtok(NULL, "/\r\n");
                ptrThird  = strtok(NULL, "/\r\n");

                if (ptrFirst != NULL)
                {
                    sprintf(msg, "@@@@!99900035;^^^^*P%s*\r\n", ptrFirst);
                    port_MONITORING_UartPutString(msg);
                }

                if (ptrThird != NULL && strlen(ptrThird) > 0)
                {
                    sprintf(msg, "{\"cmd\" : \"barcode\", \"value\" : [\"%s\",\"%s\"]}", ptrSecond, ptrThird);
                }
                else if (ptrSecond != NULL && strlen(ptrSecond) > 0)
                {
                    sprintf(msg, "{\"cmd\" : \"barcode\", \"value\" : [\"%s\"]}", ptrSecond);
                }
                else
                {
                    strcpy(msg, "");
                }

                port_OP_UartPutString(msg);
                */

                receivedFromBarcodeCount = 0;
            }
        }

        process_BARCODE();
        process_MONITORING();

        /* IoT Board 내부 버튼: LOW 감지 시 부트로더 진입 */
        if (Pin_StartBootloader_Read() == 0)
        {
            BootloaderStart();
        }
    }
}
/* [] END OF FILE */
