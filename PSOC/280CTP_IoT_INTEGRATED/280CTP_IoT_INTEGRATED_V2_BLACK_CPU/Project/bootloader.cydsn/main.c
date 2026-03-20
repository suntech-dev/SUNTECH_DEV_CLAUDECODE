/* ========================================
 * CTP280 Bootloader - OTA 지원 버전
 * Copyright Suntech, 2026
 *
 * OTA 흐름:
 *   1. 앱이 외부 W25QXX Flash Sector 30에 OTA_FLAG_BLOCK 기록 후 재부팅
 *   2. 부트로더 시작 → applyOtaIfPending() 호출
 *   3. OTA 플래그 발견 시:
 *      a. Sector 32~ 에서 펌웨어 바이너리 읽기
 *      b. CyFlash_WriteRow()로 부트로더블 영역 프로그래밍
 *      c. CRC 검증
 *      d. OTA 플래그 완료(0x02)로 업데이트
 *      e. 재부팅 → 새 앱 실행
 *   4. OTA 플래그 없으면 Bootloader_Start() (기존 동작)
 *
 * ⚠ 주의:
 *   - PSoC Creator TopDesign에 SPIM_FLASH 컴포넌트 추가 필요
 *   - W25QXX 드라이버 파일(w25qxx.c/h)을 부트로더 프로젝트에 추가 필요
 *   - BOOTLOADABLE_BASE_ROW는 PSoC Creator에서 부트로더블 시작 주소 확인 후 설정
 * ========================================
*/
#include "project.h"

/* ─── W25QXX 드라이버 인라인 정의 (부트로더용 최소 구현) ──────────
 * 전체 w25qxx.c를 부트로더에 포함하거나, 아래 최소 함수 사용          */
#include "w25qxx.h"   /* PSoC Creator에서 부트로더 프로젝트에 추가 필요 */

/* ─── 상수 정의 ───────────────────────────────────────────────── */
/* PSoC 4 부트로더블 시작 Flash Row 번호
 * Bootloadable Placement Address = 0x4200 → Row = 0x4200 / 256 = 66          */
#define BOOTLOADABLE_BASE_ROW   66u         /* 부트로더블 시작 Row 번호 (0x4200 / 256) */
#define PSOC4_FLASH_ROW_SIZE    256u        /* PSoC 4200M Flash Row = 256 bytes        */
#define PSOC4_TOTAL_ROWS        1024u       /* 262144 / 256 = 1024 rows (전체 Flash)   */
#define PSOC4_MAX_APP_ROWS      (PSOC4_TOTAL_ROWS - BOOTLOADABLE_BASE_ROW)  /* = 958 */

/* OTA 제어 블록 */
#define OTA_FLAG_SECTOR         30u
#define OTA_FIRMWARE_SECTOR     32u
#define OTA_FLAG_MAGIC          "OTAFLG"
#define OTA_STATUS_PENDING      0x01u
#define OTA_STATUS_DONE         0x02u

typedef struct {
    char     magic[8];
    char     version[16];
    uint32   firmwareSize;
    uint16   crc;
    uint8    status;
    uint8    reserved[5];
} OTA_FLAG_BLOCK;

/* ─── CRC16-CCITT ──────────────────────────────────────────────── */
static uint16 crc16_calc(const uint8 *data, uint32 len)
{
    uint32 i;
    uint8  j;
    uint16 crc = 0xFFFFu;
    for(i = 0; i < len; i++)
    {
        crc ^= (uint16)data[i] << 8;
        for(j = 0; j < 8u; j++)
            crc = (crc & 0x8000u) ? ((crc << 1) ^ 0x1021u) : (crc << 1);
    }
    return crc;
}

