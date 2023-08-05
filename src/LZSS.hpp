/*
 * LZSS
 */

#ifndef __LZSS_HPP
#define __LZSS_HPP

#include <array>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <utility>


/*
 * INDEX_BIT_COUNT : Size of address word indexing into text window
 */
constexpr const int INDEX_BIT_COUNT    = 12;    
/*
 * LENGTH_BIT_COUNT : Size of field used to encode length of a matching phrase.
 */
constexpr const int LENGTH_BIT_COUNT   = 4;
/*
 * WINDOW_SIZE : Size of the text window. This is determined by the size of the index 
 * word. In this case we can index up to (2^12)-1 words, so the window size becomes
 * 1 << 12 (4096).
 */
constexpr const int WINDOW_SIZE        = (1 << INDEX_BIT_COUNT);
constexpr const int RAW_LOOKAHEAD_SIZE = (1 << LENGTH_BIT_COUNT);
constexpr const int BREAK_EVEN         = ((1 + INDEX_BIT_COUNT + LENGTH_BIT_COUNT) / 9);
constexpr const int LOOK_AHEAD_SIZE    = (RAW_LOOKAHEAD_SIZE + BREAK_EVEN);
constexpr const int TREE_ROOT          = WINDOW_SIZE;
constexpr const int TREE_UNUSED        = 0;
constexpr const int END_OF_STREAM      = 0;



// Note this is almost certainly not portable
// Once it works consider finding a serialization lib
struct BitStream
{
    uint8_t wr_mask;
    uint8_t rd_mask;
    int wr_buf;
    int rd_buf;
    std::stringstream& ss; // TODO: would be nice if this could also be string_view...

    public:
        BitStream(std::stringstream& inp_stream) : 
            wr_mask(0x80), rd_mask(0x80), 
            wr_buf(0), rd_buf(0), ss(inp_stream) {}

        void     write_bit(uint8_t bit);
        void     write_bits(uint32_t word, int count);
        uint8_t  read_bit(void);
        uint32_t read_bits(int count);
        unsigned length(void);
        void     init(void);
        void     to_file(const std::string& filename);
};


// TODO: fold this into a single bitstream that takes various kinds of underlying buffers
struct VectorBitStream
{
    uint8_t wr_mask, rd_mask;
    int wr_buf, rd_buf;
    unsigned rd_ptr;
    std::vector<uint8_t> data;

    public:
        VectorBitStream() : wr_mask(0x80), rd_mask(0x80), wr_buf(0), rd_buf(0), rd_ptr(0) {}

        void     init(void);
        void     write_bit(uint8_t bit);
        void     write_bits(uint32_t word, int count);
        uint8_t  read_bit(void);
        uint32_t read_bits(int count);
        unsigned length(void) const;
};


/*
 * LSZZ Tree
 */

struct LZSNode
{
    int parent;
    int smaller;
    int larger;

    public:
        LZSNode() : parent(0), smaller(0), larger(0) {}
        LZSNode(int p, int s, int l) : parent(p), smaller(s), larger(l) {} 
};


using LZSSTree = std::array<LZSNode, WINDOW_SIZE+1>;
using LZSSWindow = std::array<char, WINDOW_SIZE>;


int  find_next_smallest_node(LZSSTree& tree, int node);
void contract_node(LZSSTree& tree, int old_node, int new_node);
void replace_node(LZSSTree& tree, int old_node, int new_node);
std::pair<int, int>  add_string(LZSSTree& tree, LZSSWindow& window, int new_node);
void delete_string(LZSSTree& tree, int p);
void init_tree(LZSSTree& tree, int r);


/*
 * Encoding functions
 */
std::stringstream lzss_encode(const std::string_view data);
std::stringstream lzss_decode(const std::string_view data);




#endif /*__LZSS_HPP*/
