// LSZZ Tests

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

// TODO: get rid of this
#include <iostream>
#include <iomanip>

#include <fstream>
#include <vector>

#include "LZSS.hpp"
#include "Util.hpp"



TEST_CASE("test_bitstream_add_bit", "lzss")
{
    std::stringstream ss;
    BitStream test_stream(ss);

    unsigned num_bits = 32;
    for(unsigned i = 0; i < num_bits; ++i)
        test_stream.add_bit(i % 2 == 0);            // Should be all 0xAA on intel (not portable)

    unsigned exp_length = num_bits / 8;
    unsigned str_length = test_stream.length();
    REQUIRE(str_length == exp_length);

    std::vector<uint8_t> str_vec = stream_to_vec<uint8_t>(test_stream.ss);
    REQUIRE(str_vec.size() == exp_length);

    for(unsigned i = 0; i < str_vec.size(); ++i)
        REQUIRE(str_vec[i] == 0xAA);
}


TEST_CASE("test_bitstream_add_bits", "lzss")
{
    std::stringstream ss;
    BitStream test_stream(ss);
    uint32_t code = 0x8080;
    //uint32_t code = 0x80808080;

    // Write 8 bits of this code
    test_stream.add_bits(code, 8);
    std::cout << "Stream length : " << test_stream.length() << std::endl;


    std::vector<uint8_t> str_vec;
    str_vec = stream_to_vec<uint8_t>(test_stream.ss);

    for(unsigned i = 0; i < str_vec.size(); ++i)
        std::cout << std::dec << "[" << i << "]: 0x" << std::setw(2) << std::hex << unsigned(str_vec[i]) << std::endl;


    // Reset stream and write 16 codes
    test_stream.init();
    REQUIRE(test_stream.length() == 0);
    test_stream.ss.seekp(0, std::ios::beg);
    test_stream.ss.seekg(0, std::ios::beg);
    test_stream.add_bits(code, 16);

    std::cout << "Stream length : " << test_stream.length() << std::endl;

    str_vec = stream_to_vec<uint8_t>(test_stream.ss);
    for(unsigned i = 0; i < str_vec.size(); ++i)
        std::cout << std::dec << "[" << i << "]: 0x" << std::setw(2) << std::hex << unsigned(str_vec[i]) << std::endl;


}


//TEST_CASE("test_lzss_encode", "lzss")
//{
//    const std::string input = "babaabaaa";
//
//    auto enc_out = lzss_encode(input);
//
//    std::cout << "enc_out: " << enc_out.str() << std::endl;
//}
