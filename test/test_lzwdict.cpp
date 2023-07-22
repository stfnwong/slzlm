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

    enc_out.seekg(3 * sizeof(uint32_t), std::ios::beg);   // skip header
    std::vector<uint16_t> stream_vec = stream_to_vec<uint16_t>(enc_out);
    std::vector<uint16_t> exp_data = {98, 97, 256, 257, 97, 260};

    REQUIRE(exp_data.size() == stream_vec.size());
    for(unsigned i = 0; i < stream_vec.size(); ++i)
        REQUIRE(exp_data[i] == stream_vec[i]);
}


TEST_CASE("test_function_decode", "lzw")
{
    std::vector<uint16_t> inp_data = {98, 97, 256, 257, 97, 260};
    std::stringstream input;

    // Header 
    uint32_t t = 0;
    for(unsigned i = 0; i < 3; ++i)
        input.write(reinterpret_cast<const char*>(&t), sizeof(uint32_t));

    // Stream
    for(unsigned i = 0; i < inp_data.size(); ++i)
        input.write(reinterpret_cast<const char*>(&inp_data[i]), sizeof(uint16_t));


    auto dec_out = lzw_decode(input);

    std::string exp_out_str = "babaabaaa";
    std::string dec_out_str = dec_out.str();

    REQUIRE(exp_out_str.size() == dec_out_str.size());
    REQUIRE(exp_out_str == dec_out_str);
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

    auto enc = lzw.encode(ss);

    enc.seekg(3 * sizeof(uint32_t), std::ios::beg);     // skip header
    std::vector<uint16_t> out_data = stream_to_vec<uint16_t>(enc);
    std::vector<uint16_t> exp_data = {98, 97, 256, 257, 97, 260};

    REQUIRE(out_data.size() == exp_data.size());
    for(unsigned i = 0; i < out_data.size(); ++i)
        REQUIRE(out_data[i] == exp_data[i]);
}



TEST_CASE("test_lzw_dict_decode", "lzw")
{
    LZWDict lzw;

    std::vector<uint16_t> inp_data = {98, 97, 256, 257, 97, 260};
    std::stringstream input;

    // Header 
    uint32_t t = 0;
    for(unsigned i = 0; i < 3; ++i)
        input.write(reinterpret_cast<const char*>(&t), sizeof(uint32_t));

    // Stream
    for(unsigned i = 0; i < inp_data.size(); ++i)
        input.write(reinterpret_cast<const char*>(&inp_data[i]), sizeof(uint16_t));

    auto dec_out = lzw.decode(input);

    std::string exp_out_str = "babaabaaa";
    std::string dec_out_str = dec_out.str();

    REQUIRE(exp_out_str.size() == dec_out_str.size());
    REQUIRE(exp_out_str == dec_out_str);
}
