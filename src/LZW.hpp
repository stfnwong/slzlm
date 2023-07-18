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



const constexpr uint32_t ALPHA_SIZE = 256;
using lzw_symbol_t = uint16_t;


struct LZWNode
{
    uint32_t value;
    bool leaf;
    std::map<lzw_symbol_t, std::unique_ptr<LZWNode>> children;
    //std::map<std::string, std::unique_ptr<LZWNode>> children;

    public:
        LZWNode() : value(0), leaf(false), children() {} 
        //LZWNode(uint32_t v() : value(v), leaf(false), children() {} 
};



class LZWDict
{
    uint32_t cur_key;
    std::unique_ptr<LZWNode> root;

    //LZWNode* insert(const std::string_view data, LZWNode* node);
    LZWNode* insert(const lzw_symbol_t c, LZWNode* node);
    LZWNode* search_node(const lzw_symbol_t c, LZWNode* node) const;

    //void walk(LZWNode* node, std::vector<uint32_t>& output);

    public:
    // TODO: missing the "init dict with single character strings" step
        LZWDict();
        //uint32_t search(const std::string_view data) const;
        std::vector<uint32_t> encode(const std::string_view data);
        // TODO: type for vectors should be something else 
        std::vector<uint16_t> get_code(const std::string_view word) const;       // <- debug only, remove
        std::vector<uint8_t> decode(const std::vector<uint16_t>& data) const;
        bool contains(const std::string_view data) const;

        // TODO: debug, remove 
        std::vector<std::vector<uint32_t>> find_all(void) const;
};



#endif /*__LZW_HPP*/
