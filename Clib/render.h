#ifndef RENDER_H
#define RENDER_H
#include "Cutils.h"
#include "PieceTable.h"
#include "redBlkTree.h"
#define MAX_CHAR 10000
// One Char is 1 triangle = 6 vertices. Each Vertex has 4 components (x, y, u, v), together 24 floats per character
float VBO_buffer[MAX_CHAR * 24];
float CURSOR_VBO[12]; // x, y for 2 triangles
float SELECTION_VBO[MAX_CHAR * 12];
/* configuration from the browser, we will get the info from the frontend side*/
typedef struct
{
    float viewPort_x; // browser viewport width
    float viewPort_y; // browser viewport height
    float line_h;     // line height for text
    float font_size;  // font size for text
    float padding_x;
    float padding_y;
} Config;
// For CACHED_CURSOR
typedef struct
{
    uint32_t idx;
    float x;
    float y;
} Cursor;
extern Cursor cached_cursor;

extern Config config;
// report from JS side for text width measurement
WASM_EXPORT void update_config(float vp_x, float vp_y, float line_h, float font_sz, float padding_x, float padding_y);
WASM_EXPORT uint32_t get_document_length();
WASM_EXPORT uint32_t get_selection_text(uint32_t start_idx, uint32_t len);
WASM_EXPORT void generate_VBO_frame(float scroll_y);
WASM_EXPORT float *get_vbo_buffer();
WASM_EXPORT int get_vbo_count();
WASM_EXPORT int get_selection_vbo_count();
WASM_EXPORT uint32_t update_cursor_VBO(float cursor_x, float cursor_y);
WASM_EXPORT float *get_CURSOR_VBO();
WASM_EXPORT float *get_SELECTION_VBO();
WASM_EXPORT void update_cursor_index_VBO(uint32_t target_idx);
WASM_EXPORT float get_current_cursor_x();
WASM_EXPORT float get_current_cursor_y();
WASM_EXPORT void generate_selection_VBO(uint32_t start_idx, uint32_t end_idx, float scrolly);
#endif // RENDER_H