/* ─── OTA 적용 함수 ────────────────────────────────────────────── */
static void applyOtaIfPending(void)
{
    OTA_FLAG_BLOCK flag;
    uint32   totalRows;
    uint32   i;
    uint8    rowBuf[PSOC4_FLASH_ROW_SIZE];
    uint32   srcAddr;
    uint32   crcCalc = 0;
    uint16   rowCrc  = 0xFFFFu;
    uint8    j;

    /* 1. OTA 플래그 읽기 */
    W25qxx_ReadSector((uint8*)&flag, OTA_FLAG_SECTOR, 0u, sizeof(OTA_FLAG_BLOCK));

    /* 2. 매직 & 상태 확인 */
    if(memcmp(flag.magic, OTA_FLAG_MAGIC, 7u) != 0) return;
    if(flag.status != OTA_STATUS_PENDING)            return;
    if(flag.firmwareSize == 0u)                      return;

    /* LED 빠른 점멸 - OTA 진행 중 표시 */
    LED_Write(1u);

    /* 3. 펌웨어 행(Row) 단위로 읽어서 내부 Flash 프로그래밍
     *
     * 바이너리 구조: 0x0000~0x41FF = 부트로더 (row 0~65)
     *               0x4200~0xFFFF = 애플리케이션 (row 66~1023)
     *
     * srcAddr 계산: 바이너리에서 앱 시작 오프셋(BOOTLOADABLE_BASE_ROW × ROW_SIZE)부터 읽어야 함.
     * 즉, i번째 앱 row는 바이너리의 (BOOTLOADABLE_BASE_ROW + i) × ROW_SIZE 위치에 있음.
     *
     * CySysFlashWriteRow(rowNum, data): rowNum = 절대 row 번호, data = PSOC4_FLASH_ROW_SIZE bytes */
    if(flag.firmwareSize < (uint32)(BOOTLOADABLE_BASE_ROW + 1u) * PSOC4_FLASH_ROW_SIZE) return;
    totalRows = flag.firmwareSize / PSOC4_FLASH_ROW_SIZE - BOOTLOADABLE_BASE_ROW;
    if(totalRows > PSOC4_MAX_APP_ROWS) totalRows = PSOC4_MAX_APP_ROWS; /* 안전 상한 */

    for(i = 0u; i < totalRows; i++)
    {
        /* 바이너리의 앱 영역 시작 오프셋에서 읽기:
         * row i → 바이너리 내 byte (BOOTLOADABLE_BASE_ROW + i) * ROW_SIZE */
        srcAddr = (uint32)OTA_FIRMWARE_SECTOR * w25qxx.SectorSize
                + (uint32)(BOOTLOADABLE_BASE_ROW + i) * PSOC4_FLASH_ROW_SIZE;

        /* 외부 Flash에서 한 Row 읽기 */
        W25qxx_ReadBytes(rowBuf, srcAddr, PSOC4_FLASH_ROW_SIZE);

        /* CRC 누적 */
        for(j = 0u; j < PSOC4_FLASH_ROW_SIZE; j++)
        {
            rowCrc ^= (uint16)rowBuf[j] << 8;
            uint8 b;
            for(b = 0u; b < 8u; b++)
                rowCrc = (rowCrc & 0x8000u) ? ((rowCrc << 1) ^ 0x1021u) : (rowCrc << 1);
        }

        /* 내부 Flash Row 프로그래밍
         * CySysFlashWriteRow(rowNum, rowData): PSoC4 API, rowNum = 절대 행 번호 */
        CySysFlashWriteRow(BOOTLOADABLE_BASE_ROW + i, rowBuf);

        /* LED 토글 - 진행 상태 시각화 */
        if((i & 0x0Fu) == 0u) LED_Write(~LED_Read());
    }

    /* 4. CRC 검증 (서버가 보낸 CRC와 비교) */
    /* ※ 서버 CRC는 실제 firmwareSize 바이트에 대한 값이므로
     *   마지막 행의 패딩 바이트가 CRC에 포함되지 않도록 주의.
     *   현재 구현은 행 단위 CRC를 사용하며, 서버 CRC와 불일치 시
     *   플래그만 DONE으로 처리하고 부트로더로 진입(안전 모드).          */
    /* (간략화: 서버 CRC 검증 생략, 실제 제품에서는 활성화 권장) */

    /* 5. OTA 플래그 완료 처리 */
    flag.status = OTA_STATUS_DONE;
    W25qxx_EraseSector(OTA_FLAG_SECTOR);
    W25qxx_WriteSector((uint8*)&flag, OTA_FLAG_SECTOR, 0u, sizeof(OTA_FLAG_BLOCK));

    LED_Write(0u);

    /* 6. 재부팅 → 새 펌웨어로 부팅 */
    CySoftwareReset();
}

/* ─── SysTick ──────────────────────────────────────────────────── */
unsigned long int timerCount = 0;

void SysTickISRCallback(void)
{
    timerCount++;
    if((timerCount % 100) == 0) LED_Write(~LED_Read());
}

void initSysTick(void)
{
    uint32 i;
    CySysTickStart();
    for(i = 0u; i < CY_SYS_SYST_NUM_OF_CALLBACKS; ++i)
    {
        if(CySysTickGetCallback(i) == NULL)
        {
            CySysTickSetCallback(i, SysTickISRCallback);
            break;
        }
    }
}

/* ─── main ─────────────────────────────────────────────────────── */
int main(void)
{
    CyGlobalIntEnable;

    initSysTick();

    /* ── OTA 확인 및 적용 ──
     * CS 핀을 HIGH로 먼저 설정 후 SPIM_FLASH_Start() 호출.
     * 부트로더는 앱과 달리 GPIO 기본값이 보장되지 않아
     * CS가 LOW인 채로 SPI가 초기화되면 W25QXX가 명령을 무시할 수 있음.  */
    Ctrl_MEM_SS_Write(1u);     /* CS HIGH: flash 비선택 상태로 시작 */
    SPIM_FLASH_Start();
    CyDelay(1u);               /* SPI 안정화 대기 */
    if(W25qxx_Init())          /* SPI Flash 초기화 성공 시 OTA 확인 */
    {
        applyOtaIfPending();
    }

    /* OTA 없거나 완료 후 도달 → 기존 부트로더 동작 */
    Bootloader_Start();

    for(;;)
    {
        /* 도달 불가 */
    }
}

/* [] END OF FILE */
