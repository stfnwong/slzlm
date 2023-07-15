/*
 * NODE
 * Trie Node
 */



#ifndef __NODE_HPP
#define __NODE_HPP

#include <array>
#include <map>
#include <memory>
#include <string_view>



// TODO: this can become private member of Trie
struct TrieNode
{
    bool leaf;
    uint32_t value;
    std::map<char, std::unique_ptr<TrieNode>> children;

    public:
        TrieNode() : leaf(false), value(0), children() {}
};





class Trie
{
    std::unique_ptr<TrieNode> root;

    public:
        Trie() : root(std::make_unique<TrieNode>()) {}
        void insert(const std::string_view word);
        bool search(const std::string_view word) const;
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
        TrieArrayNode() : leaf(false), value(0), children() {}
};



class TrieArray
{
    // need a global here to count the number of leaves
    std::unique_ptr<TrieArrayNode> root;

    public:
        TrieArray() : root(std::make_unique<TrieArrayNode>()) {} 
        void insert(const std::string_view word);
        bool search(const std::string_view word) const;
};


#endif /*__NODE_HPP*/
