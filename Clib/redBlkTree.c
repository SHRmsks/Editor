
#include "redBlkTree.h"
// create a new node for blk-red tree
Node *create_Node(Piece piece, uint32_t newlineCount)
{
    if (node_count >= MAX_NODES)
    {
        return NULL;
    }
    Node *n = &node_pool[node_count++];
    n->piece = piece;
    n->left = NULL;
    n->right = NULL;
    n->parent = NULL;
    n->color = 1; // new node is red
    n->subtree_size = piece.length;
    n->newLineCount = newlineCount;
    n->subtree_newLineCount = newlineCount;
    float est_height = (newlineCount == 0 ? 1 : newlineCount) * 20.0f; // guessed height
    n->subtree_height_px = est_height;
    n->node_height_px = est_height;
    return n;
}
uint32_t get_size(Node *n)
{
    return n ? n->subtree_size : 0;
}
// update a specific node's subtree size
void update_tree_size(Node *n)
{
    if (n)
    {
        n->subtree_size = get_size(n->left) + get_size(n->right) + n->piece.length;
        // update the newline counts
        uint32_t left_nl = n->left ? n->left->subtree_newLineCount : 0;
        uint32_t right_nl = n->right ? n->right->subtree_newLineCount : 0;
        n->subtree_newLineCount = left_nl + right_nl +
                                  (n->newLineCount);
        // update the heights
        float left_height = n->left ? n->left->subtree_height_px : 0.0f;
        float right_height = n->right ? n->right->subtree_height_px : 0.0f;
        n->subtree_height_px = left_height + right_height + n->node_height_px;
    }
}
// walk up from the node
void update_size_upwards(Node *n, int32_t size, int32_t newlineCounts, float height_changes)
{
    Node *curr = n;
    while (curr)
    {
        curr->subtree_size += size;
        curr->subtree_newLineCount += newlineCounts;
        curr->subtree_height_px += height_changes;
        curr = curr->parent;
    }
}
void rotate_left(PieceTable *pt, Node *x)
{
    Node *y = x->right;
    x->right = y->left;
    if (y->left != NULL)
    {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL)
    {
        pt->root = y;
    }
    else if (x == x->parent->left)
    {
        x->parent->left = y;
    }
    else
    {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
    update_tree_size(x);
    update_tree_size(y);
}
void rotate_right(PieceTable *pt, Node *x)
{
    Node *y = x->left;
    x->left = y->right;
    if (y->right != NULL)
    {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL)
    {
        pt->root = y;
    }
    else if (x == x->parent->right)
    {
        x->parent->right = y;
    }
    else
    {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
    update_tree_size(x);
    update_tree_size(y);
}
void fix_insert(PieceTable *pt, Node *x)
{
    // if the parent is Red, potential violation
    while (x->parent != NULL && x->parent->color == 1)
    {
        // if Parent is Left child
        if (x->parent == x->parent->parent->left)
        {
            // find Uncle
            Node *y = x->parent->parent->right;
            // if Uncle exists and is Red
            if (y != NULL && y->color == 1)
            {
                // paint the parent and uncle blk
                x->parent->color = 0;
                y->color = 0;
                // paint the grandparent red
                x->parent->parent->color = 1;
                x = x->parent->parent; // move x up for further check
            }
            else
            {
                // if x is right child, we perform left rotation
                if (x == x->parent->right)
                {
                    x = x->parent;
                    rotate_left(pt, x);
                }
                x->parent->color = 0;
                x->parent->parent->color = 1;
                rotate_right(pt, x->parent->parent);
            }
        }
        else
        {
            Node *y = x->parent->parent->left;
            if (y != NULL && y->color == 1)
            {
                x->parent->color = 0;
                y->color = 0;
                x->parent->parent->color = 1;
                x = x->parent->parent;
            }
            else
            {
                if (x == x->parent->left)
                {
                    x = x->parent;
                    rotate_right(pt, x);
                }
                x->parent->color = 0;
                x->parent->parent->color = 1;
                rotate_left(pt, x->parent->parent);
            }
        }
    }
    pt->root->color = 0; // root is always blk
}
// helper function to get the text of a piece
char *get_piece_text(PieceTable *pt, Piece piece)
{
    char *ptr = piece.source == SOURCE_ORIGINAL ? pt->orignalBuffer : pt->addBuffer;
    return ptr + piece.start;
}
// helper function to get newlines in a piece
uint32_t get_piece_newlines(PieceTable *pt, Piece piece)
{
    char *text = get_piece_text(pt, piece);
    uint32_t count = 0;
    for (uint32_t i = 0; i < piece.length; i++)
    {
        if (text[i] == '\n')
        {
            count++;
        }
    }
    return count;
}
// helper function to locate the node by offset, and return the Node *
Node *find_node_by_offset(Node *root, uint32_t offset, uint32_t *relative_offset, Node **lastNode)
{
    Node *curr = root;
    while (curr)
    {
        uint32_t left_size = get_size(curr->left);
        if (offset < left_size)
        {
            curr = curr->left;
        }
        else if (offset < left_size + curr->piece.length)
        {
            *relative_offset = offset - left_size;
            return curr;
        }
        else
        {
            offset -= left_size + curr->piece.length;
            curr = curr->right;
            if (lastNode)
                *lastNode = curr;
        }
    }
    return NULL;
}
// helper function to append a new node at given offset
void rb_insert_after(PieceTable *pt, Node *parent, Node *newNode)
{
    if (parent == NULL && pt->root == NULL)
    {
        pt->root = newNode;
        newNode->color = 0; // root is blk
        return;
    }
    if (parent->right == NULL)
    {
        parent->right = newNode;
        newNode->parent = parent;
    }
    else
    {
        Node *next = parent->right;
        while (next->left != NULL)
        {
            next = next->left;
        }
        next->left = newNode;
        newNode->parent = next;
    }
    update_size_upwards(newNode->parent, newNode->piece.length, newNode->newLineCount, newNode->node_height_px);
    fix_insert(pt, newNode);
}
// helper function prepend a new node at given offset
void rb_insert_front(PieceTable *pt, Node *newNode)
{
    if (pt->root == NULL)
    {
        pt->root = newNode;
        newNode->color = 0; // root is blk
        return;
    }
    Node *curr = pt->root;
    while (curr->left != NULL)
    {
        curr = curr->left;
    }
    curr->left = newNode;
    newNode->parent = curr;
    update_size_upwards(newNode->parent, newNode->piece.length, newNode->newLineCount, newNode->node_height_px);
    fix_insert(pt, newNode);
}
// insert a new piece at given offset
void insert_node(PieceTable *pt, Piece newPiece, uint32_t offset)
{
    if (pt->root == NULL)
    {
        uint32_t newlinecounts = get_piece_newlines(pt, newPiece);
        pt->root = create_Node(newPiece, newlinecounts);
        pt->root->color = 0;
        pt->root->prev = NULL;
        pt->root->next = NULL;
        return;
    }
    uint32_t rel_offset = 0;
    Node *lastNode = NULL;
    Node *y = find_node_by_offset(pt->root, offset, &rel_offset, &lastNode);
    if (y == NULL)
    {
        uint32_t newlinecounts = get_piece_newlines(pt, newPiece);
        Node *newNode = create_Node(newPiece, newlinecounts);
        rb_insert_after(pt, &lastNode, newNode);
        newNode->prev = lastNode;
        if (lastNode)
            lastNode->next = newNode;
        return;
    }
    else
    {
        if (rel_offset == 0)
        {
            if (offset == 0)
            {
                uint32_t newlinecounts = get_piece_newlines(pt, newPiece);
                Node *newNode = create_Node(newPiece, newlinecounts);
                rb_insert_front(pt, newNode);
                newNode->prev = NULL;
                newNode->next = pt->root;
                pt->root->prev = newNode;
                return;
            }
            uint32_t rel_tempoffset = 0;
            Node *temp_finalPrev = NULL;
            uint32_t newlinecounts = get_piece_newlines(pt, newPiece);
            Node *preNode = find_node_by_offset(pt->root, offset - 1, &rel_tempoffset, temp_finalPrev);
            Node *newNode = create_Node(newPiece, newlinecounts);
            rb_insert_after(pt, preNode, newNode);
            newNode->prev = preNode;
            newNode->next = preNode->next;
            if (preNode->next)
            {
                preNode->next->prev = newNode;
            }
            preNode->next = newNode;
            return;
        }
        else
        {
            float old_h = y->node_height_px;
            float ratio = y->piece.length > 0 ? 0.0f : (float)rel_offset / (float)(y->piece.length);
            float new_h = old_h * ratio;
            float right = old_h - new_h;
            y->node_height_px = new_h;
            // we need to split the node y, update the newlines
            uint32_t newlinecounts_insert = get_piece_newlines(pt, newPiece);
            Piece leftPiece = y->piece;
            leftPiece.length = rel_offset;
            uint32_t newlines_left = get_piece_newlines(pt, leftPiece);
            Piece rightPiece = y->piece;
            rightPiece.start += rel_offset;
            rightPiece.length -= rel_offset;
            // update the existing node y to be the left piece
            y->piece.length = rel_offset;
            y->newLineCount = newlines_left;

            uint32_t newlines_right = get_piece_newlines(pt, rightPiece);
            Node *rightNode = create_Node(rightPiece, newlines_right);

            rightNode->node_height_px = right;
            rightNode->subtree_height_px = right;

            Node *newNode = create_Node(newPiece, newlinecounts_insert);
            update_size_upwards(y, -(int32_t)rightPiece.length, -(int32_t)newlines_right, -right);
            rb_insert_after(pt, y, newNode);
            rb_insert_after(pt, newNode, rightNode);
            // update the in-order order
            // Left(y) -> NewNode -> right
            newNode->next = rightNode;
            newNode->prev = y;
            // update the rightnode
            rightNode->prev = newNode;
            if (y->next)
            {
                rightNode->next = y->next;
            }

            // update y's next
            if (y->next)
            {
                y->next->prev = rightNode;
            }
            y->next = newNode;
            return;
        }
    }
}
//  update the actual measured height of node
void update_node_height(Node *n, float new_height)
{
    float diff = new_height - n->node_height_px;
    n->node_height_px = new_height;
    update_size_upwards(n, 0, 0, diff);
    n->subtree_height_px += diff;
}
// find the first that is visible in the viewport
void find_visible_node(PieceTable *pt, float start_y, float end_y, Node **firstNode, float *relative_y)
{
    Node *curr = pt->root;
    while (curr)
    {
        float left_height = curr->left ? curr->left->subtree_height_px : 0.0f;
        if (start_y < left_height)
        {
            curr = curr->left;
        }
        else if (start_y < left_height + curr->node_height_px)
        {
            *firstNode = curr;
            *relative_y = start_y - left_height;
            return;
        }
        else
        {
            start_y -= left_height + curr->node_height_px;
            curr = curr->right;
        }
    }
    *firstNode = NULL;
    *relative_y = 0.0f;
    return;
}
