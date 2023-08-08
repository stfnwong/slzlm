// Unit tests for LZW


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <iterator>

#include "LZW.hpp"
#include "Util.hpp"     // for the stream -> vec function


// The stream version of the compressed data for the sting "babaabaaa"
std::stringstream generate_exp_stream_data(void)
{
    std::vector<uint16_t> inp_data = {98, 97, 256, 257, 97, 260};
    std::stringstream ss;

    // Header 
    uint32_t t = 0;
    for(unsigned i = 0; i < 3; ++i)
        ss.write(reinterpret_cast<const char*>(&t), sizeof(uint32_t));

    // Stream
    for(unsigned i = 0; i < inp_data.size(); ++i)
        ss.write(reinterpret_cast<const char*>(&inp_data[i]), sizeof(uint16_t));

    return ss;
}




TEST_CASE("test_lzw_encode", "lzw")
{
    // Init some c-style arrays...

    unsigned input_buf_len = 9;
    unsigned header_len = 12;
    uint8_t input_buf[input_buf_len] = {0x62, 0x61, 0x62, 0x61, 0x61, 0x62, 0x61, 0x61, 0x61};
    uint8_t* output_buf = new uint8_t[header_len + input_buf_len];  // length of input buf
    //std::vector<uint16_t> exp_data = {98, 97, 256, 257, 97, 260};

    std::vector<uint8_t> exp_data = {
        0x00, 0x00, 0x00, 0x00,     // offset24
        0x00, 0x00, 0x00, 0x00,     // offset32
        0x05, 0x01, 0x00, 0x00,     // num_codes
        0x62, 0x00, 0x61, 0x00,
        0x00, 0x01, 0x01, 0x01,
        0x61, 0x00, 0x04, 0x01
    };

    unsigned out_len = lzw_encode(input_buf, input_buf_len, output_buf);

    REQUIRE(out_len == exp_data.size());
    for(unsigned i = 0; i < out_len; ++i)
        REQUIRE(exp_data[i] == output_buf[i]);

    delete[] output_buf;
}


TEST_CASE("test_lzw_decode", "lzw")
{
    // Create some input data
    unsigned input_buf_len = 9;
    unsigned header_len = 12;
    uint8_t input_buf[input_buf_len] = {0x62, 0x61, 0x62, 0x61, 0x61, 0x62, 0x61, 0x61, 0x61};
    uint8_t* output_buf = new uint8_t[header_len + input_buf_len];  // length of input buf


    unsigned out_len = lzw_encode(input_buf, input_buf_len, output_buf);

    std::vector<uint8_t> decoded = lzw_decode(output_buf, out_len);

    REQUIRE(decoded.size() == input_buf_len);

    delete[] output_buf;
}



// ===== Functional encoder ===== //

//TEST_CASE("test_encode_long_string", "lzw")
//{
//    std::string test_filename = "test/shakespear.txt";
//    std::ifstream file(test_filename);
//    std::string text(std::istreambuf_iterator<char>{file}, {});
//    file.close();
//
//    std::stringstream enc_out = lzw_encode(text);
//
//    REQUIRE(enc_out.str().size() < text.size());
//    //std::cout << "enc_out: " << enc_out.str() << std::endl << std::endl;
//    std::cout << "text length    : " <<  text.size() << " characters" << std::endl;
//    std::cout << "encoded length : " << enc_out.str().size() << " characters" << std::endl;
//    float r = float(enc_out.str().size()) / float(text.size());
//    std::cout << "Ratio : " << r << std::endl;
//    REQUIRE(r < 1.0);
//}




/*
 * Array node encoder
 */
TEST_CASE("test_array_node_encoder", "lzw")
{
    const std::string test_data = "babaabaaa";
    std::stringstream enc_out = lzw_array_encode(test_data);

    enc_out.seekg(3 * sizeof(uint32_t), std::ios::beg);   // skip header
    std::vector<uint16_t> stream_vec = stream_to_vec<uint16_t>(enc_out);
    std::vector<uint16_t> exp_data = {98, 97, 256, 257, 97, 260};

    REQUIRE(exp_data.size() == stream_vec.size());
    for(unsigned i = 0; i < stream_vec.size(); ++i)
        REQUIRE(exp_data[i] == stream_vec[i]);
}



TEST_CASE("test_array_encoder_long_string", "lzw")
{
    std::string test_filename = "test/shakespear.txt";
    std::ifstream file(test_filename);
    std::string text(std::istreambuf_iterator<char>{file}, {});
    file.close();

    std::stringstream enc_out = lzw_array_encode(text);

    REQUIRE(enc_out.str().size() < text.size());

    std::cout << "text length    : " <<  text.size() << " characters" << std::endl;
    std::cout << "encoded length : " << enc_out.str().size() << " characters" << std::endl;

    float r = float(enc_out.str().size()) / float(text.size());
    std::cout << "Ratio : " << r << std::endl;

    REQUIRE(r < 1.0);
}
