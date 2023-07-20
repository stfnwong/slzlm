/*
 * COMP
 * Compressors
 *
 */

#include <stack>
#include <utility>
#include <fstream>

// TODO: debug only, remove these
#include <iostream>
#include <iomanip>


#include "LZW.hpp"


// LZWDict private methods
LZWDict::LZWNode* LZWDict::insert(const lzw_symbol_t c, LZWNode* node)
{
    auto& children = node->children;
    auto it = children.find(c);
    
    if(it == children.end())
        it = children.emplace(c, std::make_unique<LZWNode>(this->cur_key, true)).first;

    node = it->second.get();
    this->cur_key++;

    return node;
}


LZWDict::LZWNode* LZWDict::search_node(const lzw_symbol_t c, LZWNode* node) const
{
    auto& children = node->children;
    auto it = children.find(c);
    if(it == children.end())
        return nullptr;

    node = it->second.get();

    return node;
}


// Ctor
LZWDict::LZWDict() : cur_key(0), root(std::make_unique<LZWNode>())
{
    this->init();
}

void LZWDict::init(void)
{
    for(uint32_t code = 0; code < LZW_ALPHA_SIZE; ++code)
        this->insert(lzw_symbol_t(code), this->root.get());
}

void LZWDict::clear_dict(void)
{
    // Dfs through tree and clear from leaf to root;
    std::stack<LZWNode*> s;

    LZWNode* node = this->root.get();
    s.push(node);

    while(!s.empty())
    {
        LZWNode* cur_node = s.top();
        s.pop();

        auto& children = cur_node->children;
        if(children.size() > 0)
        {
            for(auto it = children.begin(); it != children.end(); ++it)
            {
                if(it->second->children.size() > 0)
                    s.push(it->second.get());
                else
                    it->second->children.clear();
            }
        }
    }
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
            //node = this->insert(lzw_symbol_t(c), node);  // this implementation is incorrect - makes very deep tree
        }
        else
            node = result_node;    // (p = p + c) node  
    }
    
    return out;
}


// TODO: replace fstream with ostream?
void LZWDict::encode_to_file(const std::string& filename, const std::string_view data)
{
    std::ofstream file(filename, std::ios::binary);

    // Write empty space for header
    file.seekp(1 * sizeof(uint32_t), file.beg);         // number of 24-bit codes 
    file.seekp(2 * sizeof(uint32_t), file.beg);         // number of 32-bit codes
    file.seekp(3 * sizeof(uint32_t), file.beg);         // total number of codes

    int bytes_per_code = 2;
    auto* node = this->root.get();

    // record offset for 24 and 32 bit codes here 
    int offset_24bit_codes = 0;
    int offset_32bit_codes = 0;

    for(auto const c: data)
    {
        auto* result_node = this->search_node(lzw_symbol_t(c), node);
        if(!result_node)
        {
            file.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);
            this->insert(lzw_symbol_t(c), node);
            node = this->root->children.find(c)->second.get();

            if(this->cur_key == 0xFFFF)
            {
                bytes_per_code = 3;         // 2^16 - 1
                offset_24bit_codes = int(file.tellp());
            }
            else if(this->cur_key == 0xFFFFFF)
            {
                bytes_per_code = 4;         // 2^24 - 1
                offset_32bit_codes = int(file.tellp());
            }
        }
        else
            node = result_node;
    }

    file.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);

    // Write header information
    file.seekp(0, file.beg);
    file.write(reinterpret_cast<const char*>(&offset_24bit_codes), sizeof(uint32_t));
    file.write(reinterpret_cast<const char*>(&offset_32bit_codes), sizeof(uint32_t));
    file.write(reinterpret_cast<const char*>(&this->cur_key), sizeof(uint32_t));

    file.close();
}


// TODO: maybe definition goes in some other file 
//std::string LZWDict::py_encode(const std::string_view data


std::stringstream LZWDict::encode2(std::stringstream& input)
{
    // We can't set up header here so skip
    //std::stringstream out(std::stringstream::binary);
    std::stringstream out;

    int bytes_per_code = 2;
    auto* node = this->root.get();

    while(!input.eof())
    {
        uint8_t c;
        input.read((char*)&c, sizeof(uint8_t));
        std::cout << "[" << __func__ << "] read " << c << " from input" << std::endl;

        auto result_node = this->search_node(lzw_symbol_t(c), node);

        if(!result_node)
        {
            out.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);
            this->insert(lzw_symbol_t(c), node);
            node = this->root->children.find(c)->second.get();  // previous character node
            
            // if we have encoded a large value adjust the size of the output code 
            if(this->cur_key == 0xFFFF)
                bytes_per_code = 3;
            if(this->cur_key == 0xFFFFFF)
                bytes_per_code = 4;
        }
        else
            node = result_node;    // input concat node
    }

    return out;
}


// TODO: don't use this - its just for testing the algorithm
std::vector<uint16_t> LZWDict::decode(const std::vector<uint32_t>& data) const
{
    std::vector<uint16_t> out;



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


