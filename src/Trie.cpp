/*
 * TRIE
 * Trie Node
 */

#include "Trie.hpp"


// Private inner implementation of search
const TrieNode* Trie::search_inner(const std::string_view word) const
{
    const auto* node = this->root.get();
    for(auto const c: word) 
    {
        auto& children = node->children;
        auto it = children.find(c);
        if(it == children.end())
            return nullptr;

        node = it->second.get();
    }

    return node;
}



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

    node->value = this->cur_key;
    this->cur_key++;
    node->leaf = true;
}

bool Trie::search(const std::string_view word) const
{
    auto result = this->search_inner(word);
    if(result != nullptr)
        return result->leaf;

    return false;
}


uint32_t Trie::search_key(const std::string_view word) const
{
    auto result = this->search_inner(word);
    if(result != nullptr)
        return result->value;

    return NULL_KEY_VALUE;
}



// Version using arrays rather than maps


const TrieArrayNode* TrieArray::search_inner(const std::string_view word) const
{
    auto* node = this->root.get();
    for(auto const c: word)
    {
        int idx = int(c);
        if(!node->children[idx].get())
            return nullptr;

        node = node->children[idx].get();
    }

    return node;
}


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

    node->value = this->cur_key;
    this->cur_key++;
    node->leaf = true;
}

bool TrieArray::search(const std::string_view word) const
{
    auto result = this->search_inner(word);
    if(result != nullptr)
        return result->leaf;

    return false;
}

uint32_t TrieArray::search_key(const std::string_view word) const
{
    auto result = this->search_inner(word);
    if(result != nullptr)
        return result->value;

    return NULL_KEY_VALUE;
}
