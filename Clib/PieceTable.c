#include "redBlkTree.h"

int node_count = 0;
static char *add_buffer_ptr = NULL;
char ScratchBuffer[4096]; // 4kb stack for js, this would be our IPC shared memory for copy operations
int init_document(char *original_buffer, uint32_t original_length)
{
    pieceTable.orignalBuffer = original_buffer; // the readonlhy buffer, persistant from the previous saved session
    // The entire heap is used as the add buffer
    pieceTable.addBuffer = (char *)&__heap_base;
    node_count = 0;                        // set the initialized node count to 0
    add_buffer_ptr = pieceTable.addBuffer; // track the end of the log
    pieceTable.root = NULL;
    Piece piece = {
        .source = SOURCE_ORIGINAL,
        .start = 0,
        .length = (uint32_t)original_length,
    };
    insert_node(&pieceTable, piece, 0);
    return 0;
}
// insert text helper function
uint32_t insert_text(const char *buffer, uint32_t length, uint32_t offset)
{
    uint32_t buffer_offset = (uint32_t)(add_buffer_ptr - pieceTable.addBuffer);
    // copy text to addbuffer
    c_memcpy(add_buffer_ptr, buffer, length);
    add_buffer_ptr += length;
    // STEP1: if the offset is at the end of the document, no need to split the node, just append a new node
    uint32_t rel_offset = 0;
    Node *targetNode = find_node_by_offset(pieceTable.root, offset > 0 ? offset - 1 : 0, &rel_offset, NULL);
    if (targetNode != NULL && targetNode->piece.source == SOURCE_ADD && rel_offset == targetNode->piece.length - 1 && targetNode->piece.start + targetNode->piece.length == buffer_offset) // that means we can just append to the current node
    {
        // DEBUG:
        // console_log_int(111);
        uint32_t newlines = 0;
        for (uint32_t i = 0; i < length; i++)
        {
            if (buffer[i] == '\n')
            {
                newlines++;
            }
        }
        targetNode->piece.length += length;
        targetNode->newLineCount += newlines;
        update_size_upwards(targetNode, (int32_t)length, (int32_t)newlines);
        return length;
    }
    // We have to insert a new Node for spliting
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
// delete text
uint32_t delete_text(uint32_t offset, uint32_t length)
{
    // PHASE 1: if the delete offset is the end of the node piece  that means we don need to split the node and we just mark the length-n as O(LogN) delete,
    if (pieceTable.root == NULL || length == 0)
    {
        return 0; // nothing to delete
    }
    uint32_t rel_offset = 0;
    Node *lastNode = NULL;
    Node *targetNode = find_node_by_offset(pieceTable.root, offset, &rel_offset, &lastNode);
    if (targetNode == NULL)
    {
        return 0; // offset is out of bounds
    }
    if (rel_offset + length == targetNode->piece.length)
    {
        char *text = get_piece_text(&pieceTable, targetNode->piece);
        // check if the deleted text contains new line for the line count update
        uint32_t newlineCount = 0;
        for (uint32_t i = rel_offset; i < rel_offset + length; i++)
        {
            if (text[i] == '\n')
            {
                newlineCount++;
            }
        }
        targetNode->piece.length -= length;
        targetNode->newLineCount -= newlineCount;
        update_size_upwards(targetNode, -(int32_t)length, -(int32_t)newlineCount);
        return length;
    }
    // Have To SPLIT that means we delete in the middle of something, we need to split the node into two pieces and mark the middle piece as deleted
    // PHASE 2: IF if the middle, we calculate the right piece node
    // clone a new piece
    Piece right_piece = targetNode->piece;
    right_piece.start += rel_offset + length;
    right_piece.length -= (rel_offset + length);
    uint32_t right_nl = get_piece_newlines(&pieceTable, right_piece);
    Node *rightNode = create_Node(right_piece, right_nl);

    // update the target node to be the left piece
    Piece left_piece = targetNode->piece;
    left_piece.length = rel_offset;
    uint32_t left_nl = get_piece_newlines(&pieceTable, left_piece);
    targetNode->piece.length = rel_offset;
    targetNode->newLineCount = left_nl;
    // deleted piece new line count
    uint32_t deletetd_nl = targetNode->newLineCount - left_nl - right_nl;
    // insert the right node
    update_size_upwards(targetNode, -(int32_t)(length + right_piece.length), -(int32_t)(deletetd_nl + right_nl));
    rb_insert_after(&pieceTable, targetNode, rightNode);
    // update the in-order linked list
    rightNode->next = targetNode->next;
    if (targetNode->next)
    {
        targetNode->next->prev = rightNode;
    }
    targetNode->next = rightNode;
    rightNode->prev = targetNode;
    return length;
}
char *getScratchBuffer()
{
    return ScratchBuffer;
}
