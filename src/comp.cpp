/*
 * COMP
 * Compressors
 *
 */


#include "comp.hpp"


LZWDict::LZWDict() : cur_key(1), root(std::make_unique<TrieNode>())
{
    // we may need to do something here 
    //for(uint32_t i = 0; i < ALPHA_SIZE; ++i)
    //    this->trie.insert(std::to_string(i));

    for(uint32_t code = 0; code < ALPHA_SIZE; ++code)
        this->insert(char(code), this->root.get());
}


//TrieNode* LZWDict::insert(const std::string_view data, TrieNode* node)

TrieNode* LZWDict::insert(const char c, TrieNode* node)
{
    //auto* node = this->root.get();
    
    auto& children = node->children;
    auto it = children.find(c);
    if(it == children.end())
        it = children.emplace(c, std::make_unique<TrieNode>()).first;

    node = it->second.get();
    node->value = this->cur_key;
    this->cur_key++;
    return node;
}


TrieNode* LZWDict::search_node(const char c, TrieNode* node) const
{
    auto& children = node->children;
    auto it = children.find(c);
    if(it == children.end())
        return nullptr;

    node = it->second.get();

    return node;
}



std::vector<uint32_t> LZWDict::construct(const std::string_view data)
{
    std::vector<uint32_t> out;

    //for(auto const c: text)
    //{
    //    uint32_t k = this->trie.search_key(c);
    //    if(k > NULL_KEY_VALUE)
    //}
    auto* node = this->root.get();

    for(auto const c : data)
    {
        auto* result_node = this->search_node(c, node);
        if(result_node != nullptr)
            node = result_node;    // (p = p + c) node 
        else
        {
            node = this->insert(c, node);
            out.push_back(node->value);
            // if this node == nullptr then we fucked up
        }
    }
    
    return out;
}

