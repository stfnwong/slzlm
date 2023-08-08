/*
 * COMP
 * Compressors
 *
 */

#include <array>
#include <stack>
#include <utility>
#include <fstream>

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





// Can this work if we only take one byte at a time?
//
// const uint8_t* inp_data - pointer to input array
// unsigned inp_length - length of input 
// uint8_t* out_data - pointer to output array this must be allocated and long enough
// 
// return:
//  unsigned - actual length of output array
unsigned lzw_encode(
        const uint8_t* inp_data, 
        unsigned inp_length, 
        uint8_t* out_data
        )
{
    int bytes_per_code = 2;    // minimum code size is 16-bits
    uint32_t offset24 = 0;
    uint32_t offset32 = 0;
    uint32_t cur_key = 0;

    unsigned out_ptr = 12;   // skip over the header section - we will fill at the end
    unsigned inp_ptr = 0;

    // Init prefix tree and dict
    std::unique_ptr<Node> root = std::make_unique<Node>();

    auto* node = root.get();
    for(cur_key = 0; cur_key < LZW_ALPHA_SIZE; ++cur_key)
        node->children.emplace(cur_key, std::make_unique<Node>(cur_key, true));

    for(inp_ptr = 0; inp_ptr < inp_length; ++inp_ptr)
    {
        // see if we have c already
        uint8_t c = inp_data[inp_ptr];
        auto it = node->children.find(c);

        if(it == node->children.end())
        {
            for(int b = 0; b < bytes_per_code; ++b)
            {
                out_data[out_ptr] = (node->value >> (8*b)) & 0xFF;
                out_ptr++;
            }

            node->children.emplace(c, std::make_unique<Node>(cur_key, true));
            // p = c
            node = root->children.find(c)->second.get();
            cur_key++;

            if(cur_key == 0xFFFF)
            {
                bytes_per_code = 3;
                offset24 = out_ptr;
            }
            if(cur_key == 0xFFFFFF)
            {
                bytes_per_code = 4;
                offset32 = out_ptr;
            }
        }
        else
            node = it->second.get();
    }
    
    // Write the last code
    for(int b = 0; b < bytes_per_code; ++b)
    {
        out_data[out_ptr] = (node->value >> (8*b)) & 0xFF;
        out_ptr++;
    }

    // Write header 
    for(int i = 0; i < 4; ++i)
        out_data[i] = (offset24 >> (8*i)) & 0xFF;       // lowest -> highest
    for(int i = 0; i < 4; ++i)
        out_data[i+4] = (offset32 >> (8*i)) & 0xFF;       // lowest -> highest
    for(int i = 0; i < 4; ++i)
        out_data[i+8] = (cur_key >> (8*i)) & 0xFF;       // lowest -> highest

    return out_ptr;
}


// As with encode, we expect inp_data and out_data to be allocated.
std::vector<uint8_t> lzw_decode(const uint8_t* inp_data, unsigned inp_length)
{
    std::vector<uint8_t> out;

    unsigned inp_ptr;

    // Reader header info 
    uint32_t offset24 = 0;
    uint32_t offset32 = 0;
    uint32_t num_codes = 0;


    for(unsigned i = 0; i < 4; ++i)
        offset24 = offset24 | (inp_data[i] << (8*i));
    for(unsigned i = 0; i < 4; ++i)
        offset32 = offset32 | (inp_data[i+4] << (8*i));
    for(unsigned i = 0; i < 4; ++i)
        num_codes = num_codes | (inp_data[i+8] << (8*i));

    // Create symbol table 
    // NOTE: do I want to try and get away from std::string?
    std::vector<std::string> table(LZW_ALPHA_SIZE);
    for(unsigned i = 0; i < LZW_ALPHA_SIZE; ++i)
        table[i] += i;

    int bytes_per_code = 2;
    unsigned old_code = 0;
    unsigned new_code = 0;
    uint8_t cc = 0;
    uint8_t cbuf;
    std::string s;

    // Read the first code
    inp_ptr = 12;
    for(int i = 0; i < bytes_per_code; ++i)
    {
        cbuf = inp_data[inp_ptr];
        inp_ptr++;
        old_code = old_code | (cbuf << (8 * i));
    }

    s += old_code;
    out.push_back(old_code);

    while(inp_ptr < inp_length)
    {
        if(inp_ptr == offset24)
            bytes_per_code = 3;
        if(inp_ptr == offset32)
            bytes_per_code = 4;

        new_code = 0;
        for(int i = 0; i < bytes_per_code; ++i)
        {
            cbuf = inp_data[inp_ptr];
            inp_ptr++;
            if(inp_ptr > inp_length)
                break;

            new_code = new_code | (cbuf << (8*i));
        }

        // Add unseen codes to table 
        if(new_code >= table.size())
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
        // write s to output
        for(const auto& e : s)
            out.push_back(e);

        std::string new_sym;
        new_sym += table[old_code];
        new_sym += cc;

        table.push_back(new_sym);
        old_code = new_code;
    }

    return out;
}



/*
 * Alternative encoder that uses an array rather than a map at each node
 */

// Null value for this node is 0xFFFFFFFF
const constexpr uint32_t ARRAY_NODE_UNUSED = 0xFFFFFFFF;

struct ArrayNode
{
    uint32_t value;
    bool leaf;
    std::array<std::unique_ptr<ArrayNode>, LZW_ALPHA_SIZE> children;

    public:
        ArrayNode() : value(0), leaf(false), children() {}
        ArrayNode(uint32_t v, bool l) : value(v), leaf(l), children() {} 
};


/*
 * Encoding but with the array node
 */
std::stringstream lzw_array_encode(const std::string_view data)
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

    std::unique_ptr<ArrayNode> prefix_tree = std::make_unique<ArrayNode>();

    // Init the dict 
    ArrayNode* node = prefix_tree.get();
    for(cur_key = 0; cur_key < LZW_ALPHA_SIZE; ++cur_key)
        node->children[cur_key] = std::make_unique<ArrayNode>(cur_key, true);

    for(const char& c : data)
    {
         //auto& children = node->children;
         if(!node->children[uint8_t(c)].get())         // what is it about these -128 values that is such a problem?
         {
             out.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);
             node->children[uint8_t(c)] = std::make_unique<ArrayNode>(cur_key, true);
             cur_key++;

             node = prefix_tree->children[uint8_t(c)].get();

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
             node = node->children[uint8_t(c)].get();
    }

    out.write(reinterpret_cast<const char*>(&node->value), bytes_per_code);
    // write offsets 
    out.seekp(0, std::ios::beg);
    out.write(reinterpret_cast<const char*>(&offset24), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&offset32), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&cur_key), sizeof(uint32_t));

    return out;
}
