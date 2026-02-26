#include "render.h"

static CMD memBuffer[MAX_CMD];
static Node *cached_start_node = NULL;
static float cached_start_y = 0.0f;
static int cmd_count = 0;
int is_whiteSpace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}
void update_config(float vp_x, float vp_y, float line_h, float font_sz)
{
    config.viewPort_x = vp_x;
    config.viewPort_y = vp_y;
    config.line_h = line_h;
    config.font_size = font_sz;
}

// update the layout for single node, including the x, y

float layout(Node *node, float *current_x, float base_y)
{
    if (!node)
        return 0.0f;
    char *text = get_piece_text(&pieceTable, node->piece);
    uint32_t total_length = node->piece.length;
    float curr_x = 0.0f;
    float curr_y = 0.0f;
    uint32_t segment_start = 0;
    for (uint32_t i = 0; i <= total_length; i++)
    {
        int is_nl = (i < total_length) && text[i] == '\n';
        int is_end = (i == total_length);
        int is_wrap = (curr_x + (i - segment_start) * config.font_size) >= config.viewPort_x;
        if (is_nl || is_end || is_wrap)
        {
            if (i > segment_start && cmd_count < MAX_CMD)
            {
                CMD *cmd = &memBuffer[cmd_count++];
                cmd->x = curr_x;
                cmd->y = base_y + curr_y;
                cmd->text_ptr = text + segment_start;
                cmd->length = i - segment_start;
            }
            if (is_nl || is_wrap)
            {
                curr_x = 0.0f;
                curr_y += config.line_h;
            }
            else
            {
                curr_x += (i - segment_start) * config.font_size;
            }
            segment_start = is_nl ? i + 1 : i;
        }
    }
    *current_x = curr_x;
    if (node->node_height_px != curr_y)
    {
        update_node_height(node, curr_y);
    }
    return curr_y;
}
void generate_frame(float scroll_y)
{
    uint32_t cmd = 0;
    Node *node = NULL;
    float relative_y = 0.0f;
    // find the first visible node
    find_visible_node(&pieceTable, scroll_y, scroll_y + config.viewPort_y, &node, &relative_y);
    if (!node)
        return;
    float curr_y = -relative_y;
    float curr_x = 0.0f;

    // walk through the nodes and generate CMDs
    while (node && cmd < MAX_CMD && curr_y <= config.viewPort_y)
    {
        float delta_y = layout_node(node);
        curr_y += delta_y;
        node = node->next;
    }
}
