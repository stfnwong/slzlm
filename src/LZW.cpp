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


/*
 * Node for function implementation
 */
struct Node
{
    uint32_t value;
    bool leaf;
    std::map<lzw_symbol_t, std::unique_ptr<Node>> children;

    public:
        Node() {} 
        Node(uint32_t v, bool l) : value(v), leaf(l) {}
};




/*
 * Encode data to LZW stream
 */
std::stringstream lzw_encode(const std::string_view data)
{
    std::stringstream out;
    int bytes_per_code = 2;    // minimum code size is 16-bits
    int offset24 = 0;
    int offset32 = 0;
    int num_codes = 0;
    uint32_t cur_key = 0;

    // Write offset data to start of stream
    out.seekp(0);
    // write dummy values for later 
    out.write(reinterpret_cast<const char*>(&offset24), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&offset32), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&num_codes), sizeof(uint32_t));

    std::unique_ptr<Node> prefix_tree = std::make_unique<Node>();

    // Init the dict 
    auto* node = prefix_tree.get();
    for(cur_key = 0; cur_key < LZW_ALPHA_SIZE; ++cur_key)
        node->children.emplace(cur_key, std::make_unique<Node>(cur_key, true));

    for(const auto& c: data)
    {
        auto& children = node->children;
        // see if we have c already
        auto it = children.find(c);

        if(it == children.end())
        {
            // write this code to output
            out.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);
            // insert 
            it = node->children.find(c);
            if(it == children.end())
                it = children.emplace(c, std::make_unique<Node>(cur_key, true)).first;
            node = prefix_tree->children.find(c)->second.get();
            cur_key++;

            if(cur_key == 0xFFFF)
            {
                bytes_per_code = 3;
                offset24 = out.tellp();
            }
            if(cur_key == 0xFFFFFF)
            {
                bytes_per_code = 4;
                offset32 = out.tellp();
            }
        }
        else
            node = it->second.get();
    }
    out.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);
    // go back and write the offsets
    out.seekp(0, std::ios::beg);
    out.write(reinterpret_cast<const char*>(&offset24), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&offset32), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&cur_key), sizeof(uint32_t));

    return out;
}


/*
 * Decode LZW stream
 */
std::stringstream lzw_decode(std::stringstream& input)
{
    // Assume that the input contains header with format 
    // offset24 (4 bytes)  - offset where first 24-bit code occurs. 0 = no 24 bit codes
    // effset32 (4 bytes)  - offset where first 32-bit code occurs. 0 = no 32 bit codes
    // num_codes (4 bytes) - total number of unique codes
    //
    // first code is guaranteed to be min_code_size (2 bytes) long.

    std::stringstream out;

    // Decode header
    uint32_t header[3];     // offset24, offset32, num_codes
    char header_buf[4];
    
    for(unsigned h = 0; h < 3; ++h)
    {
        input.read(header_buf, sizeof(uint32_t));
        header[h] = 
            header_buf[3] << 24 | 
            header_buf[2] << 16 | 
            header_buf[1] << 8 | 
            header_buf[0];
    }


    std::vector<std::string> table(LZW_ALPHA_SIZE);

    // Create symbol table
    for(unsigned i = 0; i < LZW_ALPHA_SIZE; ++i)
        table[i] += i;

    std::string s;
    int bytes_per_code = 2;
    unsigned old_code = 0;
    unsigned new_code = 0;
    char buf;
    char cc = 0;

    // Get the first code
    for(int i = 0; i < bytes_per_code; ++i)
    {
        if(!input.get(buf))
            return out;
        old_code = old_code | (buf << (8 * i));
    }
    s += old_code;
    out << s;

    while(input)
    {
        // Check what the code size should be 
        if(input.tellg() == header[0])
            bytes_per_code = 3;

        if(input.tellg() == header[1])
            bytes_per_code = 4;

        new_code = 0;
        for(int i = 0; i < bytes_per_code; ++i)
        {
            if(!input.get(buf))
                break;
            new_code = new_code | (buf << (8 * i));
        }

        if(input.eof() || input.fail())
            break;

        if(new_code >= table.size())   // unseen code
        {
            s = table[old_code];
            s += cc;
        }
        else
        {
            s = "";
            s += table[new_code];
        }
        cc = s[0];
        out << s;

        std::string new_sym;
        new_sym += table[old_code];
        new_sym += cc;

        table.push_back(new_sym);
        old_code = new_code;
    }

    return out;
}


