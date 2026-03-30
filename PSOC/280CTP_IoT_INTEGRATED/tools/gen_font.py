"""
PSoC LCD 비트맵 폰트 생성기 v2
폰트 배열 구조: [width, height, ascii_start, num_chars, bitmap...]
렌더러: UI.c LCD_DrawFont() — MSB-first, bytes_per_row=ceil(width/8)
"""

from PIL import Image, ImageDraw, ImageFont
import os, sys

WINDOWS_FONTS = "C:/Windows/Fonts"
ASCII_START   = 0x20   # ' '
NUM_CHARS     = 95     # 0x20 ~ 0x7E

def find_font(candidates):
    for name in candidates:
        path = os.path.join(WINDOWS_FONTS, name)
        if os.path.exists(path):
            return path
    return None

def render_char(ttf_path, pt_size, char, target_w, target_h, threshold=100):
    """문자를 target_w × target_h 1-bit 비트맵으로 렌더링 (세로 중앙 정렬)"""
    # 큰 캔버스에 안티앨리어싱 렌더링 후 찾기
    SCALE = 4
    canvas_w = target_w * SCALE * 3
    canvas_h = target_h * SCALE * 3
    font_obj  = ImageFont.truetype(ttf_path, pt_size * SCALE)

    img  = Image.new('L', (canvas_w, canvas_h), 0)
    draw = ImageDraw.Draw(img)
    draw.text((canvas_w // 3, canvas_h // 4), char, fill=255, font=font_obj)

    # 실제 픽셀 영역 찾기 (bounding box)
    bbox = img.getbbox()
    if bbox is None:
        return Image.new('1', (target_w, target_h), 0)

    cropped = img.crop(bbox)
    cw, ch  = cropped.size

    # target_w × target_h 에 맞게 비례 축소 (여백 포함)
    pad_x = max(1, int(target_w  * SCALE * 0.08))
    pad_y = max(1, int(target_h  * SCALE * 0.08))
    fit_w = target_w  * SCALE - pad_x * 2
    fit_h = target_h  * SCALE - pad_y * 2

    scale  = min(fit_w / cw, fit_h / ch)
    new_w  = max(1, int(cw * scale))
    new_h  = max(1, int(ch * scale))
    scaled = cropped.resize((new_w, new_h), Image.LANCZOS)

    # target_w*SCALE × target_h*SCALE 캔버스에 수직/수평 중앙 배치
    big = Image.new('L', (target_w * SCALE, target_h * SCALE), 0)
    ox  = (target_w * SCALE - new_w) // 2
    oy  = (target_h * SCALE - new_h) // 2
    big.paste(scaled, (ox, oy))

    # 다운샘플 → 임계값
    small = big.resize((target_w, target_h), Image.LANCZOS)
    bw    = small.point(lambda p: 1 if p >= threshold else 0, '1')
    return bw

def char_to_bytes(bw_img, w, h):
    bpr    = (w + 7) // 8
    result = []
    for row in range(h):
        for col_byte in range(bpr):
            val = 0
            for bit in range(8):
                col = col_byte * 8 + bit
                if col < w and bw_img.getpixel((col, row)):
                    val |= (0x80 >> bit)
            result.append(val)
    return result

def generate(ttf_path, pt_size, target_w, target_h, name, threshold=100):
    bpr            = (target_w + 7) // 8
    bytes_per_char = bpr * target_h
    total          = 4 + NUM_CHARS * bytes_per_char

    lines = []
    lines.append(f"#ifdef _FONT_{name}_")
    lines.append(f"fontdatatype {name}[{total}] PROGMEM={{")
    lines.append(f"0x{target_w:02X},0x{target_h:02X},0x{ASCII_START:02X},0x{NUM_CHARS:02X},  "
                 f"// width={target_w}, height={target_h}, ascii_start={ASCII_START}, num={NUM_CHARS}")

    for code in range(ASCII_START, ASCII_START + NUM_CHARS):
        ch    = chr(code)
        bw    = render_char(ttf_path, pt_size, ch, target_w, target_h, threshold)
        data  = char_to_bytes(bw, target_w, target_h)
        hexs  = ','.join(f'0x{b:02X}' for b in data)
        label = ch if ch.strip() else 'SP'
        lines.append(f"{hexs},  // {code} '{label}'")

    lines.append("};")
    lines.append("#endif")
    return '\n'.join(lines), total

# ── 폰트 선택 ──────────────────────────────────────────────────────────────
# 작은 크기에서도 획이 굵고 선명한 폰트 우선
candidates = ["verdanab.ttf", "arialbd.ttf", "verdana.ttf", "arial.ttf", "calibrib.ttf", "calibri.ttf"]
ttf_path   = find_font(candidates)
if not ttf_path:
    print("ERROR: 적합한 폰트를 찾을 수 없습니다.", file=sys.stderr)
    sys.exit(1)
print(f"// 사용 폰트: {ttf_path}")

# pt_size: 캔버스가 SCALE=4배이므로 실제 렌더링 pt = pt_size*4
# 12x16 → target_h=16, 문자 높이의 ~75% = 12px 캡하이트 → pt~11
# 10x20 → target_h=20, 문자 높이의 ~75% = 15px 캡하이트 → pt~13
src12, sz12 = generate(ttf_path, 11, 12, 16, "Font12x16", threshold=90)
src10, sz10 = generate(ttf_path, 13, 10, 20, "Font10x20", threshold=90)

out_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "generated_fonts.h")
with open(out_path, 'w', encoding='utf-8') as f:
    f.write("// 자동 생성된 폰트 데이터 — gen_font.py\n")
    f.write(f"// Font12x16 : {sz12} bytes (12px wide x 16px high, 95 chars)\n")
    f.write(f"// Font10x20 : {sz10} bytes (10px wide x 20px high, 95 chars)\n\n")
    f.write(src12)
    f.write("\n\n")
    f.write(src10)

print(f"생성 완료 → {out_path}")
print(f"  Font12x16 = {sz12} bytes | Flash: {sz12} bytes")
print(f"  Font10x20 = {sz10} bytes | Flash: {sz10} bytes")

# ── 간단한 텍스트 미리보기 ────────────────────────────────────────────────
def preview(ttf_path, pt_size, w, h, name, threshold=90, sample="AaBbCc123"):
    print(f"\n▶ {name} ({w}x{h}) 미리보기: '{sample}'")
    for ch in sample:
        bw = render_char(ttf_path, pt_size, ch, w, h, threshold)
        for row in range(h):
            line = ""
            for col in range(w):
                line += "#" if bw.getpixel((col, row)) else "."
            print(f"  {line}")
        print()

preview(ttf_path, 11, 12, 16, "Font12x16", sample="A1B")
preview(ttf_path, 13, 10, 20, "Font10x20", sample="A1B")
