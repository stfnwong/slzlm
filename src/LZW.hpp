/*
 * COMP
 * Compressors
 *
 */

#ifndef __LZW_HPP
#define __LZW_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>




const constexpr uint32_t LZW_ALPHA_SIZE = 256;
using lzw_symbol_t = uint16_t;


// TODO: re-write as functions

// Encode function
std::stringstream lzw_encode(const std::string_view data);
std::stringstream lzw_decode(std::stringstream& data);


/*
 * Encoder
 * Object oriented encoder using LZW algorithm. The idea is that you can encode 
 * data larger than "memory" by breaking it into chunks and calling encode() in a loop.
 */
class LZWEncoder
{
    // Prefix tree 
    struct LZWNode
    {
        uint32_t value;
        bool leaf;
        std::map<lzw_symbol_t, std::unique_ptr<LZWNode>> children;

        public:
            LZWNode() : value(0), leaf(false), children() {} 
            LZWNode(uint32_t v, bool l) : value(v), leaf(l), children() {} 
    };
    uint32_t cur_key;
    std::unique_ptr<LZWNode> root;

    // Encoded stream
    std::stringstream out;

    LZWNode* insert(const lzw_symbol_t c, LZWNode* node);
    LZWNode* search_node(const lzw_symbol_t c, LZWNode* node) const;
    void clear_dict(void);

    public:
        LZWEncoder();
        void init(void);
        bool contains(const std::string_view data) const;
        // TODO: re-write to return void, add write() method
        std::stringstream encode(std::stringstream& input);
        //std::vector<uint16_t> get_code(const std::string_view word) const; // <- debug only, remove
};



/*
 * LZWDecoder
 * Object oriented decoder using LZW algorithm. The idea is that you can decode 
 * data larger than "memory" by breaking it into chunks and calling decode() in a loop.
 */
class LZWDecoder
{
    // Symbol table
    std::vector<std::string> table;

    uint32_t offset24;
    uint32_t offset32;
    uint32_t num_codes;
    // We assume first chunk contains header. On the first chunk we read the header 
    // information and set read_header true. If true then when we call decode() the first 12 
    // bytes are assumed to be part of the data stream.
    bool read_header;       

    std::stringstream out;


    public:
        LZWDecoder();
        void init(void);
        std::stringstream decode(std::stringstream& input);
};



#endif /*__LZW_HPP*/
