#ifndef PIECETABLE_H
#define PIECETABLE_H
#include <stdint.h>
#include <stddef.h>

#define WASM_EXPORT __attribute__((visibility("default"))) __attribute__((used))
// original Buffer READ ONLY
extern unsigned char __heap_base;

typedef enum
{
    SOURCE_ORIGINAL,
    SOURCE_ADD
} EditBuffer;
// piece structure
typedef struct
{
    EditBuffer source;
    uint32_t start;  // offset in the buffer
    uint32_t length; // length of the piece
} Piece;
typedef struct Node
{
    Piece piece;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
    int color;                     // 0 for blk, 1 for red
    uint32_t subtree_size;         // total length of pieces in subtree
    uint32_t newLineCount;         // number of new lines in this piece
    uint32_t subtree_newLineCount; // total new lines in subtree
    float node_height_px;          // height for rendering
    float subtree_height_px;       // total height in subtree
    // in_order pointers
    struct Node *prev;
    struct Node *next;
} Node;
typedef struct
{
    char *orignalBuffer;
    char *addBuffer;
    Node *root; // red-black tree root
} PieceTable;
static PieceTable pieceTable;
WASM_EXPORT int init_document(char *original_buffer, uint32_t original_length);
WASM_EXPORT uint32_t insert_text(const char *buffer, uint32_t length, uint32_t offset);
WASM_EXPORT char *getScratchBuffer();

#endif // PIECETABLE_H