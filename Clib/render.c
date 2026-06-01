#include "render.h"
#include "Glyph.h"
extern Glyph fonts[256];
Config config = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
Cursor cached_cursor = {0xFFFFFFFF, 0.0f, 0.0f};
int VBO_IDX = 0;
// check if a character is white space
int is_whiteSpace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}
// helper function for JS to update the config based on the real browser info
void update_config(float vp_x, float vp_y, float line_h, float font_sz, float padding_x, float padding_y)
{
    config.viewPort_x = vp_x;
    config.viewPort_y = vp_y;
    config.line_h = line_h;
    config.font_size = font_sz;
    config.padding_x = padding_x;
    config.padding_y = padding_y;
}
static Node *tree_first(Node *root)
{
    if (!root)
        return NULL;
    while (root->left)
        root = root->left;
    return root;
}
void generate_VBO_frame(float scroll_y)
{
    VBO_IDX = 0;
    float curr_x = config.padding_x;                            // start with padding
    float curr_y = config.padding_y + config.line_h - scroll_y; // the base line
    // starting from the first node in the tree
    Node *node = tree_first(pieceTable.root);
    if (!node)
        return;

    while (node && curr_y <= config.viewPort_y + config.line_h)
    {
        char *text = get_piece_text(&pieceTable, node->piece);
        uint32_t len = node->piece.length;
        for (uint32_t i = 0; i < len; i++)
        {
            char c = text[i];
            //  if this is the break already
            if (c == '\n')
            {
                curr_x = config.padding_x; // reset to the padding x
                curr_y += config.line_h;
                continue;
            }

            Glyph glyph = fonts[(unsigned char)c];
            if (c == '\t')
            {
                glyph = fonts[32];     // Steal the space glyph metrics
                glyph.advance *= 4.0f; // Make it 4 spaces wide
            }
            // if the wrap
            if (curr_x + glyph.advance * config.font_size > config.viewPort_x - config.padding_x)
            {
                // DEBUG:
                // console_log_string("Line wrap occurred");
                // console_log_int(curr_x + glyph.advance * config.font_size);
                // console_log_int(config.viewPort_x);
                curr_x = config.padding_x; // reset to the padding x
                curr_y += config.line_h;
            }
            // if the current is visible
            if (curr_y >= 0.0f && curr_y - config.line_h <= config.viewPort_y)
            {
                if (!is_whiteSpace(c) && VBO_IDX + 24 <= MAX_CHAR * 24)
                {
                    // Add the glyph to the VBO
                    float x0 = curr_x + glyph.plane_l * config.font_size;
                    float x1 = curr_x + glyph.plane_r * config.font_size;
                    float y0 = curr_y - (glyph.plane_t * config.font_size);
                    float y1 = curr_y - (glyph.plane_b * config.font_size);
                    float u0 = glyph.atlas_l;
                    float u1 = glyph.atlas_r;
                    float v0 = glyph.atlas_t;
                    float v1 = glyph.atlas_b;
                    // Triangle 1
                    // top left
                    VBO_buffer[VBO_IDX++] = x0;
                    VBO_buffer[VBO_IDX++] = y0;
                    VBO_buffer[VBO_IDX++] = u0;
                    VBO_buffer[VBO_IDX++] = v0;
                    // bottom left
                    VBO_buffer[VBO_IDX++] = x0;
                    VBO_buffer[VBO_IDX++] = y1;
                    VBO_buffer[VBO_IDX++] = u0;
                    VBO_buffer[VBO_IDX++] = v1;
                    // top right
                    VBO_buffer[VBO_IDX++] = x1;
                    VBO_buffer[VBO_IDX++] = y0;
                    VBO_buffer[VBO_IDX++] = u1;
                    VBO_buffer[VBO_IDX++] = v0;

                    // Triangle 2
                    // top right
                    VBO_buffer[VBO_IDX++] = x1;
                    VBO_buffer[VBO_IDX++] = y0;
                    VBO_buffer[VBO_IDX++] = u1;
                    VBO_buffer[VBO_IDX++] = v0;
                    // bottom left
                    VBO_buffer[VBO_IDX++] = x0;
                    VBO_buffer[VBO_IDX++] = y1;
                    VBO_buffer[VBO_IDX++] = u0;
                    VBO_buffer[VBO_IDX++] = v1;
                    // bottom right
                    VBO_buffer[VBO_IDX++] = x1;
                    VBO_buffer[VBO_IDX++] = y1;
                    VBO_buffer[VBO_IDX++] = u1;
                    VBO_buffer[VBO_IDX++] = v1;
                }
            }
            curr_x += glyph.advance * config.font_size;
        }
        node = node->next;
    }
}
void find_cursor_position(Node *root, float target_x, float target_y, float *out_x, float *out_y, uint32_t *out_idx)
{
    // This function will find the cursor position based on the current text layout
    float curr_x = config.padding_x;                 // start with padding
    float curr_y = config.padding_y + config.line_h; // the first base line
    uint32_t index = 0;                              // the logical position so that we can insert text
    if (target_y < curr_y - config.font_size)
    {
        *out_x = config.padding_x;
        *out_y = curr_y - config.font_size;
        *out_idx = 0;
        return;
    }
    while (root != NULL)
    {
        char *text = get_piece_text(&pieceTable, root->piece);
        uint32_t len = root->piece.length;
        for (uint32_t i = 0; i < len; i++)
        {
            float past_x = curr_x;
            char c = text[i];
            float ceiling = curr_y - config.font_size;               // the ceiling of the current line
            float floor = curr_y + config.line_h - config.font_size; // the floor of the current line
            if (c == '\n')
            {
                // jump to the last one before the new line
                if (target_y >= ceiling && target_y < floor)
                {
                    // console_log_int(1);
                    *out_x = curr_x;
                    *out_y = ceiling;
                    *out_idx = index;
                    return; // before jump to the new line
                }
                curr_x = config.padding_x; // reset to the padding x
                curr_y += config.line_h;
                index++;
                continue;
            }

            // consider the wrap
            Glyph glyph = fonts[(unsigned char)c];
            if (c == '\t')
            {
                glyph = fonts[32];     // Steal the space glyph metrics
                glyph.advance *= 4.0f; // Make it 4 spaces wide
            }
            if (curr_x + glyph.advance * config.font_size > config.viewPort_x - config.padding_x)
            {
                if (target_y >= ceiling && target_y < floor)
                {
                    *out_x = curr_x;
                    *out_y = ceiling;
                    *out_idx = index;
                    return; // before jump to the new line
                }
                curr_x = config.padding_x;
                curr_y += config.line_h;
                past_x = curr_x; // after wrap the x is 0, so update past_x as well
                ceiling = curr_y - config.font_size;
                floor = curr_y + config.line_h - config.font_size;
            }

            if (target_y >= ceiling && target_y < floor)
            {
                // we are in the target line, now check the x
                if (target_x < past_x + (glyph.advance * config.font_size) / 2)
                {
                    // the cursor should be before this character
                    *out_x = past_x;
                    *out_y = ceiling;
                    *out_idx = index;
                    return;
                }
            }
            curr_x += glyph.advance * config.font_size;
            index++;
        }
        root = root->next;
    }
    // for the end of document, empty space
    *out_x = curr_x;
    *out_y = curr_y - config.font_size;
    *out_idx = index;
}
uint32_t update_cursor_VBO(float cursor_x, float cursor_y)
{
    float x = 0.0f, y = 0.0f;
    uint32_t idx = 0;
    // two triangles to form a cursor rectangle
    float cursor_width = 3.5f;           // 2 pixels wide
    float cursor_height = config.line_h; // height of the line
    // get x and y based on the node
    find_cursor_position(tree_first(pieceTable.root), cursor_x, cursor_y, &x, &y, &idx);
    // update the cached cursor
    cached_cursor.idx = idx;
    cached_cursor.x = x;
    cached_cursor.y = y;
    // Triangle 1
    // top left
    CURSOR_VBO[0] = x;
    CURSOR_VBO[1] = y;
    // bottom left
    CURSOR_VBO[2] = x;
    CURSOR_VBO[3] = y + cursor_height;
    // top right
    CURSOR_VBO[4] = x + cursor_width;
    CURSOR_VBO[5] = y;

    // Triangle 2
    // top right
    CURSOR_VBO[6] = x + cursor_width;
    CURSOR_VBO[7] = y;
    // bottom left
    CURSOR_VBO[8] = x;
    CURSOR_VBO[9] = y + cursor_height;
    // bottom right
    CURSOR_VBO[10] = x + cursor_width;
    CURSOR_VBO[11] = y + cursor_height;
    return idx;
}
void update_cursor_index_VBO(uint32_t target_idx)
{
    float x = config.padding_x, y = config.padding_y + config.line_h;
    float cursor_width = 3.5f;           // 2 pixels wide
    float cursor_height = config.line_h; // height of the line
    // OPTIMIZATION: if the target_idx is just after the cached_cursor.idx, we can directly move the cursor without traversing the tree again
    if (cached_cursor.idx != 0xFFFFFFFF && target_idx == cached_cursor.idx + 1)
    {
        uint32_t rel_offset = 0;
        Node *node = find_node_by_offset(pieceTable.root, cached_cursor.idx, &rel_offset, NULL);
        if (node)
        {
            char *text = get_piece_text(&pieceTable, node->piece);
            char c = text[rel_offset];
            Glyph glyph = fonts[(unsigned char)c];
            if (c == '\n')
            {
                cached_cursor.x = config.padding_x; // reset to the padding x
                cached_cursor.y += config.line_h;
            }

            else
            {
                if (c == '\t')
                {
                    glyph = fonts[32];     // Steal the space glyph metrics
                    glyph.advance *= 4.0f; // Make it 4 spaces wide
                }
                if (cached_cursor.x + glyph.advance * config.font_size > config.viewPort_x - config.padding_x)
                {
                    cached_cursor.x = config.padding_x;
                    cached_cursor.y += config.line_h;
                }
                cached_cursor.x += glyph.advance * config.font_size;
            }
            cached_cursor.idx = target_idx;
            goto draw;
        }
    }

    // slow path: If it's the first time or the target_idx is not adjacent to the cached cursor, we need to find the position from the tree
    uint32_t idx = 0;
    Node *root = tree_first(pieceTable.root);
    while (root)
    {
        char *text = get_piece_text(&pieceTable, root->piece);
        uint32_t len = root->piece.length;
        for (uint32_t i = 0; i < len; i++)
        {
            if (idx == target_idx)
            {
                cached_cursor.idx = target_idx;
                cached_cursor.x = x;
                cached_cursor.y = y - config.font_size; // the cursor y should be at the ceiling of the line
                goto draw;
            }
            char c = text[i];
            Glyph glyph = fonts[(unsigned char)c];
            if (c == '\n')
            {
                x = config.padding_x; // reset to the padding x
                y += config.line_h;
                idx++;
                continue;
            }
            if (c == '\t')
            {
                glyph = fonts[32];     // Steal the space glyph metrics
                glyph.advance *= 4.0f; // Make it 4 spaces wide
            }
            if (x + glyph.advance * config.font_size > config.viewPort_x - config.padding_x)
            {
                x = config.padding_x;
                y += config.line_h;
            }
            x += glyph.advance * config.font_size;
            idx++;
        }
        root = root->next;
    }

    cached_cursor.idx = target_idx;
    cached_cursor.x = x;
    cached_cursor.y = y - config.font_size;

draw:
    // Triangle 1
    // top left
    CURSOR_VBO[0] = cached_cursor.x;
    CURSOR_VBO[1] = cached_cursor.y;
    // bottom left
    CURSOR_VBO[2] = cached_cursor.x;
    CURSOR_VBO[3] = cached_cursor.y + cursor_height;
    // top right
    CURSOR_VBO[4] = cached_cursor.x + cursor_width;
    CURSOR_VBO[5] = cached_cursor.y;

    // Triangle 2
    // top right
    CURSOR_VBO[6] = cached_cursor.x + cursor_width;
    CURSOR_VBO[7] = cached_cursor.y;
    // bottom left
    CURSOR_VBO[8] = cached_cursor.x;
    CURSOR_VBO[9] = cached_cursor.y + cursor_height;
    // bottom right
    CURSOR_VBO[10] = cached_cursor.x + cursor_width;
    CURSOR_VBO[11] = cached_cursor.y + cursor_height;
}
float *get_vbo_buffer()
{
    return VBO_buffer;
}
float *get_CURSOR_VBO()
{
    return CURSOR_VBO;
}
int get_vbo_count()
{
    return VBO_IDX;
}
float get_current_cursor_x()
{
    return cached_cursor.x;
}
float get_current_cursor_y()
{
    return cached_cursor.y;
}