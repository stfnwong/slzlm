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


constexpr const int N = 256;            // enough for ASCII
                                        //

// TODO: this can become private member of Trie
struct TrieNode
{
    bool leaf;
    uint32_t value;
    std::map<char, std::unique_ptr<TrieNode>> children;
    //std::array<TrieNode*, N> children;
    //TrieNode* children[N];


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



#endif /*__NODE_HPP*/
