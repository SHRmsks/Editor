#ifndef RENDER_H
#define RENDER_H
#include <stdint.h>
#include <stddef.h>
#include "PieceTable.h"
#include "redBlkTree.h"
#define MAX_CMD 10000
typedef struct
{
    float x; // using float for lighter memory
    float y;
    char *text_ptr;
    int32_t length; // the bytes length of the text
} CMD;
typedef struct
{
    float viewPort_x;
    float viewPort_y;
    float line_h;
    float font_size;
} Config;
static Config config = {0.0f, 0.0f, 20.0f, 14.0f};
// report from JS side for text width measurement
extern float meaure_text_width(char *text, int32_t length, int32_t font_sz);
void update_config(float vp_x, float vp_y, float line_h, float font_sz);
void generate_frame(float scroll_y);
#endif // RENDER_H