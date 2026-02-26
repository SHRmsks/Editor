#include "redBlkTree.h"

static int node_count = 0;
static char *add_buffer_ptr = NULL;
char ScratchBuffer[4096]; // 4kb stack for js
int init_document(char *original_buffer, uint32_t original_length)
{
    pieceTable.orignalBuffer = original_buffer;
    pieceTable.addBuffer = (char *)&__heap_base;
    node_count = 0;
    add_buffer_ptr = pieceTable.addBuffer;
    Piece piece = {
        .source = SOURCE_ORIGINAL,
        .start = 0,
        .length = (uint32_t)original_length,
    };
    pieceTable.root = create_Node(piece, 0);
    return 0;
}
// insert text helper function
uint32_t insert_text(const char *buffer, uint32_t length, uint32_t offset)
{
    uint32_t buffer_offset = (uint32_t)(add_buffer_ptr - pieceTable.addBuffer);
    // copy text to add buffer
    memcpy(add_buffer_ptr, buffer, length);
    add_buffer_ptr += length;
    // create new piece
    Piece newPiece = {
        .source = SOURCE_ADD,
        .start = buffer_offset,
        .length = length,
    };
    // insert new piece into red-black tree
    insert_node(&pieceTable, newPiece, offset);
    return length;
}
char *getScratchBuffer()
{
    return ScratchBuffer;
}
