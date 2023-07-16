/*
 * TRIE
 */



#ifndef __TRIE_HPP
#define __TRIE_HPP

#include <array>
#include <map>
#include <memory>
#include <string_view>


static constexpr const uint32_t NULL_KEY_VALUE = 0;


// TODO: this can become private member of Trie
struct TrieNode
{
    bool leaf;
    uint32_t value;
    std::map<char, std::unique_ptr<TrieNode>> children;

    public:
        TrieNode() : leaf(false), value(1), children() {}
        TrieNode(uint32_t v) : leaf(false), value(v), children() {}
};



class Trie
{
    std::unique_ptr<TrieNode> root;
    uint32_t cur_key;           // Note that key 0 is implicitly null key

    // common search implementation
    //std::unique_ptr<TrieNode> search_inner(const std::string_view word) const;
    const TrieNode* search_inner(const std::string_view word) const;

    public:
        Trie() : root(std::make_unique<TrieNode>()), cur_key(1) {}
        void insert(const std::string_view word);
        bool search(const std::string_view word) const;
        uint32_t search_key(const std::string_view word) const;
};



// TODO: Test the map implementation against the array implementation
constexpr const int N = 256;            // enough for ASCII

// Implementation that uses arrays rather than a map
struct TrieArrayNode
{
    bool leaf;
    uint32_t value;          // this is meant to be like the "key" for a given string
    std::array<std::unique_ptr<TrieArrayNode>, N> children;
    //std::array<TrieNode*, N> children;

    public:
        TrieArrayNode() : leaf(false), value(1), children() {}
        TrieArrayNode(uint32_t v) : leaf(false), value(v), children() {}
};



class TrieArray
{
    // need a global here to count the number of leaves
    std::unique_ptr<TrieArrayNode> root;
    uint32_t cur_key;

    const TrieArrayNode* search_inner(const std::string_view word) const;

    public:
        TrieArray() : root(std::make_unique<TrieArrayNode>()), cur_key(1) {} 
        void insert(const std::string_view word);
        bool search(const std::string_view word) const;
        uint32_t search_key(const std::string_view word) const;
};


#endif /*__TRIE_HPP*/
