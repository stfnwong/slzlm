// Test utilities


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"


#include "Util.hpp"


TEST_CASE("test_stream_to_vec", "util")
{
    std::stringstream ss;
    std::vector<uint8_t> inp_vec = {0, 1, 2, 3};

    ss = vec_to_stream<uint8_t>(inp_vec);
    ss.seekg(0, std::ios::end);
    size_t stream_len = ss.tellg();

    REQUIRE(stream_len == inp_vec.size());
}



TEST_CASE("test_vec_to_stream", "util")
{
    // make a stream
    std::stringstream ss;

    std::vector<uint16_t> inp_vec = {98, 97, 256, 257, 97, 260};

    // Stream
    for(unsigned i = 0; i < inp_vec.size(); ++i)
        ss.write(reinterpret_cast<const char*>(&inp_vec[i]), sizeof(uint16_t));

    std::vector<uint16_t> out_vec = stream_to_vec<uint16_t>(ss);
    REQUIRE(inp_vec.size() == out_vec.size());
    
    for(unsigned i = 0; i < out_vec.size(); ++i)
        REQUIRE(out_vec[i] == inp_vec[i]);
}
