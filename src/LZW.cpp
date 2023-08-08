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
            // To make this fast I need to be not doing this byte-by-byte
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
    // TODO: do I want to try and get away from std::string
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


/*
 * Encoder
 * Object oriented encoder using LZW algorithm. The idea is that you can encode 
 * data larger than "memory" by breaking it into chunks and calling encode() in a loop.
 */

LZWEncoder::LZWEncoder() : cur_key(0), root(std::make_unique<LZWNode>()),
    offset24(0), offset32(0), num_codes(0), bytes_per_code(2), insert_header(true),
    out_ptr(0), inp_ptr(0)
{
    this->init();
}

/*
 * LZW Objects
 */

// LZWEncoder private methods
LZWEncoder::LZWNode* LZWEncoder::insert(const lzw_symbol_t c, LZWNode* node)
{
    auto& children = node->children;
    auto it = children.find(c);
    
    if(it == children.end())
        it = children.emplace(c, std::make_unique<LZWNode>(this->cur_key, true)).first;

    node = it->second.get();
    this->cur_key++;

    return node;
}

LZWEncoder::LZWNode* LZWEncoder::search_node(const lzw_symbol_t c, LZWNode* node) const
{
    auto& children = node->children;
    auto it = children.find(c);
    if(it == children.end())
        return nullptr;

    node = it->second.get();

    return node;
}


void LZWEncoder::clear_dict(void)
{
    this->root->children.clear();
}

/*
 * Encoder public methods
 */

/*
 * init()
 * Reset the state of the encoder
 */
void LZWEncoder::init(void)
{
    this->out.clear();
    this->clear_dict();

    for(uint32_t code = 0; code < LZW_ALPHA_SIZE; ++code)
        this->insert(lzw_symbol_t(code), this->root.get());

    this->offset24 = 0;
    this->offset32 = 0;
    this->num_codes = 0;
    this->bytes_per_code = 2;
    this->insert_header = true;
    this->out_ptr = 0;
    this->inp_ptr = 0;
}


/*
 * contains()
 * Returns true if there is a prefix that matches the sequence in data
 */
//bool LZWEncoder::contains(const std::string_view data) const
//{
//    auto* node = this->root.get();
//
//    for(auto const c: data) 
//    {
//        auto& children = node->children;
//        auto it = children.find(lzw_symbol_t(c));
//        if(it == children.end())
//            return false;
//
//        node = it->second.get();
//    }
//
//    return true;
//}


/*
 * Encode a stream into an LZW stream
 */
void LZWEncoder::encode(const uint8_t* data, unsigned length)
{
    unsigned inp_ptr = 0;
    // On the first call, write the header information to the output stream
    if(this->insert_header)
    {
        this->out_ptr = 12;
        this->out.reserve(length + 12);
        for(unsigned i = 0; i < 12; ++i)
            this->out.push_back(0);     // reserve some memory - this is probably an inefficient way to do this..

        //this->out.write(reinterpret_cast<const char*>(&this->offset24), sizeof(uint32_t));
        //this->out.write(reinterpret_cast<const char*>(&this->offset32), sizeof(uint32_t));
        //this->out.write(reinterpret_cast<const char*>(&this->num_codes), sizeof(uint32_t));
        this->insert_header = false;
        this->cur_node = this->root.get();
    }
    else
        this->out.reserve(this->out.size() + length);

    // Check if we should extend the out vector a bit
    //if((this->out_ptr + length) > this->out.size())
    //    this->out.reserve(this->out.size() + length);
    

    // TODO: out_ptr doesn't really do anything...
    for(inp_ptr = 0; inp_ptr < length; ++inp_ptr)
    {
        uint8_t c = data[inp_ptr];
        auto it = cur_node->children.find(c);

        if(it == cur_node->children.end())
        {
            // Write output
            for(int b = 0; b < this->bytes_per_code; ++b)
            {
                //this->out[this->out_ptr] = (cur_node->value >> (8*b)) & 0xFF;
                this->out.push_back((cur_node->value >> (8*b)) & 0xFF);
                this->out_ptr++;
            }
            this->insert(c, this->cur_node);
            this->cur_node = this->root->children.find(lzw_symbol_t(c))->second.get();

            if(this->cur_key == 0xFFFF)
            {
                this->bytes_per_code = 3;
                this->offset24 = this->out_ptr;
            }
            if(this->cur_key == 0xFFFFFF)
            {
                this->bytes_per_code = 4;
                this->offset32 = this->out_ptr;
            }
        }
        else
            cur_node = it->second.get();
    }

    // Write the last node in the sequence
    for(int b = 0; b < this->bytes_per_code; ++b)
    {
        //this->out[this->out_ptr] = (cur_node->value >> (8*b)) & 0xFF;
        this->out.push_back((cur_node->value >> (8*b)) & 0xFF);
        this->out_ptr++;
    }
    
    //out.write(reinterpret_cast<const char*>(&this->cur_node->value), bytes_per_code);
}


/*
 * get()
 * Get the underlying stream generated by one or more calls to encode()
 */