/*
 * LZW Objects
 */

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
    


/*
 * contains()
 * Returns true if there is a prefix that matches the sequence in data
 */
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


/*
 * Return the prefix code for some sequence if it exists, otherwise 
 * return an empty vector.
 */
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




/*
 * Encode a stream into an LZW stream
 */
std::stringstream LZWDict::encode(std::stringstream& input)
{
    std::stringstream out; // this can become a member, the idea being we can keep calling encode on long strings (longer than "memory")
    unsigned bytes_per_code = 2;

    int offset32 = 0;
    int offset24 = 0;
    int num_codes = 0;

    // reserve some space in the stream for offsets
    out.write(reinterpret_cast<const char*>(&offset24), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&offset32), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&num_codes), sizeof(uint32_t));

    char c;
    auto* node = this->root.get();

    while(input)
    {
        if(!input.get(c))
            break;
        if(input.eof() || input.fail())
            break;

        auto* result_node = this->search_node(lzw_symbol_t(c), node);
        if(!result_node)
        {
            out.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);
            this->insert(lzw_symbol_t(c), node);
            node = this->root->children.find(c)->second.get();

            if(this->cur_key == 0xFFFF)
            {
                bytes_per_code = 3;
                offset24 = out.tellp();
            }
            if(this->cur_key == 0xFFFFFF)
            {
                bytes_per_code = 4;
                offset32 = out.tellp();
            }
        }
        else
            node = result_node;
    }

    out.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);

    // update header 
    out.seekp(0, std::ios::beg);
    out.write(reinterpret_cast<const char*>(&offset24), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&offset32), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&this->cur_key), sizeof(uint32_t));

    return out;
}


// TODO: don't use this - its just for testing the algorithm
std::stringstream LZWDict::decode(std::stringstream& input) const
{
    std::stringstream out;

    uint32_t header[3]; // 0=offset24, 1=offset32, 2=num_codes
    char header_buf[4];

    for(unsigned h = 0; h < 3; ++h)
    {
        input.read(header_buf, sizeof(uint32_t));
        header[h] = 
            header_buf[3] << 24 | 
            header_buf[2] << 16 | 
            header_buf[1] << 8 | 
            header_buf[0];
    }

    // Init table 
    std::vector<std::string> table(LZW_ALPHA_SIZE);
    for(unsigned i = 0; i < LZW_ALPHA_SIZE; ++i)
        table[i] += i;

    // Decode loop
    std::string s;
    int bytes_per_code = 2;
    unsigned old_code = 0;
    unsigned new_code = 0;
    char cc = 0;
    char buf;

    // Read first code and write directly to output
    for(int i = 0; i < bytes_per_code; ++i)
    {
        if(!input.get(buf))
            return out;             // we can't recover stream anyway, just quit
        old_code = old_code | (buf << (8 * i));
    }
    s += old_code;
    out << s;

    while(input)
    {
        // Check what the code size should be 
        if(input.tellg() == header[0])
            bytes_per_code = 3;

        if(input.tellg() == header[1])
            bytes_per_code = 4;

        new_code = 0;
        for(int i = 0; i < bytes_per_code; ++i)
        {
            if(!input.get(buf))
                break;
            new_code = new_code | (buf << (8 * i));
        }

        if(input.eof() || input.fail())
            break;

        if(new_code >= table.size())   // unseen code
        {
            s = table[old_code];
            s += cc;
        }
        else
        {
            s = "";
            s += table[new_code];
        }
        cc = s[0];
        out << s;

        std::string new_sym;
        new_sym += table[old_code];
        new_sym += cc;

        table.push_back(new_sym);
        old_code = new_code;
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


