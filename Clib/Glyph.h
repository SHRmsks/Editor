#ifndef GLYPH_H
#define GLYPH_H
#include "PieceTable.h"
typedef struct
{
    float advance, plane_l, plane_r, plane_t, plane_b, atlas_l, atlas_r, atlas_t, atlas_b;
} Glyph;
WASM_EXPORT void load_glyph_metric(int char_code, float adv, float pl, float pb, float pr, float pt, float al, float ab, float ar, float at);
#endif // GLYPH_H
