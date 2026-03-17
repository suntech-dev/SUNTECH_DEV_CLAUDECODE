# PSoC 프로젝트 SRAM 최적화 분석 보고서

**최초 작성일**: 2025-12-20 (IoT_PWJ_2025.12.15 기반)
**갱신일**: 2026-03-09 (280CTP_IoT_INTEGRATED_V1 적용)
**프로젝트**: Integrated REV 9.8.3
**대상**: 280CTP_IoT_INTEGRATED_V1/Project/Design.cydsn

---

## 📊 현재 상황

### ✅ 최적화 완료 (1단계) — 실측값

- **SRAM 사용량**: 21,620 / 32,768 바이트 (**66.0%**)
- **여유 공간**: 11,148 바이트 (34.0%)
- **상태**: 🟢 안정 - 정상 동작 가능
- **절감량**: **9,992 바이트** (30.5% 감소) — 예상(8,192 바이트)보다 우수

### 최적화 전 상황 (참고)

- **SRAM 사용량**: 31,612 / 32,768 바이트 (**96.5%**)
- **여유 공간**: 1,156 바이트 (3.5%)
- **상태**: 🔴 매우 위험 - 즉각적인 최적화 필요

---

## 🔍 주요 SRAM 사용처 분석

### 1. **WiFi Access Point 배열** - 🥇 최대 사용처
- **위치**: `lib/WIFI.c:16`
- **크기**: `ACCESS_POINT g_APs[40]` = **2,400 바이트**
- **구조**: 각 항목 60바이트 (RSSI 2 + SSID 40 + MAC 18)
- **문제점**: 40개 AP 저장은 과도함

### 2. **이미지 데이터** - 🥈 두 번째 사용처
- **위치**: `lib/image.h`
- **총 크기**: 약 **6-8KB**
- **주요 항목**:
  - `image_wifi_0[1024]`: 2,048 바이트 (활성화)
  - `image_wifi_4[1024]`: 2,048 바이트 (활성화)
  - `image_suntech[]`: ~2KB
  - `image_danger[]`: ~2KB
  - 화살표 이미지들 등

### 3. **폰트 데이터** - 🥉 세 번째 사용처
- **위치**: `lib/fonts.h`
- **총 크기**: 약 **6.5KB**
- **주요 항목**:
  - `AlibriNumBold32x48[1924]`: 1,924 바이트
  - `Arial_round_16x24[4564]`: 4,564 바이트

### 4. **통신 버퍼들**
- **총 크기**: 약 **6KB**
- **상세**:
  - `g_WIFI_ReceiveBuffer[1025]`: 1,025 바이트 (`lib/WIFI.c:13`)
  - `g_USB_ReceiveBuffer[501]`: 501 바이트 (`lib/USB.c:59`)
  - `g_UART_buff[512]`: 512 바이트 (`uartJson.c:23`)
  - `uart.c`의 로컬 `buff[1024]` x2: 2,048 바이트 (스택)
  - `jsonUtil.c:131`의 static `buff[1024]`: 1,024 바이트
  - `lib/WIFI.c:405`의 로컬 `buff[1024]`: 1,024 바이트 (스택)

### 5. **구조체 인스턴스**
- `g_Info` (notice 배열 포함): 약 400 바이트
- `g_AndonLists`: 약 200 바이트
- `g_network`: 약 150 바이트

---

## 💡 최적화 권장사항 (우선순위별)

### ⭐ **우선순위 1: 즉시 적용 가능 (예상 절감: 약 10-12KB)**

#### 1-1. 이미지/폰트 데이터를 FLASH로 이동 ✅
**절감량**: ~**8-10KB**

```c
// lib/image.h, lib/fonts.h - 현재
static uint16_t image_wifi_0[] = { ... };
static fontdatatype AlibriNumBold32x48[1924] = { ... };

// 수정 제안 - const 추가로 FLASH에 저장
static const uint16_t image_wifi_0[] = { ... };
static const fontdatatype AlibriNumBold32x48[1924] = { ... };
```

