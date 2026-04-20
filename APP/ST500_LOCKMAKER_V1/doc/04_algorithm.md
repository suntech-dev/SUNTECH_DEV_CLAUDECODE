# 04. 잠금 코드 생성 알고리즘

## 개요

Old Code(기존 잠금코드)와 Lock Day(잠금 일수)를 입력받아 New Code를 계산하는 수학 알고리즘.  
원본: `lockmaker_211206/app/src/main/java/org/suntechdev/lockmaker/lockmake.kt` `MakeCalNum()` 함수.

> ⚠️ **보안 위험**: 이 알고리즘이 클라이언트 코드에 노출되면 누구나 임의 코드를 생성할 수 있음.  
> 향후 **PHP 서버 API로 이전** 검토 필요. (doc/08_roadmap.md 참조)

---

## 알고리즘 상세

### 입력값

| 항목 | 타입 | 조건 | 설명 |
|---|---|---|---|
| `oldCode` | string | 8~9자리 숫자 | 기존 잠금 코드 |
| `lockDay` | number | 1~999 | 잠금 일수 |
| `isUnlock` | boolean | — | true이면 lockDay = 151 (영구 해제) |

### 출력값

| 항목 | 타입 | 형식 |
|---|---|---|
| `newCode` | string | 9자리 숫자 (앞에 '0' 패딩 가능) |

---

### 단계별 계산

**Step 1: Old Code를 3분할**
```
oldCode = 9자리 숫자 → 정수 변환
n1 = Math.floor(oldCode / 1000000)    // 상위 3자리
n2 = Math.floor((oldCode % 1000000) / 1000)  // 중위 3자리
n3 = oldCode % 1000                   // 하위 3자리
```

**Step 2: 각 3자리를 백/십/일 자리로 분해 후 가중치 계산**

```javascript
function calcNum(n, [a, b, c, d]) {
  const i1 = Math.floor(n / 100)           // 백의 자리
  const i2 = Math.floor((n % 100) / 10)   // 십의 자리
  const i3 = n % 10                        // 일의 자리
  return Math.floor(
    i1*a + i2*b + i3*c + (i1*100 + i2*10 + i3)*d
  ) % 1000
}
```

| 계산 대상 | 가중치 계수 `[a, b, c, d]` |
|---|---|
| n1 → c1 | `[3.305982, 2.358196, 1.141059, 6.78213]` |
| n2 → c2 | `[3.219283, 1.153023, 2.019283, 8.23143]` |
| n3 → c3 | `[1.113569, 9.123123, 7.213213, 6.12374]` |

**Step 3: Lock Day 가산 및 mod 1000**
```javascript
c1 = (c1 + lockDay) % 1000
c2 = (c2 + lockDay) % 1000
c3 = (c3 + lockDay) % 1000
```

**Step 4: New Code 조합**
```javascript
const result = c1 * 1000000 + c2 * 1000 + c3
// 9자리 미만이면 앞에 '0' 패딩
newCode = result < 100000000 ? '0' + String(result) : String(result)
```

---

### UnLock 모드

```javascript
if (isUnlock) {
  lockDay = 151  // 영구 잠금 해제 고정값
}
```

- 체크박스 선택 시 Lock Day 입력 필드 비활성화
- Lock Day = 151로 고정하여 계산
- 기계의 영구 잠금 해제 코드 생성

---

## JavaScript 구현 전문

**파일**: `src/views/LockMakeView.vue` (generate 함수)

```javascript
function calcNum(n, coeffs) {
  const [a, b, c, d] = coeffs
  const i1 = Math.floor(n / 100)
  const i2 = Math.floor((n % 100) / 10)
  const i3 = n % 10
  return Math.floor(i1 * a + i2 * b + i3 * c + (i1 * 100 + i2 * 10 + i3) * d) % 1000
}

function generate() {
  const code = oldCode.value.trim()
  if (code.length < 8) { showToast('OLD CODE를 8자리 이상 입력하세요'); return }

  let day = isUnlock.value ? 151 : parseInt(lockDay.value, 10)

  const codeNum = parseInt(code, 10)
  const n1 = Math.floor(codeNum / 1000000)
  const rem = codeNum - n1 * 1000000
  const n2 = Math.floor(rem / 1000)
  const n3 = rem % 1000

  let c1 = calcNum(n1, [3.305982, 2.358196, 1.141059, 6.78213])
  let c2 = calcNum(n2, [3.219283, 1.153023, 2.019283, 8.23143])
  let c3 = calcNum(n3, [1.113569, 9.123123, 7.213213, 6.12374])

  c1 = (c1 + day) % 1000
  c2 = (c2 + day) % 1000
  c3 = (c3 + day) % 1000

  const result = c1 * 1000000 + c2 * 1000 + c3
  newCode.value = result < 100000000 ? '0' + String(result) : String(result)
}
```

---

## 원본 Kotlin 코드 (참고용)

```kotlin
// lockmake.kt MakeCalNum()
fun MakeCalNum(ii1: Long, ii2: Long, ii3: Long) {
    var i1: Double; var i2: Double; var i3: Double
    i1 = (ii1 / 100).toDouble()
    i2 = ((ii1 % 100) / 10).toDouble()
    i3 = (ii1 % 10).toDouble()
    CalNum1 = (i1*3.305982 + i2*2.358196 + i3*1.141059 + (i1*100+i2*10+i3)*6.78213).toLong() % 1000
    i1 = (ii2 / 100).toDouble()
    i2 = ((ii2 % 100) / 10).toDouble()
    i3 = (ii2 % 10).toDouble()
    CalNum2 = (i1*3.219283 + i2*1.153023 + i3*2.019283 + (i1*100+i2*10+i3)*8.23143).toLong() % 1000
    i1 = (ii3 / 100).toDouble()
    i2 = ((ii3 % 100) / 10).toDouble()
    i3 = (ii3 % 10).toDouble()
    CalNum3 = (i1*1.113569 + i2*9.123123 + i3*7.213213 + (i1*100+i2*10+i3)*6.12374).toLong() % 1000
}
```

> Kotlin `toLong()` = JavaScript `Math.floor()` — 동일 결과 확인 필요.
