// Unit tests for LZW


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

#include "LZW.hpp"


TEST_CASE("test_encode_string_1", "lzw")
{
    LZWDict lzw;
    const std::string test_input = "aaaaab";
    auto result = lzw.encode(test_input);

    std::vector<std::string> words = {
        "a",
        "b",
        "aa",
        "aaa",
        "baa",
    };

    // {inp, code}
    using pair_t = std::pair<std::string, std::vector<lzw_symbol_t>>;
    std::vector<pair_t> str_code_pairs = {
        {"a", {97}},
        {"b", {98}},
        {"aa", {256}},
        {"aaa", {257}},
        {"baa", {98, 256}},
    };

    for(unsigned w = 0; w < words.size(); ++w)
    {
        auto codes = lzw.get_code(str_code_pairs[w].first);
        REQUIRE(codes.size() == str_code_pairs[w].second.size());

        for(unsigned i = 0; i < codes.size(); ++i)
            REQUIRE(codes[i] == str_code_pairs[w].second[i]);
    }
}


TEST_CASE("test_encode_string_2", "lzw")
{
    LZWDict lzw;

    std::string test_input = "babaabaaa"; // exp sequence: 98, 97, 256, 257, 97, 260
    auto enc = lzw.encode(test_input);

    std::cout << "Encoded to: ";
    for(unsigned i = 0; i < enc.size(); ++i)
        std::cout << enc[i] << " ";
    std::cout << std::endl;

    std::vector<std::string> words = {
        "a",    // 97
        "b",    // 98
        "ba",   // 256
        "ab",   // 257
        "baa",  // 258
        "aba",  // 259
        "aa",   // 260
        "bb",   // 98, 98
        "babaa", // 256, 258
    };

    std::vector<std::vector<lzw_symbol_t>> exp_codes = {
        {97},       // a
        {98},       // b
        {256},      // ba 
        {257},      // ab
        {258},      // baa
        {259},      // aba
        {260},      // aa
        {98, 98},   // bb
        {256, 258}, // babaa
    };

    REQUIRE(words.size() == exp_codes.size());

    for(unsigned w = 0; w < words.size(); ++w)
    {
        auto codes = lzw.get_code(words[w]);
        REQUIRE(codes.size() == exp_codes[w].size());
        for(unsigned i = 0; i < codes.size(); ++i)
            REQUIRE(codes[i] == exp_codes[w][i]);
    }
}



TEST_CASE("test_encode_to_file", "lzw")
{
    LZWDict lzw;

    const std::string input = "babaabaaa";
    const std::string test_filename = "ba.lzw";

    std::cout << "Encoding to file [" << test_filename << "]" << std::endl;

    lzw.encode_to_file(test_filename, input);
}


TEST_CASE("test_stringstream", "lzw")
{

    unsigned test_data_len = 256;
    std::vector<uint8_t> sample_data;
    for(unsigned i = 0; i < test_data_len; ++i)
        sample_data.push_back(uint8_t(i));

    std::ostringstream oss;
    for(unsigned i = 0; i < test_data_len; ++i)
        oss.write(reinterpret_cast<const char*>(&sample_data[i]), sizeof(uint8_t));

    // When I print this it should look like garbage 
    std::cout << "test string is [" << oss.str() << "]" << std::endl;

    const std::string test_filename = "sstring.test";

    std::ofstream file(test_filename, std::ios::binary);
    //file.write((const char*) oss.rdbuf(), oss.tellp() * sizeof(const char));
    //file.write((const char*) &sample_data[0], sample_data.size() * sizeof(uint8_t));
    file << oss.str();
    file.close();



}


TEST_CASE("test_encode2", "lzw")
{
    unsigned test_data_size = 256;
    std::stringstream test_data;

    test_data << "babaabaaa";
    
    //for(unsigned i = 0; i < test_data_size; ++i)
    //    test_data.write(reinterpret_cast<const char*>(&i), sizeof(const char));

    LZWDict lzw;
    auto enc_out = lzw.encode2(test_data);
    // write this to some random file
    const std::string test_filename = "encode2.test";
    std::ofstream file(test_filename, std::ios::binary);
    file << enc_out.rdbuf();

    enc_out.seekp(0, std::ios::beg);
    
    std::cout << "enc_out:" << enc_out.str() << std::endl;
    std::cout << "rdbuf:" << enc_out.rdbuf() << std::endl;

    //enc_out.seekp(0, std::ios::end);
    //size_t buf_size = enc_out.tellp();
    //enc_out.seekp(0, std::ios::beg);
    //file.write(enc_out.rdbuf(), buf_size);
    
    //for(const uint8_t b : enc_out.rdbuf())
    //    file.write(reinterpret_cast<const char*>(&b), sizeof(const char));

    file.close();
}