**근거**:
- 이미지와 폰트는 읽기 전용 데이터
- `const` 키워드 추가 시 컴파일러가 자동으로 FLASH에 배치
- SRAM 부담 없이 동일하게 사용 가능
- **가장 효과적이고 안전한 방법**

**적용 대상 파일**:
- `lib/image.h`: 모든 `image_*` 배열
- `lib/fonts.h`: `AlibriNumBold32x48`, `Arial_round_16x24` 등 모든 폰트 배열

#### 1-2. WiFi Access Point 배열 축소 ✅
**절감량**: ~**2,000 바이트**

```c
// lib/WIFI.h - 현재
#define MAX_NO_OF_ACCESS_POINT 40

// 수정 제안
#define MAX_NO_OF_ACCESS_POINT 10  // 또는 15
```

**근거**:
- 실제로 40개의 AP를 모두 사용하는 경우는 드뭅니다
- 10-15개면 대부분의 환경에서 충분합니다
- 10개로 줄이면: 40 → 10 = 30개 절약 = 1,800 바이트 절감

#### 1-3. 사용하지 않는 이미지 비활성화 ✅
**절감량**: ~**2-4KB**

```c
// lib/image.h - 현재 활성화된 것만 사용
#define USE_WIFI_IAMGE0
#define USE_WIFI_IAMGE4

// 실제로 사용하지 않는 이미지가 있다면 주석 처리
// #define USE_WIFI_IAMGE1
// #define USE_WIFI_IAMGE2
// #define USE_WIFI_IAMGE3
```

**작업**: 코드 검토하여 실제 사용 중인 이미지만 활성화

---

### ⭐ **우선순위 2: 중간 난이도 (예상 절감: 약 1-2KB)**

#### 2-1. 통신 버퍼 크기 최적화 ✅

```c
// 현재
#define MAX_WIFI_RECEIVE_BUFFER 1024  // lib/WIFI.h
#define MAX_USB_RECEIVE_BUFFER 500    // lib/USB.h
#define UART_BUFFER_SIZE 512          // uartJson.c

// 제안 (실제 필요한 최대 크기 확인 후)
#define MAX_WIFI_RECEIVE_BUFFER 768   // -256 바이트
#define MAX_USB_RECEIVE_BUFFER 384    // -116 바이트
#define UART_BUFFER_SIZE 384          // -128 바이트
```

**절감량**: ~**500 바이트**

**주의**: 실제 통신 데이터 크기를 먼저 확인해야 합니다.

#### 2-2. uart.c의 큰 로컬 버퍼 최적화 ✅

`uart.c`의 `uart_printf()`와 `uart_vprintf()` 함수에서 `char buff[1024]`를 사용 중입니다.

```c
// uart.c - 현재
void uart_printf(const char *fmt, ...) {
    char buff[1024];  // 스택에 1KB
    // ...
}

void uart_vprintf(const char *fmt, ...) {
    char buff[1024];  // 스택에 1KB
    // ...
}

// 제안
void uart_printf(const char *fmt, ...) {
    char buff[256];  // 또는 512
    // ...
}
```

**절감량**: ~**1,024-1,536 바이트** (스택 절약)

---

### ⭐ **우선순위 3: 고급 최적화 (예상 절감: 추가 1-2KB)**

#### 3-1. 공유 버퍼 사용 전략 ✅

여러 통신 버퍼가 동시에 사용되지 않는다면 하나의 버퍼를 공유:

```c
// 공통 헤더에 정의
#define SHARED_BUFFER_SIZE 1024
extern char g_sharedBuffer[SHARED_BUFFER_SIZE];

// WIFI, USB, UART가 동시에 사용하지 않으면 공유 가능
```

**절감량**: ~**1-2KB** (구현 방식에 따라)

