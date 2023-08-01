/*
 * LZSS
 */


#include <algorithm>
#include <fstream>

#include "LZSS.hpp"




// For reference - these are the outputbit() and outputbits() routines from
// The Data Compression Book
static inline int mod_window(int a)
{
    return a & (WINDOW_SIZE - 1);
}



/*
 * Wrapper for bitstream
 */
void BitStream::write_bit(uint8_t bit)
{
    if(bit)
        this->wr_buf = (this->wr_buf | this->wr_mask);
    this->wr_mask = this->wr_mask >> 1;

    if(this->wr_mask == 0)
    {
        this->ss.write(reinterpret_cast<const char*>(&this->wr_buf), sizeof(uint8_t));
        this->wr_buf = 0;
        this->wr_mask = 0x80;
    }
}


/*
 * Write the lowest N bits of word
 */
void BitStream::write_bits(uint32_t word, int N)
{
    uint32_t mask;

    mask = 1L << (N - 1);
    while(mask != 0)
    {
        if(mask & word)
            this->wr_buf = (this->wr_buf | this->wr_mask);
        this->wr_mask = this->wr_mask >> 1;

        if(this->wr_mask == 0)
        {
            this->ss.write(reinterpret_cast<const char*>(&this->wr_buf), sizeof(uint8_t));
            this->wr_buf = 0;
            this->wr_mask = 0x80;
        }
        mask = mask >> 1;
    }
}


/*
 * Read one bit from the stream
 */
uint8_t BitStream::read_bit(void)
{
    int value;
    char c;

    if(this->rd_mask == 0x80)
    {
        this->ss.read(&c, sizeof(char));
        if(c == EOF)
            return 0;
        this->rd_buf = c;
    }

    //value = this->rd_buf & this->rd_mask;
    value = this->rd_buf & this->rd_mask;
    this->rd_mask = this->rd_mask >> 1;
    
    if(this->rd_mask == 0)
        this->rd_mask = 0x80;

    return value ? 1 : 0;
}


uint32_t BitStream::read_bits(int count)
{
    uint32_t mask, rv;
    char c;

    mask = 1L << (count - 1);
    rv = 0;    

    while(mask != 0)
    {
        if(this->rd_mask == 0x80)
        {
            this->ss.read(&c, sizeof(char));
            if(c == EOF)
                return 0;           // Would be better to signal (fatal) error. 
            this->rd_buf = c;
        }

        if(this->rd_buf & this->rd_mask)
            rv = rv | mask;
        mask = mask >> 1;
        this->rd_mask = this->rd_mask >> 1;

        if(this->rd_mask == 0)
            this->rd_mask = 0x80;
    }

    return rv;
}
    



unsigned BitStream::length(void)
{
    size_t old_pos = this->ss.tellg();
    this->ss.seekg(0, std::ios::end);
    unsigned end = this->ss.tellg();
    this->ss.seekg(old_pos);

    return end;
}


void BitStream::init(void)
{
    this->wr_mask = 0x80;
    this->rd_mask = 0x80;
    this->wr_buf = 0;
    this->rd_buf = 0;
    this->ss.clear();
    this->ss.str("");
}


void BitStream::to_file(const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary);
    file << this->ss.rdbuf();
    file.close();
    this->ss.seekg(0, std::ios::beg);       // why does rdbuf advance the get pointer?
}


int find_next_smallest_node(LZSSTree& tree, int node)
{
    int next = tree[node].smaller;
    while(tree[next].larger != TREE_UNUSED)
        next = tree[next].larger;

    return next;
}


void contract_node(LZSSTree& tree, int old_node, int new_node)
{
    tree[old_node].parent = tree[new_node].parent;
    if(tree[tree[old_node].parent].larger == old_node)
        tree[tree[old_node].parent].larger = new_node;
    else
        tree[tree[old_node].parent].smaller = new_node;
    tree[old_node].parent = TREE_UNUSED;
}


void replace_node(LZSSTree& tree, int old_node, int new_node)
{
    int parent = tree[old_node].parent;
    if(tree[parent].smaller == old_node)
        tree[parent].smaller = new_node;
    else
        tree[parent].larger = new_node;

    tree[new_node] = tree[old_node];
    tree[tree[new_node].smaller].parent = new_node;
    tree[tree[new_node].larger].parent = new_node;
    tree[old_node].parent = TREE_UNUSED;
}


/*
 * Binary tree deletion. If the node to be deleted is a null link in either direction
 * then just pull the non-null link up one level to replace the existing link.
 * If noth links exist delete the next link in order (which is guaranteed to have a null
 * link) then replace the node to be deleted with the next link.
 */
void delete_string(LZSSTree& tree, int p)
{
    int replacement;

    if(tree[p].parent == TREE_UNUSED)
        return;

    if(tree[p].larger == TREE_UNUSED)
        contract_node(tree, p, tree[p].smaller);
    else if(tree[p].smaller == TREE_UNUSED)
        contract_node(tree, p, tree[p].smaller);
    else
    {
        replacement = find_next_smallest_node(tree, p);
        delete_string(tree, replacement);
        replace_node(tree, p, replacement);
    }
}



