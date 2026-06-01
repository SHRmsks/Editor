#include "Glyph.h"
Glyph fonts[256]; // 256 ASCII characters
// initialize the fonts array
void load_glyph_metric(int char_code, float adv, float pl, float pb, float pr, float pt, float al, float ab, float ar, float at)
{
    fonts[char_code].advance = adv;
    fonts[char_code].plane_l = pl;
    fonts[char_code].plane_r = pr;
    fonts[char_code].plane_t = pt;
    fonts[char_code].plane_b = pb;
    fonts[char_code].atlas_l = al;
    fonts[char_code].atlas_r = ar;
    fonts[char_code].atlas_t = at;
    fonts[char_code].atlas_b = ab;
}
