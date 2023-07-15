/*
 * NODE
 * Trie Node
 */

#include "Node.hpp"




void Trie::insert(const std::string_view word)
{
    auto* node = this->root.get();
    for(auto const c: word)
    {
        auto& children = node->children;
        auto it = children.find(c);
        if(it == children.end())
            it = children.emplace(c, std::make_unique<TrieNode>()).first;
        
        node = it->second.get();
    }

    node->leaf = true;
}


bool Trie::search(const std::string_view word) const
{
    auto* node = this->root.get();
    for(auto const c: word) 
    {
        auto& children = node->children;
        auto it = children.find(c);
        if(it == children.end())
            return false;            // we don't have it

        node = it->second.get();
    }

    return node->leaf;
}


// Version using arrays rather than maps


void TrieArray::insert(const std::string_view word)
{
    auto* node = this->root.get();
    for(auto const c: word)
    {
        int idx = int(c);
        if(!node->children[idx].get())
            node->children[idx] = std::make_unique<TrieArrayNode>();

        node = node->children[idx].get();
    }

    node->leaf = true;
}

bool TrieArray::search(const std::string_view word) const
{
    auto* node = this->root.get();
    for(auto const c: word)
    {
        int idx = int(c);
        if(!node->children[idx].get())
            return false;
        node = node->children[idx].get();
    }

    return node->leaf;
}




// TODO: for LZ I think I need a version that gives back the key