// More from the book
// This function implements most of the logic of the LZSS encoder
int add_string(
        LZSSTree& tree,
        LZSSWindow& window,
        int new_node, 
        int* match_position
)
{
    int child; // I don't get why this is a pointer in the original implementation?
    int test_node;
    int match_length;
    // comp encodes comparison of new_node and test_node,
    // < 1 if new_node < test_node, 
    // 0 if new_node == test_node
    // > 1 if new_node > test_node
    int comp;          

    if(new_node == END_OF_STREAM)
        return 0;

    test_node = tree[TREE_ROOT].larger;
    match_length = 0;

    // TODO: doesn't terminate
    for(;;)
    {
        // Find the number of characters that match
        int i;
        for(i = 0; i < LOOK_AHEAD_SIZE; ++i)
        {
            comp = window[mod_window(new_node + i)] - window[mod_window(test_node + i)];
            if(comp != 0)
                break;
        }

        if(i >= match_length)
        {
            match_length = i;
            *match_position = test_node;

            if(match_length >= LOOK_AHEAD_SIZE)
            {
                replace_node(tree, test_node, new_node);
                return match_length;
            }
        }

        if(comp >= 0)
            child = tree[test_node].larger;
        else
            child = tree[test_node].smaller;

        if(child == TREE_UNUSED)
        {
            child = new_node;
            tree[new_node].parent = test_node;
            tree[new_node].larger = TREE_UNUSED;
            tree[new_node].smaller = TREE_UNUSED;

            return match_length;
        }
        test_node = child;
    }
}



void init_tree(LZSSTree& tree, int r)
{
    tree[TREE_ROOT].larger = r;
    tree[r].parent = TREE_ROOT;
    tree[r].larger = TREE_UNUSED;
    tree[r].smaller = TREE_UNUSED;
}





// LZSS encode 
// Stream consists of either SYMBOLS or REFERENCES
//
// SYMBOL: An actual symbol. Encoding is a single 1-bit followed by 8-bits of symbol 
// data. 
//
// REFERENCE: Instruction to repeat some sequence from the window. Encoding is a 
// single 0-bit followed by 12-bit index representing position, followed by 4-bit
// index representing length. Eg:
//
// 1 | pos | len 
std::stringstream lzss_encode(const std::string_view data)
{
    std::stringstream ss;
    BitStream out_stream(ss);

    LZSSTree tree;
    LZSSWindow window;
    int look_ahead_bytes;
    int match_length = 0;
    int match_pos = 0;
    int replace_count = 0;


    unsigned cur_pos = 1;
    unsigned inp_pos = 0;
    unsigned N = std::min(LOOK_AHEAD_SIZE, int(data.size()));
    for(inp_pos = 0; inp_pos < N; ++inp_pos)
        window[cur_pos + inp_pos] = data[inp_pos];

    look_ahead_bytes = 1;
    init_tree(tree, cur_pos);

    char c;
    while(look_ahead_bytes > 0)
    {
        if(match_length > look_ahead_bytes)
            match_length = look_ahead_bytes;

        // Encode a single character
        if(match_length <= BREAK_EVEN)
        {
            replace_count = 1;
            out_stream.write_bit(1);
            out_stream.write_bits(window[cur_pos], 8);
        }
        // Encode a reference
        else
        {
            out_stream.write_bit(0);
            out_stream.write_bits(match_pos, INDEX_BIT_COUNT);
            out_stream.write_bits(match_length - (BREAK_EVEN+1), LENGTH_BIT_COUNT);
            replace_count = match_length;
        }

        for(int i = 0; i < replace_count; ++i)
        {
            delete_string(tree, mod_window(cur_pos + LOOK_AHEAD_SIZE));
            // if end of string, look_ahead_bytes--
            c = data[inp_pos];          // TODO: how to implement this condition?
            if(inp_pos >= unsigned(data.size()-1))
                look_ahead_bytes--;
            else
                window[mod_window(cur_pos + LOOK_AHEAD_SIZE)] = c;
            
            inp_pos++;
            cur_pos = mod_window(cur_pos+1);        // basically window position

            if(look_ahead_bytes)
                match_length = add_string(tree, window, cur_pos, &match_pos);
        }
    }

    out_stream.write_bit(0);
    out_stream.write_bits(END_OF_STREAM, INDEX_BIT_COUNT);

    return ss;
}



/*
 * Decode
 */
std::stringstream lzss_decode(const std::string_view data)
{
    int inp_pos = 0;
    int cur_pos = 1;
    int match_pos;
    int match_length;

    cur_pos = 1;

    char c;
    for(;;)
    {
        c = data[inp_pos];
        inp_pos++;

    }
}
