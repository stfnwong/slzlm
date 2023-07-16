// Unit tests for LZW


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <iostream>

// TODO: change this name
#include "comp.hpp"


TEST_CASE("test_construct", "lzw")
{
    LZWDict lzw;

    const std::string input_text = "fuck your couch charlie murphy";

    std::vector<uint32_t> out = lzw.construct(input_text);

    std::cout << "input len: " << input_text.size() << ", out len: " << out.size() << std::endl;

    for(unsigned i = 0; i < out.size(); ++i)
        std::cout << "[" << i << "]: " << out[i] << std::endl;



}
