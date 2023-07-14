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
    auto* node = root.get();
    for(auto const c: word) 
    {
        auto& children = node->children;
        auto it = children.find(c);
        
        // If we reach the end and don't find it then the search has failed
        if(it == children.end())
            return false;

        node = it->second.get();
    }

    return node->leaf;
}
