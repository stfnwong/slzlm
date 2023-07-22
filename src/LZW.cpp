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




LZStream LZWDict::encode(std::stringstream& input)
{
    LZStream out;
    auto* node = this->root.get();
    unsigned bytes_per_code = 2;
    unsigned size = 0;      // total stream length

    char c;
    while(input)
    {
        if(!input.get(c))
            break;
        if(input.eof() || input.fail())
            break;

        auto* result_node = this->search_node(lzw_symbol_t(c), node);
        if(!result_node)
        {
            out.write(node->value, bytes_per_code);
            this->insert(lzw_symbol_t(c), node);
            node = this->root->children.find(c)->second.get();
            size += bytes_per_code;

            if(this->cur_key == 0xFFFF)
            {
                bytes_per_code = 3;
                out.offset24 = size;
            }
            if(this->cur_key == 0xFFFFFF)
            {
                bytes_per_code = 4;
                out.offset32 = size;
            }
        }
        else
            node = result_node;
    }

    out.write(node->value, bytes_per_code);
    size += bytes_per_code;
    out.size = size;
    out.num_codes = this->cur_key;

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
    std::stringstream out;

    size_t size = 0;        // output length in bytes
    int bytes_per_code = 2;
    auto* node = this->root.get();

    char c;
    while(input)
    {
        if(!input.get(c))
            break;
        if(input.eof() || input.fail())
            break;

        auto result_node = this->search_node(lzw_symbol_t(c), node);

        if(!result_node)
        {
            out.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);
            this->insert(lzw_symbol_t(c), node);
            node = this->root->children.find(c)->second.get();  // previous character node

            size += bytes_per_code;
            
            // if we have encoded a large value adjust the size of the output code 
            if(this->cur_key == 0xFFFF)
                bytes_per_code = 3;
            if(this->cur_key == 0xFFFFFF)
                bytes_per_code = 4;
        }
        else
            node = result_node;    // input concat node
    }
    out.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);
    size += bytes_per_code;

    return out;
}


// TODO: don't use this - its just for testing the algorithm
std::stringstream LZWDict::decode(const std::vector<uint32_t>& data) const
{
    std::stringstream out;
    std::vector<std::vector<uint32_t>> table(LZW_ALPHA_SIZE);
    //std::vector<std::string> table(LZW_ALPHA_SIZE);
    std::vector<uint32_t> s;

    // Init table 
    for(uint16_t i = 0; i < LZW_ALPHA_SIZE; ++i)
        table[i].push_back(i);

    unsigned cur_key = table.size();

    uint32_t old_code = data[0];         // old 
    uint32_t new_code = 0;         // new
    uint32_t c = 0;
    //uint32_t cur_seq;

    // write the first code 
    out.write(reinterpret_cast<const char*>(&table[old_code][0]), sizeof(uint16_t));

    // In real life we need to keep track of the code size
    for(unsigned i = 1; i < data.size(); ++i)
    {
        new_code = data[i];    // artificially this is always the same size
        if(new_code >= table.size())
        {
            s.push_back(old_code);
            s.push_back(c);
            // s = t(old + c)
            //cur_seq = old_code + new_code;
            //s.push_back(cur_code + table[old_code]);
        }
        else
        {
            s.clear();
            for(const auto elem : table[new_code])
                s.push_back(elem);
            //cur_seq = table[new_code];
        }

        // output s
        for(const auto symbol : s)
            out.write(reinterpret_cast<const char*>(&symbol), sizeof(uint16_t));
        c = s[0];
        std::cout << "[" << __func__ << "] s at [" << i << "]: ";
        for(const auto symbol : s)
            std::cout << symbol << " ";
        std::cout << std::endl;

        table.push_back({old_code, c});

        // update table
        //table[cur_code] = (table[old_code] + cur_code);
        old_code = new_code;
        cur_key++;
    }

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


