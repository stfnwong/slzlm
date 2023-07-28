/*
 * LZW
 * C implementation of LZW algorithm
 *
 */


#include <stdint.h>

#include "lzw.h"


typedef struct Node Node;



struct Node
{
    uint32_t value;
    Node* children[LZW_ALPHA_SIZE];
    bool leaf;
};


Node* create_node(void)
{
    Node* node = calloc(1, sizeof(Node));
    // init alphabet
    for(unsigned i = 0; i < LZW_ALPHA_SIZE; ++i)
        node->children[i] = NULL;
    node->leaf = false;
    
    return node;
}


void destroy_node(Node* node)
{
    for(unsigned i = 0; i < LZW_ALPHA_SIZE; ++i)
    {
        if(node->children[i])
            destroy_node(node->children[i]);
    }

    free(node);
}


/*
 * search()
 *
 * Find if a character is a valid child of a node. Given a pointer to a node and 
 * a character, return a pointer to the child node containing that character if it
 * exists, or a null pointer otherwise.
 */
Node* search(const Node* node, const char c)
{
    if(node->children[c])
        return node->children[c];

    return NULL;
}

/*
 * insert()
 * Add the character c to a given node
 */
void insert(Node* node, const char c, uint32_t value)
{
    unsigned idx = (unsigned) c;
    if(node->children[idx])
        return;

    node->children[idx] = create_node();
    node->children[idx]->leaf = true;
    node->children[idx]->value = value;
}




ssize_t lzw_encode(const uint8_t* input, size_t input_len, uint8_t* output)
{
    int bytes_per_code = 2;
    int offset24 = 0;
    int offset32 = 0;

    // Allocate output memory - for this version we just allocate the same amount as 
    // in the input and then return how much of this is valid. Ideally we would have
    // some kind of container that would grow as we create more entries, so that we 
    // didn't waste possibly a large amount of memory (if the compression ration was 
    // actually good) but at least this scheme is always bounded.

    output = malloc(input_len * sizeof(uint8_t));
    if(!output)         // system ded
        return -1;

    ssize_t out_pos = 0;
    // Write header data here...
    
    out_pos += 12;

    Node* root = create_node();
    if(!root)
        return -1;

    uint32_t cur_key = LZW_ALPHA_SIZE;
    Node* node = root;

    for(size_t c = 0; c < input_len; ++c)
    {
        Node* child_node = search(node, c);
        if(!child_node)
        {
            // write the bytes out
            out_pos += bytes_per_code;
            insert(node, c, cur_key);
            cur_key++;

            if(cur_key == 0xFFFF)
            {
                bytes_per_code = 3;
                offset24 = c;
            }
            if(cur_key == 0xFFFFFF)
            {
                bytes_per_code = 4;
                offset32 = c;
            }
        }
        else
            node = child_node;
    }

    // write final value to output stream

    return out_pos;
}
