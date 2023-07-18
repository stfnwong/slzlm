/*
 * COMP
 * Compressors
 *
 */

#include <stack>
#include <utility>

// TODO: debug only
#include <iostream>

#include "comp.hpp"


LZWDict::LZWDict() : cur_key(0), root(std::make_unique<LZWNode>())
{
    for(uint32_t code = 0; code < ALPHA_SIZE; ++code)
        this->insert(lzw_symbol_t(code), this->root.get());
}


//LZWNode* LZWDict::insert(const std::string_view data, LZWNode* node)

LZWNode* LZWDict::insert(const lzw_symbol_t c, LZWNode* node)
{
    auto& children = node->children;
    auto it = children.find(c);
    
    if(it == children.end())
        it = children.emplace(c, std::make_unique<LZWNode>()).first;

    node = it->second.get();
    node->value = this->cur_key;
    node->leaf = true;
    this->cur_key++;

    return node;
}


LZWNode* LZWDict::search_node(const lzw_symbol_t c, LZWNode* node) const
{
    auto& children = node->children;
    auto it = children.find(c);
    if(it == children.end())
        return nullptr;

    node = it->second.get();

    return node;
}


bool LZWDict::contains(const std::string_view data) const
{
    auto* node = this->root.get();

    for(auto const c: data) 
    {
        auto& children = node->children;
        auto it = children.find(lzw_symbol_t(c));
        if(it == children.end())
            return false;

        node = it->second.get();
    }

    return true;
}


// TODO: this may turn into decode....? 
std::vector<uint16_t> LZWDict::get_code(const std::string_view word) const
{
    std::vector<uint16_t> out;
    auto* node = this->root.get();

    for(auto const c: word)
    {
        auto& children = node->children;
        auto it = children.find(lzw_symbol_t(c));
        if(it == children.end())
        {
            out.push_back(node->value);
            node = this->root->children.find(c)->second.get();
        }
        else
            node = it->second.get();
    }

    if(node->leaf)
        out.push_back(node->value);

    return out;
}



std::vector<uint32_t> LZWDict::encode(const std::string_view data)
{
    std::vector<uint32_t> out;
    auto* node = this->root.get();

    for(auto const c : data)
    {
        auto* result_node = this->search_node(lzw_symbol_t(c), node);
        if(result_node == nullptr)
        {
            out.push_back(node->value);
            this->insert(lzw_symbol_t(c), node);
            node = this->root->children.find(c)->second.get();  // p = c node
            //node = this->insert(lzw_symbol_t(c), node);  // this line makes a very deep tree
        }
        else
            node = result_node;    // (p = p + c) node  
    }
    
    return out;
}


std::vector<uint8_t> LZWDict::decode(const std::vector<uint16_t>& data) const
{
    std::vector<uint8_t> out;

    return out;         // shut linter up
}



// DEBUG FUNCTIONS 

std::vector<std::vector<uint32_t>> LZWDict::find_all(void) const
{
    std::vector<std::vector<uint32_t>> out;
    std::vector<uint32_t> traversal;

    using StackData = std::pair<LZWNode*, int>;

    std::stack<StackData> s;
    LZWNode* node = this->root.get();
    s.push({node, 0});

    int cur_level = 0;
    while(!s.empty())
    {
        auto stack_data = s.top();
        s.pop();
        LZWNode* cur_node = stack_data.first;
        int node_level = stack_data.second;

        if(node_level < cur_level)
        {
            out.push_back(traversal);
            traversal.clear();
        }

        if(cur_level > 0)
            traversal.push_back(cur_node->value);

        auto& children = cur_node->children;
        for(auto it = children.begin(); it != children.end(); ++it)
            s.push({it->second.get(), cur_level+1});
        cur_level++;
    }
    

    return out;
}
