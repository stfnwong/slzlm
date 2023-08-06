// LSZZ Tests

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

// TODO: get rid of this
#include <iostream>
#include <iomanip>

//#include <fstream>
#include <vector>

#include "LZSS.hpp"
#include "Util.hpp"




/*
 * Bitstream
 */
TEST_CASE("test_stringbitstream_write_bit", "lzss")
{
    std::stringstream ss;
    StringBitStream test_stream(ss);

    unsigned num_bits = 32;
    for(unsigned i = 0; i < num_bits; ++i)
        test_stream.write_bit(i % 2 == 0);            // Should be all 0xAA on intel (not portable)

    unsigned exp_length = num_bits / 8;
    unsigned str_length = test_stream.length();
    REQUIRE(str_length == exp_length);

    std::vector<uint8_t> str_vec = stream_to_vec<uint8_t>(test_stream.ss);
    REQUIRE(str_vec.size() == exp_length);

    for(unsigned i = 0; i < str_vec.size(); ++i)
        REQUIRE(str_vec[i] == 0xAA);
}


TEST_CASE("test_stringbitstream_write_bit_pattern", "lzss")
{
    std::stringstream ss;
    StringBitStream test_stream(ss);

    // Most significant to least significant
    std::vector<int> test_bit_pattern = {
        1, 0, 0, 0, 1, 0, 0, 0
    };

    for(unsigned i = 0; i < test_bit_pattern.size(); ++i)
        test_stream.write_bit(test_bit_pattern[i]); 

    unsigned exp_length = 1;
    REQUIRE(test_stream.length() == exp_length);
    std::vector<uint8_t> str_vec = stream_to_vec<uint8_t>(test_stream.ss);
    REQUIRE(str_vec.size() == exp_length);

    //std::cout << "str[0]: 0x" << std::hex << std::setw(2) << unsigned(str_vec[0]) << std::endl;

    REQUIRE(str_vec[0] == 0x88);
}


TEST_CASE("test_stringbitstream_write_bits", "lzss")
{
    std::stringstream ss;
    StringBitStream test_stream(ss);
    uint32_t code = 0xFFFF8080;

    // Write 8 bits of this code
    test_stream.write_bits(code, 8);
    REQUIRE(test_stream.length() == 1);

    //test_stream.to_file("str8.out");

    std::vector<uint8_t> str_vec;
    str_vec = stream_to_vec<uint8_t>(test_stream.ss);
    REQUIRE(str_vec.size() == 1);
    for(unsigned i = 0; i < str_vec.size(); ++i)
        REQUIRE(str_vec[i] == 0x80);


    // Reset stream and write 16 codes
    test_stream.init();
    REQUIRE(test_stream.length() == 0);

    test_stream.ss.seekp(0, std::ios::beg);
    test_stream.ss.seekg(0, std::ios::beg);
    test_stream.write_bits(code, 16);

    //test_stream.to_file("str16.out");

    REQUIRE(test_stream.length() == 2);
    str_vec = stream_to_vec<uint8_t>(test_stream.ss);
    REQUIRE(str_vec.size() == 2);

    for(unsigned i = 0; i < str_vec.size(); ++i)
        REQUIRE(str_vec[i] == 0x80);

    test_stream.init();
    REQUIRE(test_stream.length() == 0);

    test_stream.write_bits(code, 32);
    //test_stream.to_file("str32.out");
    str_vec = stream_to_vec<uint8_t>(test_stream.ss);
    REQUIRE(str_vec.size() == 4);

    // Note: from most significant to least significant
    std::vector<uint8_t> exp_vec = {0xFF, 0xFF, 0x80, 0x80};

    for(unsigned i = 0; i < str_vec.size(); ++i)
        REQUIRE(str_vec[i] == exp_vec[i]);
}


TEST_CASE("test_stringbitstream_read_bit", "lszz")
{
    // Test reading single bits 

    std::stringstream ss;
    StringBitStream test_stream(ss);

    unsigned num_bits = 32;
    for(unsigned i = 0; i < num_bits; ++i)
        test_stream.write_bit(i % 2 == 0);

    // Bitstream handles the underlying stream, we can just consume one bit at a time
    REQUIRE(test_stream.read_bit() == 1);
    REQUIRE(test_stream.read_bit() == 0);
    REQUIRE(test_stream.read_bit() == 1);
    REQUIRE(test_stream.read_bit() == 0);

    // etc
    // If we read the next 4 bits the pattern should continue
    uint32_t more_bits;
    more_bits = test_stream.read_bits(4);
    REQUIRE(more_bits == 0xA);

    // There are 3 more bytes
    more_bits = test_stream.read_bits(12);
    REQUIRE(more_bits == 0xAAA);
}


// Vector bitstream 
// These should get re-implemented as subclasses of some abstract StringBitStream class
TEST_CASE("test_vector_bitstream_write_bits", "lzss")
{
    //VectorBitStream test_stream;
    VectorBitStream test_stream;
    uint32_t code = 0xFFFF8080;
    uint32_t out_word;

    // Write 8 bits of this code
    test_stream.write_bits(code, 8);
    REQUIRE(test_stream.length() == 1);

    //test_stream.to_file("str8.out");
    out_word = test_stream.read_bits(8);
    REQUIRE(out_word == 0x80);

    // Reset stream and write 16 codes
    test_stream.init();
    REQUIRE(test_stream.length() == 0);

    test_stream.write_bits(code, 16);
    REQUIRE(test_stream.length() == 2);

    for(unsigned i = 0; i < test_stream.length(); ++i)
        REQUIRE(test_stream.data[i] == 0x80);

    test_stream.init();
    REQUIRE(test_stream.length() == 0);

    test_stream.write_bits(code, 32);

    // Note: from most significant to least significant
    std::vector<uint8_t> exp_vec = {0xFF, 0xFF, 0x80, 0x80};

    REQUIRE(test_stream.length() == exp_vec.size());

    for(unsigned i = 0; i < test_stream.length(); ++i)
        REQUIRE(test_stream.data[i] == exp_vec[i]);
}




/*
 * Tree manipulation
 */
//TEST_CASE("test_lzss_tree", "lzss")
//{
//    std::string test_filename = "test/shakespear.txt";
//    std::ifstream file(test_filename);
//    std::string text(std::istreambuf_iterator<char>{file}, {});
//    file.close();
//
//    LZSSTree tree;
//    LZSSWindow window;
//
//    // preload window 
//    unsigned inp_pos = 0;
//    unsigned cur_pos = 1;
//    for(inp_pos = 0; inp_pos < WINDOW_SIZE; ++inp_pos)
//        window[inp_pos + cur_pos] = text[inp_pos];
//}





/*
 * Encoding and Decoding
 */
TEST_CASE("test_lzss_encode", "lzss")
{
    // TODO: walk through the algorithm step by step and create a more detailed test case
    const std::string input = "The Cruelty of Really Tea";

    std::stringstream enc_out = lzss_encode(input);
    enc_out.seekg(0, std::ios::end);
    enc_out.seekg(0, std::ios::beg);

    std::vector<uint8_t> enc_byte_vec = stream_to_vec<uint8_t>(enc_out);

    REQUIRE(enc_byte_vec.size() < input.size());
}


//TEST_CASE("test_lzss_decode", "lzss")
//{
//    const std::string input = "The Cruelty of Really Tea";
//
//    // Encode a string 
//    std::stringstream enc_out = lzss_encode(input);
//
//    std::stringstream dec_out = lzss_decode(enc_out);
//
//
//}
