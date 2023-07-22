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


#include "Stream.hpp"



const constexpr uint32_t LZW_ALPHA_SIZE = 256;
using lzw_symbol_t = uint16_t;


// TODO: re-write as functions

// Encode function
std::stringstream lzw_encode(const std::string_view data);
std::stringstream lzw_decode(std::stringstream& data);


/*
 * This will turn into an object-oriented encoder
 */
class LZWDict
{
    // Prefix tree node
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

    LZWNode* insert(const lzw_symbol_t c, LZWNode* node);
    LZWNode* search_node(const lzw_symbol_t c, LZWNode* node) const;


    public:
        LZWDict();
        void init(void);
        void clear_dict(void);

        std::stringstream encode(std::stringstream& input);

        std::vector<uint16_t> get_code(const std::string_view word) const;       // <- debug only, remove
        std::stringstream decode(const std::vector<uint32_t>& data) const;
        bool contains(const std::string_view data) const;

        // TODO: debug, remove 
        std::vector<std::vector<uint32_t>> find_all(void) const;
};





#endif /*__LZW_HPP*/
