"""
Font8x16 (IBM VGA 비트맵) 기반으로 Font12x16, Font10x20 생성
 - 동일한 소스 비트맵을 nearest-neighbor 스케일링
 - 폰트간 높이/굵기 일관성 보장
"""
import re, os

SRC_FONTS_H = (
    "C:/SUNTECH_DEV_CLAUDECODE/PSOC/280CTP_IoT_UART_TEST"
    "/280CTP_IoT_UART_TEST_V1/Project/Design.cydsn/lib/fonts.h"
)
OUT_PATH = "C:/SUNTECH_DEV_CLAUDECODE/tools/generated_fonts.h"

# ── 1. Font8x16 원본 데이터 파싱 ─────────────────────────────────────────
with open(SRC_FONTS_H, encoding='utf-8', errors='replace') as f:
    content = f.read()

start = content.find('fontdatatype Font8x16[')
end   = content.find('};', start) + 2
block = content[start:end]

# 모든 0x?? 값 추출
raw = [int(x, 16) for x in re.findall(r'0x[0-9A-Fa-f]{2}', block)]
print(f"Font8x16 총 바이트: {len(raw)}")

# 헤더 4바이트 검증
src_w, src_h, ascii_start, num_chars = raw[0], raw[1], raw[2], raw[3]
print(f"  width={src_w}, height={src_h}, start=0x{ascii_start:02X}({chr(ascii_start)}), chars={num_chars}")
assert src_w == 8 and src_h == 16 and num_chars == 95

src_bpr      = 1           # bytes per row for 8px = 1
src_bpc      = src_bpr * src_h   # bytes per char = 16
char_data    = raw[4:]     # 실제 문자 데이터
assert len(char_data) == num_chars * src_bpc, f"데이터 크기 불일치: {len(char_data)}"

def get_pixel(char_idx, row, col):
    """Font8x16에서 (row, col) 픽셀 반환 (0 or 1)"""
    base = char_idx * src_bpc + row * src_bpr
    byte = char_data[base + col // 8]
    return 1 if (byte & (0x80 >> (col % 8))) else 0

# ── 2. Nearest-Neighbor 스케일링 ─────────────────────────────────────────
def nn_scale(src_w, src_h, dst_w, dst_h, get_src_pixel):
    """
    nearest-neighbor 스케일링 결과를 2D 리스트로 반환 [row][col] = 0/1
    공식: src_x = round((dst_x + 0.5) * src_w / dst_w - 0.5)
    """
    result = []
    for dr in range(dst_h):
        row = []
        sr = min(src_h - 1, int((dr + 0.5) * src_h / dst_h))
        for dc in range(dst_w):
            sc = min(src_w - 1, int((dc + 0.5) * src_w / dst_w))
            row.append(get_src_pixel(sr, sc))
        result.append(row)
    return result

def bitmap_to_bytes(bitmap, dst_w, dst_h):
    """2D 비트맵 → 바이트 배열 (MSB-first, bytes_per_row = ceil(dst_w/8))"""
    bpr = (dst_w + 7) // 8
    result = []
    for row in bitmap:
        for b in range(bpr):
            val = 0
            for i in range(8):
                col = b * 8 + i
                if col < dst_w and row[col]:
                    val |= (0x80 >> i)
            result.append(val)
    return result

# ── 3. 폰트 생성 함수 ────────────────────────────────────────────────────
def generate(dst_w, dst_h, name, comment):
    bpr  = (dst_w + 7) // 8
    bpc  = bpr * dst_h
    total = 4 + num_chars * bpc

    lines = []
    lines.append(f"#ifdef _FONT_{name}_")
    lines.append(f"// {comment}")
    lines.append(f"static")
    lines.append(f"fontdatatype {name}[{total}] PROGMEM={{")
    lines.append(f"0x{dst_w:02X},0x{dst_h:02X},0x{ascii_start:02X},0x{num_chars:02X},")

    for ci in range(num_chars):
        ch = chr(ascii_start + ci)

        def get_px(row, col, ci=ci):
            return get_pixel(ci, row, col)

        bitmap = nn_scale(src_w, src_h, dst_w, dst_h, get_px)
        data   = bitmap_to_bytes(bitmap, dst_w, dst_h)
        hexs   = ','.join(f'0x{b:02X}' for b in data)
        label  = ch if ch.strip() else '<space>'
        lines.append(f"{hexs},  // {label}")

    lines.append("};")
    lines.append("#endif")
    return '\n'.join(lines), total

# ── 4. 생성 ──────────────────────────────────────────────────────────────
src12, sz12 = generate(12, 16, "Font12x16",
    "12x16  IBM VGA 8x16 기반 1.5x 너비 스케일  ASCII 0x20~0x7E  (95 chars)")
src10, sz10 = generate(10, 20, "Font10x20",
    "10x20  IBM VGA 8x16 기반 1.25x 스케일  ASCII 0x20~0x7E  (95 chars)")

with open(OUT_PATH, 'w', encoding='utf-8') as f:
    f.write("// 자동 생성 — Font8x16 (IBM VGA) 비트맵 스케일링\n")
    f.write(f"// Font12x16 : {sz12} bytes\n")
    f.write(f"// Font10x20 : {sz10} bytes\n\n")
    f.write(src12)
    f.write("\n\n")
    f.write(src10)

print(f"\n생성 완료 → {OUT_PATH}")
print(f"  Font12x16 = {sz12} bytes")
print(f"  Font10x20 = {sz10} bytes")

# ── 5. 텍스트 미리보기 ───────────────────────────────────────────────────
def preview(dst_w, dst_h, name, sample="A0BRi"):
    print(f"\n▶ {name} ({dst_w}x{dst_h})")
    for ch in sample:
        ci = ord(ch) - ascii_start
        def get_px(row, col, ci=ci):
            return get_pixel(ci, row, col)
        bitmap = nn_scale(src_w, src_h, dst_w, dst_h, get_px)
        print(f"  '{ch}':")
        for row in bitmap:
            print("    " + ''.join('#' if p else '.' for p in row))

preview(12, 16, "Font12x16", "A0B")
preview(10, 20, "Font10x20", "A0B")
