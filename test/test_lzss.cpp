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



//TEST_CASE("test_lszz_bitstream", "lzss")
//{
//    std::stringstream ss;
//    BitStream test_stream(ss);
//
//    // Write stuff to bitstream
//    const std::string test_filename = "test/shakespear.txt";
//    std::ifstream file(test_filename, std::ios::binary);
//    // TODO: how to read only N characters?
//    std::string text(std::istreambuf_iterator<char>{file}, {});
//}


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
    {
        // TODO: remove print, also something wrong with first byte...
        std::cout << std::dec << "[" << i << "] : 0x" << std::setw(2) << std::hex << unsigned(str_vec[i]) << std::endl;
        REQUIRE(str_vec[i] == 0xAA);
    }
}



//TEST_CASE("test_lzss_encode", "lzss")
//{
//    const std::string input = "babaabaaa";
//
//    auto enc_out = lzss_encode(input);
//
//    std::cout << "enc_out: " << enc_out.str() << std::endl;
//}
