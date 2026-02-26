#ifndef REDBLKTREE_C
#define REDBLKTREE_C
#include "PieceTable.h"
extern int node_count;
#define MAX_NODES 100000
static Node node_pool[MAX_NODES];
Node *create_Node(Piece piece, uint32_t newlineCount);
uint32_t get_size(Node *n);
void update_tree_size(Node *n);
void update_size_upwards(Node *n, int32_t size, int32_t newlineCounts, float height_changes);
char *get_piece_text(PieceTable *pt, Piece piece);
void rotate_left(PieceTable *pt, Node *x);
void rotate_right(PieceTable *pt, Node *x);
void fix_insert(PieceTable *pt, Node *x);
Node *find_node_by_offset(Node *root, uint32_t offset, uint32_t *relative_offset, Node **lastNode);
void rb_insert_after(PieceTable *pt, Node *parent, Node *newNode);
void rb_insert_front(PieceTable *pt, Node *newNode);
void insert_node(PieceTable *pt, Piece newPiece, uint32_t offset);
void find_visible_node(PieceTable *pt, float start_y, float end_y, Node **firstNode, float *relative_y);
void update_node_height(Node *n, float new_height);
#endif // REDBLKTREE_C