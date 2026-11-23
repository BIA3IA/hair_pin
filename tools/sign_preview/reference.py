"""Build golden rasters from the firmware's unmodified wrap/draw_text functions."""
import json
import subprocess
from pathlib import Path
root = Path(__file__).resolve().parents[2]
source = (root/'src/cartello/cartello.cpp').read_text()
fonts = json.loads((root/'app/src/bianca/sign-fonts.json').read_text())
out = Path('/tmp/bianca-sign-reference'); out.mkdir(exist_ok=True)
def function(name):
    start = source.index('static ', source.index(name) - 30)
    # Find the definition, skipping forward declarations.
    at = source.index(name, start)
    brace = source.index('{', at)
    while ';' in source[at:brace]:
        at = source.index(name, brace)
        start = source.rfind('static ', 0, at)
        brace = source.index('{', at)
    depth = 1; end = brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}'); end += 1
    return source[start:end]
header = '''#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <climits>
#include <string>
#include <unordered_map>
using std::min; using std::max;
struct Glyph {int w,h,x,y,advance; const char *bits;};
std::unordered_map<int,Glyph> fonts[3];
const uint8_t u8g2_font_5x8_tf[]={0}, u8g2_font_helvB10_tf[]={1}, u8g2_font_helvB08_tf[]={2};
'''
setup = 'void init_fonts() {\n'
for index, glyphs in enumerate(fonts.values()):
    for code, g in glyphs.items():
        setup += f'fonts[{index}][{code}] = {{{g["w"]},{g["h"]},{g["x"]},{g["y"]},{g["advance"]},"{g["bits"]}"}};\n'
setup += '}\n'
header += setup
header += '''
class Gfx {
 int font=0, scale=1, cx=0, cy=0;
 public:
 uint8_t pixels[90*56] = {};
 void setFont(const uint8_t *f) {font=*f;}
 void setTextSize(int s) {scale=s;}
 void setUTF8Print(bool) {}
 void setTextColor(int) {}
 void setCursor(int x,int y) {cx=x;cy=y;}
 template<class F> void each(const char *s,F fn) {
  const unsigned char *p=(const unsigned char*)s;
  while(*p) {
   unsigned code=*p++;
   if(code>=192) {unsigned n=code>=240?3:code>=224?2:1;code &= (1<<(6-n))-1;while(n-- && *p)code=(code<<6)|(*p++&63);}
   auto found=fonts[font].find(code);if(found!=fonts[font].end()) fn(found->second);
  }
 }
 void getTextBounds(const char *s,int16_t x,int16_t y,int16_t *ox,int16_t *oy,uint16_t *w,uint16_t *h) {
  int minx=239,miny=279,maxx=0,maxy=0;
  each(s,[&](Glyph g){int x1=x+g.x*scale,y1=y-(g.h+g.y)*scale;
   minx=min(minx,x1);miny=min(miny,y1);maxx=max(maxx,x1+g.w*scale-1);maxy=max(maxy,y1+g.h*scale-1);x+=g.advance*scale;});
  *ox=0;*oy=0;*w=0;*h=0;if(maxx>=minx){*ox=minx;*w=maxx-minx+1;}if(maxy>=miny){*oy=miny;*h=maxy-miny+1;}
 }
 void print(const char *s) {
  each(s,[&](Glyph g){for(int y=0;y<g.h;y++)for(int x=0;x<g.w;x++)if(g.bits[y*g.w+x]=='1')
   for(int dy=0;dy<scale;dy++)for(int dx=0;dx<scale;dx++){
    int px=cx+(g.x+x)*scale+dx,py=cy-(g.h+g.y-y)*scale+dy;
    if(px>=0&&px<90&&py>=0&&py<56)pixels[py*90+px]=1;
   }cx+=g.advance*scale;});
 }
} device;
Gfx *gfx=&device;
const int MAX_LINES=4, MAX_LINE_BYTES=48, SIDE_MARGIN=4, SIGN_TEXT_COLOR=0x18c3;
struct TextStyle {const uint8_t *font; uint8_t scale; int line_h,baseline,bold_x,bold_y;};
'''
start = source.index('const TextStyle HUGE')
end = source.index('static int last_phrase')
header += source[start:end]
# Functions are copied directly from the firmware for this verification.
for name in ['use_style(', 'text_width(', 'wrap(']:
    header += function(name) + '\n'
# misura e disegno del testo: dal blocco TextBlock fino a draw_text compresa
start = source.index('struct TextBlock {')
header += source[start:source.index('\nvoid sign_draw', start)]
header += '\nint main(int argc,char **argv){init_fonts();draw_text(argv[1],0,0,90,56);for(auto p:device.pixels)putchar(p?49:48);}\n'
(out/'main.cpp').write_text(header)
subprocess.run(['c++','-std=c++17','-O2',str(out/'main.cpp'),'-o',str(out/'reference')],check=True)
examples = ['FAME','Caffè','Ti voglio bene','OVER-\nTHINKING','Ho dimenticato qualcosa?','Un secondo','éàèìòù','  Che  bello  ','Andiamo a spasso?','W'*48]
fixtures = [{ 'text': text, 'pixels': subprocess.check_output([str(out/'reference'),text]).decode() } for text in examples]
(root/'app/test/sign-reference.json').write_text(json.dumps(fixtures,ensure_ascii=False)+'\n')
print(f'Generated {len(fixtures)} reference rasters from firmware layout functions')
