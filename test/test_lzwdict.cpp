// Unit tests for LZW


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

#include "LZW.hpp"


TEST_CASE("test_encode2_to_stream", "lzw")
{
    LZWDict lzw;

    std::string test_input = "babaabaaa"; // exp sequence: 98, 97, 256, 257, 97, 260
    std::stringstream ss;
    ss << test_input;
    auto enc = lzw.encode2(ss);

    std::vector<uint16_t> exp_data = {98, 97, 256, 257, 97, 260};
    std::vector<uint16_t> out_data;

    unsigned bytes_read = 0;
    while(enc)
    {
        if(enc.eof() || enc.fail())
            break;

        char word_buf[2];
        char buf;
        enc.read(&buf, sizeof(uint8_t));
        word_buf[bytes_read % 2] = buf;
        
        if(bytes_read % 2)
            out_data.push_back((word_buf[1] << 8 | word_buf[0]));
        bytes_read++;
    }

    std::cout << "encode2() output data: [";
    for(const auto& elem: out_data)
        std::cout << elem << " ";
    std::cout << "]" << std::endl;
    
    REQUIRE(out_data.size() == exp_data.size());
    for(unsigned i = 0; i < out_data.size(); ++i)
        REQUIRE(out_data[i] == exp_data[i]);

}

TEST_CASE("test_encode2_to_file", "lzw")
{
    LZWDict lzw;

    std::string test_input = "babaabaaa"; // exp sequence: 98, 97, 256, 257, 97, 260
    std::stringstream ss;
    ss << test_input;
    auto enc = lzw.encode2(ss);

    const std::string test_filename = "encode2_disk.test";

    std::ofstream file(test_filename, std::ios::binary);

    std::vector<uint16_t> out_data;
    char word_buf[2];

    char buf;
    unsigned read_count = 0;
    while(!enc.eof())
    {
        if(enc.fail())
            break;

        enc.read(&buf, 1);
        file.write(reinterpret_cast<const char*>(&buf), sizeof(const char));
        read_count++;

        word_buf[read_count % 2] = buf;
        if(read_count % 2)
            out_data.push_back(word_buf[1] << 8 | word_buf[0]);
    }

    file.close();


    // read the file off disk and check?


}





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
    
    //enc_out.seekp(0, std::ios::end);
    //size_t buf_size = enc_out.tellp();
    //enc_out.seekp(0, std::ios::beg);
    //file.write(enc_out.rdbuf(), buf_size);
    
    //for(const uint8_t b : enc_out.rdbuf())
    //    file.write(reinterpret_cast<const char*>(&b), sizeof(const char));

    file.close();
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
