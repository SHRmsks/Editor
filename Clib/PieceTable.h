#ifndef PIECETABLE_H
#define PIECETABLE_H
#include "Cutils.h"
// expose signature macro
#define WASM_EXPORT __attribute__((visibility("default"))) __attribute__((used)) // export every function in wasm
// original Buffer READ ONLY, THIS IS THE ADDRESS INJECTED TO BY WASM LD
extern unsigned char __heap_base;
// the two buffer types, used for the piece table
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
// the Piece (Red Blac Tree Node)
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
    // in_order pointers
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct
{
    char *orignalBuffer; // the readonly buffer
    char *addBuffer;     // session buffer
    Node *root;          // red-black tree root
} PieceTable;

extern PieceTable pieceTable;
WASM_EXPORT int init_document(char *original_buffer, uint32_t original_length);
WASM_EXPORT uint32_t insert_text(const char *buffer, uint32_t length, uint32_t offset);
WASM_EXPORT uint32_t delete_text(uint32_t offset, uint32_t length);
WASM_EXPORT char *getScratchBuffer();
#endif // PIECETABLE_H