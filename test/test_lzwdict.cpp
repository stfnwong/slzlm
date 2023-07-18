// Unit tests for LZW


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <iostream>

// TODO: change this name
#include "comp.hpp"


TEST_CASE("test_encode", "lzw")
{
    LZWDict lzw;

    //const std::string input_text = "fuck your couch charlie murphy";
    const std::string input_text = "babaabaaa";

    std::vector<uint32_t> out = lzw.encode(input_text);

    std::cout << "input len: " << input_text.size() << ", out len: " << out.size() << std::endl;

    for(unsigned i = 0; i < out.size(); ++i)
        std::cout << "[" << i << "]: " << out[i] << std::endl;

    std::vector<std::string> search_test = {
        "ba",
        "ab",
        "baa"
    };

    //std::vector<std::string> search_test = {
    //    "fuck",
    //    "your",
    //    "couch",
    //    "norm macdonald"
    //};

    for(const auto& s : search_test)
        std::cout << "[" << s << "] " << (lzw.contains(s) ? " found" : "not found") << std::endl;

}

TEST_CASE("test_insert_same_string", "lzw")
{
    LZWDict lzw;
    const std::string test_input = "aaaaab";
    auto result = lzw.encode(test_input);

    std::vector<std::string> search = {
        "a",
        "aa",
        "aaa",
        "b"
    };

    // {inp, code}
    //std::vector<std::pair<std::string, uint32_t>> str_code_pairs = {
    //    {"a", 97},
    //    {"b", 98},
    //    {"aa", 256},
    //    {"aaa", 257},
    //    {"baa", 258},
    //};

    //for(const auto& p: str_code_pairs)
    //    REQUIRE(lzw.get_code(p.first) == p.second);

    //for(const auto& s: search)
    //    std::cout << "[" << s << "]: " << lzw.get_code(s) << std::endl;

    //std::cout << "Encoding results:" << std::endl;
    //for(unsigned i = 0; i < result.size(); ++i)
    //    std::cout << result[i] << " ";
    //std::cout << std::endl;

}


TEST_CASE("test_reverse_char", "lzw")
{
    LZWDict lzw;

    std::string test_input = "babaabaaa"; // exp sequence: 98, 907, 256, 257, 97, 260
    auto enc = lzw.encode(test_input);

    std::cout << "Encoded to: ";
    for(unsigned i = 0; i < enc.size(); ++i)
        std::cout << enc[i] << " ";
    std::cout << std::endl;

    std::vector<std::string> words = {
        "a",    // 97
        "b",    // 98
        "ba",   // 256
        "ab",   // 97, 98 (doesn't compress? should be 257)
        "baa",  // 256, 97
        "aba",
        "aa",
        "bb"
    };

    for(auto const& s: words)
    {
        auto codes = lzw.get_code(s);
        std::cout << "Codes [" << s << "]: ";
        for(unsigned i = 0; i < codes.size(); ++i)
            std::cout << codes[i] << " ";
        std::cout << std::endl;
    }

}



// Mainly for debugging
TEST_CASE("test_encode_method", "lzw")
{
    LZWDict lzw;

    std::string test_input = "babaabaaa";
    lzw.encode(test_input);

    auto traversals = lzw.find_all();
    std::cout << "Found " << traversals.size() << " traversals" 
        << " for input [" << test_input << "]" << std::endl;

    for(unsigned t = 0; t < traversals.size(); ++t)
    {
        if(traversals[t].size() == 1)
            continue;

        std::cout << "Traversal " << t << ": [" ;
        for(unsigned n = 0; n < traversals[t].size(); ++n)
            std::cout << traversals[t][n] << " ";
        std::cout << "]" << std::endl;
    }
}
