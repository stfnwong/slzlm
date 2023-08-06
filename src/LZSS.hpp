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



template <class StreamImpl> struct BitStream
{
    uint8_t wr_mask;
    uint8_t rd_mask;
    int wr_buf;
    int rd_buf;

    private:
        BitStream() : wr_mask(0x80), rd_mask(0x80), wr_buf(0), rd_buf(0) {}
        friend StreamImpl;

    public:

        uint8_t read_bit(void)
        {
            uint8_t value, c;
            
            if(this->rd_mask == 0x80)
                this->rd_buf = static_cast<StreamImpl*>(this)->read();

            value = this->rd_buf & this->rd_mask;
            this->rd_mask = this->rd_mask >> 1;

            if(this->rd_mask == 0)
                this->rd_mask = 0x80;

            return value ? 1 : 0;
        }

        uint32_t read_bits(int count)
        {
            uint32_t mask, rv;

            mask = 1L << (count - 1);
            rv = 0;

            while(mask != 0)
            {
                if(this->rd_mask == 0x80)
                    this->rd_buf = static_cast<StreamImpl*>(this)->read();

                if(this->rd_buf & this->rd_mask)
                    rv = rv | mask;
                mask = mask >> 1;
                this->rd_mask = this->rd_mask >> 1;

                if(this->rd_mask == 0)
                    this->rd_mask = 0x80;
            }

            return rv;
        }

        void write_bit(uint8_t bit)
        {
            if(bit)
                this->wr_buf = (this->wr_buf | this->wr_mask);

            this->wr_mask = this->wr_mask >> 1;

            if(this->wr_mask == 0)
            {
                static_cast<StreamImpl*>(this)->write(this->wr_buf);
                this->wr_buf = 0;
                this->wr_mask = 0x80;
            }
        }

        void write_bits(uint32_t word, int count)
        {
            uint32_t mask;

            mask = 1L << (count - 1);

            while(mask != 0)
            {
                if(mask & word)
                    this->wr_buf = (this->wr_buf | this->wr_mask);
                this->wr_mask = this->wr_mask >> 1;

                if(this->wr_mask == 0)
                {
                    static_cast<StreamImpl*>(this)->write(this->wr_buf);
                    this->wr_buf = 0;
                    this->wr_mask = 0x80;
                }

                mask = mask >> 1;
            }
        }
};



struct VectorBitStream : BitStream<VectorBitStream>
{
    unsigned rd_ptr;
    std::vector<uint8_t> data;

    public:
        VectorBitStream() : rd_ptr(0) {} 

        uint8_t read(void) {
            this->rd_ptr++;
            return this->data[this->rd_ptr-1];
        }

        void write(uint8_t word) {
            this->data.push_back(word);
        }

        void init(void)
        {
            this->wr_mask = 0x80;
            this->rd_mask = 0x80;
            this->wr_buf = 0;
            this->rd_buf = 0;
            this->data.clear();
        }

        unsigned length(void) const {
            return this->data.size();
        }
};


struct StringBitStream : BitStream<StringBitStream>
{
    std::stringstream& ss;

    public:
        StringBitStream(std::stringstream& inp_stream) : ss(inp_stream) {} 

        uint8_t read(void) {
            char c;
            this->ss.read(&c, sizeof(uint8_t));
            return uint8_t(c);
        }

        void write(uint8_t word) { 
            this->ss.write(reinterpret_cast<const char*>(&word), sizeof(uint8_t));
        }

        void init(void)
        {
            this->wr_mask = 0x80;
            this->rd_mask = 0x80;
            this->wr_buf = 0;
            this->rd_buf = 0;
            this->ss.clear();
            this->ss.str("");
        }

        unsigned length(void) {
            return (unsigned) this->ss.tellp();
        }

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
