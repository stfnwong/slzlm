/*
 * COMP
 * Compressors
 *
 */

#ifndef __COMP_HPP
#define __COMP_HPP

#include <string>
#include <vector>

#include "Trie.hpp"


const constexpr uint32_t ALPHA_SIZE = 256;



class LZWDict
{
    uint32_t cur_key;
    std::unique_ptr<TrieNode> root;

    //TrieNode* insert(const std::string_view data, TrieNode* node);
    TrieNode* insert(const char c, TrieNode* node);
    TrieNode* search_node(const char c, TrieNode* node) const;

    public:
    // TODO: missing the "init dict with single character strings" step
        LZWDict();
        uint32_t search(const std::string_view data) const;
        std::vector<uint32_t> construct(const std::string_view data);
};



#endif /*__COMP_HPP*/