#### 3-2. ANDON Notice 배열 최적화 ✅

```c
// andonApi.h - 현재
#define MAX_COL_NOTICE 14
#define MAX_ROW_NOTICE 20
char notice[MAX_ROW_NOTICE][MAX_COL_NOTICE+1];  // 300 바이트

// 제안 - 실제 필요한 크기 확인 후 축소
#define MAX_ROW_NOTICE 15  // 또는 더 적게
```

**절감량**: ~**75-150 바이트**

---

## 📋 구현 단계별 계획

### **1단계: 즉시 적용 (30분 소요)**
1. ✅ 모든 이미지/폰트 배열에 `const` 키워드 추가 → **8-10KB 절감**
2. ✅ `MAX_NO_OF_ACCESS_POINT` 40 → 10으로 변경 → **1.8KB 절감**
3. ✅ 컴파일 테스트

**실측 SRAM 사용량**: 31,612 → **21,620 바이트 (66.0%)** ✅ 완료

### **2단계: 버퍼 최적화 (1-2시간 소요)**
1. ✅ 통신 버퍼 크기 축소 (실제 사용량 확인 후)
2. ✅ uart.c 로컬 버퍼 크기 축소
3. ✅ 기능 테스트

**추가 절감**: **~1.5KB**

**예상 SRAM 사용량**: **약 20,000 바이트 (61%)**

### **3단계: 고급 최적화 (필요시)**
- 공유 버퍼 구현
- 동적 할당 검토

---

## 🎯 최적화 결과

| 단계 | SRAM 사용량 | 사용률 | 여유 공간 | 상태 |
|------|-------------|--------|-----------|------|
| **최적화 전** | 31,612 바이트 | 96.5% | 1,156 바이트 | 🔴 위험 |
| **1단계 완료** | **21,620 바이트** | **66.0%** | **11,148 바이트** | ✅ **안정** |
| **절감량** | **-9,992 바이트** | **-30.5%** | **+9,992 바이트** | 🎉 예상 초과 |

### 추가 최적화 가능 (선택사항)

| 단계 | 예상 SRAM | 예상 사용률 | 예상 여유 공간 |
|------|-----------|-------------|----------------|
| **2단계 후** | ~20,100 바이트 | ~61% | ~12,700 바이트 |
| **3단계 후** | ~18,100 바이트 | ~55% | ~14,700 바이트 |

---

## ⚠️ 주의사항

1. **const 키워드 추가 시**: 코드 동작은 변경 없음, 안전하게 적용 가능
2. **버퍼 크기 축소 시**: 반드시 실제 최대 데이터 크기 확인 필요
3. **AP 배열 축소 시**: WiFi 스캔 기능 테스트 필요
4. **변경 후**: 전체 시스템 기능 테스트 필수

---

## 📝 변경 이력

| 날짜 | 단계 | 작업 내용 | 결과 |
|------|------|-----------|------|
| 2025-12-20 | 분석 | SRAM 사용 현황 분석 완료 (IoT_PWJ 기반) | 보고서 작성 |
| 2026-03-09 | 1단계 | lib/image.h - 13개 이미지 배열 `static uint16_t` → `static const uint16_t` | ✅ 완료 |
| 2026-03-09 | 1단계 | lib/WIFI.h - `MAX_NO_OF_ACCESS_POINT` 40 → 10 | ✅ 완료 |
| 2026-03-09 | 빌드 확인 | PSoC Creator 재빌드 결과 확인 — SRAM 21,620/32,768 (66.0%) | ✅ 완료 |

> **참고**: `lib/fonts.h`는 `fontdatatype = const unsigned char` 정의로 이미 FLASH 배치됨 — 별도 수정 불필요

---

## 📞 문의사항

최적화 작업 중 문제가 발생하면:
1. 백업 확인
2. 이전 버전으로 롤백
3. 단계별로 하나씩 적용하여 문제 원인 파악
