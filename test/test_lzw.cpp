// Unit tests for LZW


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>

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


TEST_CASE("test_segfault", "lzw")
{
    std::cout << "TIME TO ENCODE THE BARD" << std::endl;
    std::string test_filename = "test/shakespear.txt";
    std::ifstream file(test_filename);
    std::string text(std::istreambuf_iterator<char>{file}, {});
    file.close();
    std::cout << "read " << text.size() << " characters from [" << test_filename << "]" << std::endl;


    std::stringstream enc_out = lzw_encode(text);

    std::cout << enc_out.str() << std::endl;
    std::cout << "enc_out.str().size(): " << enc_out.str().size() << std::endl;

    REQUIRE(enc_out.str().size() < text.size());

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



TEST_CASE("test_lzw_encoder_encode", "lzw")
{
    LZWEncoder lzw;
    unsigned header_size = 12;

    std::vector<uint16_t> exp_data = {98, 97, 256, 257, 97, 260};
    std::string test_input = "babaabaaa"; // exp sequence: 98, 97, 256, 257, 97, 260

    lzw.encode(test_input);
    auto out_data = lzw.get();

    // We need to format this into a new vector since the std::string doesn't know about
    // the alignment of the stream.
    std::vector<uint16_t> out_data_vec;
    char buf[2];
    for(unsigned i = header_size; i < out_data.size(); ++i)
    {
        buf[i % 2] = out_data[i];
        if(i % 2)
            out_data_vec.push_back(buf[1] << 8 | buf[0]);
    }


    // Note that first 12 bytes of this are header information

    REQUIRE(out_data_vec.size() == exp_data.size());
    for(unsigned i = 0; i < out_data_vec.size(); ++i)
        REQUIRE(out_data_vec[i] == exp_data[i]);
}

// TODO: test encode in loop
TEST_CASE("test_lzw_encoder_large", "lzw")
{
    std::string test_filename = "test/shakespear.txt";
    std::ifstream file(test_filename);
    std::string text;


    file >> text;
    file.close();

}


TEST_CASE("test_lzw_decoder_decode", "lzw")
{
    LZWDecoder lzw;

    std::vector<uint16_t> inp_data = {98, 97, 256, 257, 97, 260};
    std::stringstream input;

    // Header 
    uint32_t t = 0;
    for(unsigned i = 0; i < 3; ++i)
        input.write(reinterpret_cast<const char*>(&t), sizeof(uint32_t));

    // Stream
    for(unsigned i = 0; i < inp_data.size(); ++i)
        input.write(reinterpret_cast<const char*>(&inp_data[i]), sizeof(uint16_t));

    lzw.decode(input);
    auto dec_out_str = lzw.get();

    std::string exp_out_str = "babaabaaa";

    REQUIRE(exp_out_str.size() == dec_out_str.size());
    REQUIRE(exp_out_str == dec_out_str);
}


// TODO: test decode in loop