const std::vector<uint8_t>& LZWEncoder::get(void) 
{
    // Write the header information
    //this->out.seekp(0, std::ios::beg);
    //this->out.write(reinterpret_cast<const char*>(&this->offset24), sizeof(uint32_t));
    //this->out.write(reinterpret_cast<const char*>(&this->offset32), sizeof(uint32_t));
    //this->out.write(reinterpret_cast<const char*>(&this->cur_key), sizeof(uint32_t));
    //this->out.seekp(0, std::ios::end);
    //this->out.seekg(0, std::ios::end);
    //return this->out.str();     // TODO: this is wrong due to string formatting

    // Write header information
    for(int i = 0; i < 4; ++i)
        this->out[i] = (this->offset24 >> (8*i)) & 0xFF;
    for(int i = 0; i < 4; ++i)
        this->out[i+4] = (this->offset32 >> (8*i)) & 0xFF;
    for(int i = 0; i < 4; ++i)
        this->out[i+8] = (this->cur_key >> (8*i)) & 0xFF;

    return out;
}


/*
 * get_stream()
 * Return a new stream with the contents of the current internal stream
 */
//std::stringstream LZWEncoder::get_stream(void)
//{
//    std::stringstream ss;
//    ///this->out.seekp(0, std::ios::beg);
//    ss << this->out.rdbuf();
//    return ss;
//}


/*
 * to_file()
 */
//void LZWEncoder::to_file(const std::string& filename)
//{
//    std::ofstream file(filename, std::ios::binary);
//    file << this->out.rdbuf();
//    file.close();
//}

/*
 * Counts number of nodes in prefix tree
 */
unsigned LZWEncoder::tree_size(void) const
{
    std::stack<LZWNode*> s;
    unsigned count = 0;

    s.push(this->root.get());
    while(!s.empty())
    {
        LZWNode* node = s.top();
        s.pop();
        count++;

        if(node->children.size() > 0)
        {
            for(auto it = node->children.begin(); it != node->children.end(); ++it)
                s.push(it->second.get());
        }
    }
    count--;            // don't count the root node

    return count;
}


unsigned LZWEncoder::length(void) const
{
    return this->out.size();
}


/*
 * LZWDecoder
 * Object oriented decoder using LZW algorithm. The idea is that you can decode 
 * data larger than "memory" by breaking it into chunks and calling decode() in a loop.
 */
LZWDecoder::LZWDecoder() : table(LZW_ALPHA_SIZE), offset24(0), offset32(0), 
    num_codes(0), bytes_per_code(2), read_header(false), 
    inp_ptr(0), out_ptr(0)
{
    this->init();
}


void LZWDecoder::init(void)
{
    if(this->table.size() > LZW_ALPHA_SIZE)
        this->table.resize(LZW_ALPHA_SIZE);
    else
    {
        for(unsigned i = 0; i < LZW_ALPHA_SIZE; ++i)
            this->table[i] += i;
    }
    
    // reset stream info
    this->out.clear();
    this->offset24 = 0;
    this->offset32 = 0;
    this->num_codes = 0;
    this->bytes_per_code = 2;
    this->read_header = true;
    this->inp_ptr = 0;
    this->out_ptr = 0;
}

/*
 * decode()
 * Run decode on some chunk of the stream, updating the output and table.
 */
void LZWDecoder::decode(const uint8_t* data, unsigned length) 
{
    uint32_t offset24 = 0;
    uint32_t offset32 = 0;
    uint32_t num_codes = 0;

    unsigned inp_ptr = 0;

    if(this->read_header)
    {
        for(unsigned i = 0; i < 4; ++i)
            offset24 = offset24 | (data[i] << (8*i));
        for(unsigned i = 0; i < 4; ++i)
            offset32 = offset32 | (data[i+4] << (8*i));
        for(unsigned i = 0; i < 4; ++i)
            num_codes = num_codes | (data[i+8] << (8*i));

        inp_ptr = 12;
        this->inp_ptr = inp_ptr;   // having two pointers kind of sucks, 
                                   // TODO: at least change the name...
        this->read_header = false;
    }

    // Decode loop
    std::string s;
    unsigned old_code = 0;
    unsigned new_code = 0;
    char cc = 0;
    uint8_t cbuf;

    // Read first code and write directly to output
    for(int i = 0; i < this->bytes_per_code; ++i)
    {
        cbuf = data[inp_ptr];
        inp_ptr++; this->inp_ptr++;
        old_code = old_code | (cbuf << (8 * i));
    }

    s += old_code;
    out.push_back(old_code);

    while(inp_ptr < length)
    {
        // Check what the code size should be 
        if(this->inp_ptr == offset24)
            this->bytes_per_code = 3;

        if(this->inp_ptr == offset32)
            this->bytes_per_code = 4;

        new_code = 0;
        for(int i = 0; i < bytes_per_code; ++i)
        {
            cbuf = data[inp_ptr];
            inp_ptr++;
            this->inp_ptr++;        // this is a shit design...
            if(inp_ptr > length)
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

        this->table.push_back(new_sym);
        old_code = new_code;
    }
}


const std::vector<uint8_t>& LZWDecoder::get(void) const
{
    return this->out;
}

/*
 * get()
 * Get the underlying stream from a decoder
 */
//std::string LZWDecoder::get(void)
//{
//    return this->out.str();
//}

/*
 * get_stream()
 * Return a new stream with the contents of the current internal stream
 */
//std::stringstream LZWDecoder::get_stream(void)
//{
//    std::stringstream ss;
//    ///this->out.seekp(0, std::ios::beg);
//    ss << this->out.rdbuf();
//    return ss;
//}

/*
 * to_file()
 */
//void LZWDecoder::to_file(const std::string& filename)
//{
//    std::ofstream file(filename, std::ios::binary);
//    file << this->out.rdbuf();
//    file.close();
//}


unsigned LZWDecoder::size(void) const
{
    return this->table.size();
}

