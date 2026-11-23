"""Export U8g2 bitmap glyphs used by the firmware; no platform font substitution."""
import ast
import json
import re
from pathlib import Path

root = Path(__file__).resolve().parent.parent
source = (root / '.pio/libdeps/esp32-c6-devkitc-1/U8g2/src/clib/u8g2_fonts.c').read_text(encoding='latin1')
fonts = {}
for name in ['5x8', 'helvB10', 'helvB08']:
    symbol = f'u8g2_font_{name}_tf'
    match = re.search(rf'const uint8_t {symbol}\[(\d+)\].*?=\s*((?:"(?:[^"\\]|\\.)*"\s*)+);', source, re.S)
    data = b''.join(ast.literal_eval('b' + literal) for literal in re.findall(r'"(?:[^"\\]|\\.)*"', match[2])) + b'\0'
    assert len(data) == int(match[1]), (symbol, len(data), match[1])
    glyphs = {}
    pos = 23
    while data[pos + 1]:
        code, length = data[pos:pos + 2]
        bit = (pos + 2) * 8
        def read(count):
            global bit
            value = sum(((data[(bit+i)//8] >> ((bit+i)%8)) & 1) << i for i in range(count))
            bit += count
            return value
        width, height = read(data[4]), read(data[5])
        x = read(data[6]) - (1 << (data[6]-1))
        y = read(data[7]) - (1 << (data[7]-1))
        advance = read(data[8]) - (1 << (data[8]-1))
        pixels = []
        while width and len(pixels) < width * height:
            zero, one = read(data[2]), read(data[3])
            while True:
                pixels.extend([0] * zero + [1] * one)
                if not read(1): break
        glyphs[str(code)] = dict(w=width, h=height, x=x, y=y, advance=advance, bits=''.join(map(str,pixels[:width*height])))
        pos += length
    fonts[name] = glyphs
out = root / 'app/src/bianca/sign-fonts.json'
out.write_text(json.dumps(fonts, separators=(',', ':')) + '\n')
print(f'Exported {sum(map(len,fonts.values()))} glyphs to {out}')
