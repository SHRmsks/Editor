
#include "redBlkTree.h"
// create a new node for blk-red tree
Node *create_Node(Piece piece, uint32_t newlineCount)
{
    console_log_int(node_count);
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
    return n;
}
// safe way to get the size of a node's subtree
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
        // safe way to get the newline count from the left and right subtrees
        uint32_t left_nl = n->left ? n->left->subtree_newLineCount : 0;
        uint32_t right_nl = n->right ? n->right->subtree_newLineCount : 0;
        n->subtree_newLineCount = left_nl + right_nl +
                                  (n->newLineCount);
    }
}
// walk up from the node
void update_size_upwards(Node *n, int32_t size, int32_t newlineCounts)
{
    Node *curr = n;
    while (curr)
    {
        curr->subtree_size += size;
        curr->subtree_newLineCount += newlineCounts;
        curr = curr->parent;
    }
}
// RB tree rotation left
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
// helper function to get the text of a piece O(1)
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
// helper function to locate the node by offset, and return the Node * O(logN)
Node *find_node_by_offset(Node *root, uint32_t offset, uint32_t *relative_offset, Node **lastNode)
{
    Node *curr = root;
    Node *parent = NULL; // track the parent
    while (curr)
    {
        parent = curr; // update the parent before going down
        uint32_t left_size = get_size(curr->left);
        if (offset < left_size)
        {
            curr = curr->left;
        }
        else if (offset < left_size + curr->piece.length)
        {
            *relative_offset = offset - left_size;
            if (lastNode)
            {
                *lastNode = parent;
            }
            return curr;
        }
        else
        {
            offset -= left_size + curr->piece.length;
            curr = curr->right;
        }
    }
    if (lastNode)
    {
        *lastNode = parent;
    }
    return NULL; // that means the offset is beyond the total size, which is the end of the document
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
    update_size_upwards(newNode->parent, newNode->piece.length, newNode->newLineCount);
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
    update_size_upwards(newNode->parent, newNode->piece.length, newNode->newLineCount);
    fix_insert(pt, newNode);
}
// insert a new piece at given offset, the global offset
void insert_node(PieceTable *pt, Piece newPiece, uint32_t offset)
{
    // console_log_int(offset);
    if (pt->root == NULL)
    {
        uint32_t newlinecounts = get_piece_newlines(pt, newPiece);
        pt->root = create_Node(newPiece, newlinecounts);
        pt->root->color = 0;
        pt->root->prev = NULL;
        pt->root->next = NULL;
        return;
    }
    uint32_t rel_offset = 0; // the relative offset within the node
    Node *lastNode = NULL;
    Node *y = find_node_by_offset(pt->root, offset, &rel_offset, &lastNode);
    if (y == NULL) // the last leaf node
    {
        uint32_t newlinecounts = get_piece_newlines(pt, newPiece);
        Node *newNode = create_Node(newPiece, newlinecounts);
        rb_insert_after(pt, lastNode, newNode); // rb insert after the last node
        newNode->prev = lastNode;
        if (lastNode)
            lastNode->next = newNode;
        return;
    }
    else
    {
        if (rel_offset == 0) // that means it's landed the exact start position of the node
        {
            if (offset == 0) // if this is the absolute start of the document
            {
                uint32_t newlinecounts = get_piece_newlines(pt, newPiece);
                Node *newNode = create_Node(newPiece, newlinecounts);
                rb_insert_front(pt, newNode);
                newNode->prev = NULL;
                newNode->next = y; // the new node becoems the first node
                y->prev = newNode;
                return;
            }
            uint32_t rel_tempoffset = 0;
            Node *temp_finalPrev = NULL;
            uint32_t newlinecounts = get_piece_newlines(pt, newPiece);
            Node *preNode = find_node_by_offset(pt->root, offset - 1, &rel_tempoffset, &temp_finalPrev); // since this is the relative first offset, we need to find the node before it
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
            Node *newNode = create_Node(newPiece, newlinecounts_insert);
            update_size_upwards(y, -(int32_t)rightPiece.length, -(int32_t)newlines_right);
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
// visual logic model: find the which node contains the specified line
Node *find_node_by_line(Node *root, uint32_t target_line, uint32_t *relative_line)
{
    Node *current = root;
    while (current)
    {
        uint32_t left_lines = current->left ? current->left->newLineCount : 0;
        // if in the left subtree, go search
        if (target_line < left_lines)
        {
            current = current->left;
        }
        else if (target_line <= left_lines + current->newLineCount)
        {
            *relative_line = target_line - left_lines;
            return current;
        }
        else
        {
            target_line -= left_lines + current->newLineCount;
            current = current->right;
        }
    }
    return NULL;
}