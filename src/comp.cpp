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
    // we may need to do something here 
    //for(uint32_t i = 0; i < ALPHA_SIZE; ++i)
    //    this->trie.insert(std::to_string(i));

    for(uint32_t code = 0; code < ALPHA_SIZE; ++code)
        this->insert(lzw_symbol_t(code), this->root.get());
}


//LZWNode* LZWDict::insert(const std::string_view data, LZWNode* node)

LZWNode* LZWDict::insert(const lzw_symbol_t c, LZWNode* node)
{
    auto& children = node->children;
    auto it = children.find(c);
    
    //std::cout << "[" << __func__ << "] inserting [" << c << "] at node [" << node->value << "]";
    std::cout << "[" << __func__ << "] ";
    if(it == children.end())
    {
        std::cout << "inserting [" << c << "] -> [" << node->value << "]";
        it = children.emplace(c, std::make_unique<LZWNode>()).first;
    }

    node = it->second.get();
    node->value = this->cur_key;
    node->leaf = true;
    std::cout << " returning node [" << node->value << "]" << std::endl;
    // TODO: debug only, remove 
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


// TODO: remove all print statements
std::vector<uint16_t> LZWDict::get_code(const std::string_view word) const
{
    std::vector<uint16_t> out;
    auto* node = this->root.get();

    std::cout << "[" << __func__ << "] input symbol [" << word << "]" << std::endl;
    for(auto const c: word)
    {
        std::cout << " checking [" << c << "]... ";

        auto& children = node->children;
        auto it = children.find(lzw_symbol_t(c));
        if(it == children.end())
        {
            std::cout << " no child with symbol [" << c << "]";
            out.push_back(node->value);
            node = this->root.get();
        }
        else
        {
            std::cout << " have prefix for [" << c << "]";
            node = it->second.get();
        }
        std::cout << std::endl;

    }
    //out.push_back(node->value);
    //if(out.size() == 0 && node != nullptr && node->leaf)
    //if(node != this->root.get())
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
        // TODO: lookahead isn't working correctly...
        if(result_node == nullptr)
        {
            out.push_back(node->value);
            node = this->insert(lzw_symbol_t(c), node);
            // if this node == nullptr then we fucked up
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