TEST_CASE("test_deez_nuts", "lzw")
{
    LZWDict lzw;

    std::string test_input = "babaabaaa"; // exp sequence: 98, 97, 256, 257, 97, 260
    std::stringstream ss;
    ss << test_input;
    auto enc = lzw.encode2(ss);


    // can't seek in stringstream?
    enc.seekg(0, std::ios::end);
    size_t enc_size = enc.tellp();
    enc.seekg(0, std::ios::beg);
    std::cout << "[" << __func__ << "] enc_size:" << enc_size << std::endl;



    std::vector<unsigned char> out_data(enc_size);
    enc.read((char*) out_data.data(), std::streamsize(out_data.size()));

    uint8_t buf[2];
    for(unsigned i = 0; i < out_data.size(); ++i)
    {
        buf[i % 2] = out_data[i];
        if(i % 2 == 1)
            std::cout << (buf[1] << 8 | buf[0]) << std::endl;
    }


    //enc.read(reinterpret_cast<const char*>(out_data.data()), std::streamsize(out_data.size()));

    //std::ofstream file("fuck_your_couch_charlie.murphy", std::ios::binary);

    //char buf;
    //while(!enc.eof())
    //{
    //    if(enc.fail())
    //        break;

    //    enc.read(&buf, 1);
    //    file.write(reinterpret_cast<const char*>(&buf), sizeof(const char));
    //}

    //file.close();


    //char word_buf[2];
    //enc.seekg(0, std::ios::beg);
    //while(!enc.eof())
    //{
    //    if(enc.fail())
    //        break;

    //    enc.read(word_buf, 2);
    //    uint16_t word = word_buf[1] << 8 | word_buf[0];
    //    out_data.push_back(word);
    //}


    std::cout << "Deez nuts (length " << out_data.size() << ") encoded to: ";
    for(unsigned i = 0; i < out_data.size(); ++i)
        std::cout << out_data[i] << " ";
    std::cout << std::endl;

    std::vector<std::string> words = {
        "a",    // 97
        "b",    // 98
        "ba",   // 256
        "ab",   // 257
        "baa",  // 258
        "aba",  // 259
        "aa",   // 260
        "bb",   // 98, 98
        "babaa", // 256, 258
    };

    std::vector<std::vector<lzw_symbol_t>> exp_codes = {
        {97},       // a
        {98},       // b
        {256},      // ba 
        {257},      // ab
        {258},      // baa
        {259},      // aba
        {260},      // aa
        {98, 98},   // bb
        {256, 258}, // babaa
    };

    REQUIRE(words.size() == exp_codes.size());
}


TEST_CASE("test_clear_dict", "lzw")
{
    LZWDict lzw;

    //std::vector<std::string> test = {"a", "b", "c", "d"};
    //for(unsigned i = 0; i < test.size(); ++i)
    //{
    //    std::cout << "Checking [" << std::string(i) << "]" << std::endl;
    //    REQUIRE(lzw.contains(std::string(i)) == true);
    //}

    //for(unsigned i = 0; i < LZW_ALPHA_SIZE; ++i)
    //    REQUIRE(lzw.contains(std::string(reinterpret_cast<const char*>(&i), 2)) == true);
    //for(unsigned i = LZW_ALPHA_SIZE; i < 2 * LZW_ALPHA_SIZE; ++i)
    //    REQUIRE(lzw.contains(std::string(reinterpret_cast<const char*>(&i), 2)) == false);

    //const std::string input = "babaabaaa";
    //auto codes = lzw.encode(input);   // should be 260 symbols now 

    //for(unsigned i = 0; i < 1 * LZW_ALPHA_SIZE; ++i)
    //{
    //    if(i < 260)
    //        REQUIRE(lzw.contains(std::string(reinterpret_cast<const char*>(&i), 2)) == true);
    //    else
    //        REQUIRE(lzw.contains(std::string(reinterpret_cast<const char*>(&i), 2)) == false);
    //}
    //
    //// if we clear the dict now there should be zero entries 
    //lzw.clear_dict();
    //for(unsigned i = 0; i < LZW_ALPHA_SIZE; ++i)
    //    REQUIRE(lzw.contains(std::string(reinterpret_cast<const char*>(&i), 2)) == false);

    //lzw.init();
    //for(unsigned i = 0; i < 1 * LZW_ALPHA_SIZE; ++i)
    //{
    //    if(i < LZW_ALPHA_SIZE)
    //        REQUIRE(lzw.contains(std::string(reinterpret_cast<const char*>(&i), 2)) == true);
    //    else
    //        REQUIRE(lzw.contains(std::string(reinterpret_cast<const char*>(&i), 2)) == false);
    //}
}
