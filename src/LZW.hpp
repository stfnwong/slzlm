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
#include <sstream>



using lzw_symbol_t = uint8_t;
const constexpr uint32_t LZW_ALPHA_SIZE = 1 << (8 * sizeof(lzw_symbol_t));


// Encode function
std::stringstream lzw_encode(const std::string_view data);
std::stringstream lzw_decode(std::stringstream& data);

/*
 * Encode from a std::stringstream rather than a std::string_view
 */
std::stringstream lzw_encode_from_stream(std::stringstream& data);


/*
 * Encoder where the node contains an array rather than a map
 */
std::stringstream lzw_array_encode(const std::string_view data);

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
    LZWNode* cur_node;

    // Header information
    uint32_t offset24;
    uint32_t offset32;
    uint32_t num_codes;
    int bytes_per_code;
    bool insert_header;

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
        void encode(const std::string_view input);
        std::string get(void);
        std::stringstream get_stream(void);
        void to_file(const std::string& filename);
        unsigned size(void) const;
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
    int bytes_per_code;
    // We assume first chunk contains header. On the first chunk we read the header 
    // information and set read_header true. If true then when we call decode() the first 12 
    // bytes are assumed to be part of the data stream.
    bool read_header;       

    std::stringstream out;

    public:
        LZWDecoder();
        void init(void);
        void decode(std::stringstream& input);
        std::string get(void);
        std::stringstream get_stream(void);
        void to_file(const std::string& filename);
        unsigned size(void) const;
};



#endif /*__LZW_HPP*/
