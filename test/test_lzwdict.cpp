// Unit tests for LZW


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

#include "LZW.hpp"
#include "Util.hpp"     // for the stream -> vec function



TEST_CASE("test_function_encode", "lzw")
{
    const std::string test_data = "babaabaaa";
    std::stringstream enc_out = lzw_encode(test_data);

    std::vector<uint16_t> exp_data = {98, 97, 256, 257, 97, 260};
    enc_out.seekg(3 * sizeof(uint32_t), std::ios::beg);   // skip over the header 
    std::vector<uint16_t> stream_vec = stream_to_vec<uint16_t>(enc_out);

    REQUIRE(exp_data.size() == stream_vec.size());
    for(unsigned i = 0; i < stream_vec.size(); ++i)
        REQUIRE(exp_data[i] == stream_vec[i]);
}


//TEST_CASE("test_clear_dict", "lzw")
//{
//    LZWDict lzw;
//}



TEST_CASE("test_lzw_dict_encode", "lzw")
{
    LZWDict lzw;

    std::string test_input = "babaabaaa"; // exp sequence: 98, 97, 256, 257, 97, 260
    std::stringstream ss;
    ss << test_input;
    // TODO: split this 
    auto enc = lzw.encode(ss);

    std::vector<uint16_t> exp_data = {98, 97, 256, 257, 97, 260};

    // skip over header and read 
    enc.seekg(3 * sizeof(uint32_t), std::ios::beg);
    std::vector<uint16_t> out_data = stream_to_vec<uint16_t>(enc);

    enc.seekg(0);

    REQUIRE(out_data.size() == exp_data.size());
    for(unsigned i = 0; i < out_data.size(); ++i)
        REQUIRE(out_data[i] == exp_data[i]);
}




TEST_CASE("test_fake_decode", "lzw")
{
    LZWDict lzw;

    std::vector<uint32_t> test_data = {98, 97, 256, 257, 97, 260};
    std::stringstream out = lzw.decode(test_data);

    std::string str_out = out.str();
    std::vector<uint16_t> out_vec = stream_to_vec<uint16_t>(out);

    for(unsigned i = 0; i < out_vec.size(); ++i)
        std::cout << "[" << i << "] : " << out_vec[i] << std::endl;
    
}

