// Unit tests for LZW


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
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


// ===== Functional encoder ===== //

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


TEST_CASE("test_encode_long_string", "lzw")
{
    std::string test_filename = "test/shakespear.txt";
    std::ifstream file(test_filename);
    std::string text(std::istreambuf_iterator<char>{file}, {});
    file.close();

    std::stringstream enc_out = lzw_encode(text);

    REQUIRE(enc_out.str().size() < text.size());
    //std::cout << "enc_out: " << enc_out.str() << std::endl << std::endl;
    std::cout << "text length    : " <<  text.size() << " characters" << std::endl;
    std::cout << "encoded length : " << enc_out.str().size() << " characters" << std::endl;
    float r = float(enc_out.str().size()) / float(text.size());
    std::cout << "Ratio : " << r << std::endl;
    REQUIRE(r < 1.0);
}



// ===== Functional decoder ===== //

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


// ===== Object oriented encoder ===== //

TEST_CASE("test_lzw_encoder_count", "lzw")
{
    LZWEncoder lzw;

    // We insert LZW_ALPHA_SIZE nodes by default 
    REQUIRE(lzw.size() == LZW_ALPHA_SIZE);
    std::string test_input = "babaabaaa"; 

    // If we encode the test string we should generate 5 extra symbols
    lzw.encode(test_input);
    REQUIRE(lzw.size() == LZW_ALPHA_SIZE + 5);

    // Calling clear() should reset the code size
    lzw.init();
    REQUIRE(lzw.size() == LZW_ALPHA_SIZE);
}



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


TEST_CASE("test_lzw_encoder_loop", "lzw")
{
    std::string test_filename = "test/shakespear.txt";
    std::ifstream file(test_filename);
    std::string text(std::istreambuf_iterator<char>{file}, {});
    file.close();

    LZWEncoder lzw;

    unsigned chunk_size = 64;
    unsigned num_chunks = 8;

    for(unsigned c = 0; c < num_chunks-1; ++c)
    {
        std::string chunk = text.substr(c * chunk_size, chunk_size);
        std::cout << "chunk [" << c+1 << "/" <<  num_chunks << "]: (" << chunk.size() << " characters)" << std::endl;
        lzw.encode(chunk);
    }

    auto out_data = lzw.get_stream();
    std::cout << "lzw contains " << lzw.size() << " symbols after encoding " 
        << chunk_size * num_chunks << " characters" << std::endl;
    //REQUIRE(out_data.size() < chunk_size * num_chunks);

    std::string text_substr = text.substr(0, num_chunks * chunk_size);
    std::cout << std::endl << "Complete text substring: [" << text_substr << "]" << std::endl;
    std::cout << "Text substring contains " << text_substr.size() << " characters" << std::endl;
    
    //LZWEncoder ref_enc;
    //ref_enc.encode(text_substr);
    //auto exp_out_data = ref_enc.get();
    auto exp_out_data = lzw_encode(text_substr);

    std::cout << std::endl << "out_data ; " << out_data.str() << std::endl;
    std::cout << std::endl << "exp_data ; " << exp_out_data.str() << std::endl;

    // TODO: debug only, remove 
    std::ofstream enc_file("test/bard_loop_encode.lz", std::ios::binary);
    enc_file << out_data.rdbuf();
    enc_file.close();

    std::ofstream ref_file("test/bard_func_encode.lz", std::ios::binary);
    ref_file << exp_out_data.rdbuf();
    ref_file.close();


    
    //REQUIRE(out_data.size() == exp_out_data.size());
    out_data.seekg(0, std::ios::end);
    size_t out_data_size = out_data.tellg();
    exp_out_data.seekg(0, std::ios::end);
    size_t exp_out_data_size = exp_out_data.tellg();

    REQUIRE(exp_out_data_size == out_data_size);
}


TEST_CASE("test_lzw_encoder_to_file", "lzw")
{
    LZWEncoder lzw;
    const std::string test_data = "babaabaaa";
    const std::string test_filename = "test/to_file_test.lz";

    lzw.encode(test_data);
    lzw.to_file(test_filename);

    // Read the file and check
}


// ===== Object oriented encoder ===== //

TEST_CASE("test_lzw_decoder_count", "lzw")
{
    LZWDecoder lzw;
    // Get some encoded data 
    std::stringstream input = generate_exp_stream_data();

    REQUIRE(lzw.size() == LZW_ALPHA_SIZE);

    // As we decode we expect more symbols
    lzw.decode(input);
    REQUIRE(lzw.size() == LZW_ALPHA_SIZE+5);

    lzw.init();
    REQUIRE(lzw.size() == LZW_ALPHA_SIZE);
}


TEST_CASE("test_lzw_decoder_decode", "lzw")
{
    LZWDecoder lzw;

    std::stringstream input = generate_exp_stream_data() ;
    lzw.decode(input);
    auto dec_out_str = lzw.get();

    std::string exp_out_str = "babaabaaa";

    REQUIRE(exp_out_str.size() == dec_out_str.size());
    REQUIRE(exp_out_str == dec_out_str);
}






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


TEST_CASE("test_lzw_decode_sv", "lzw")
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

    std::string inp_str = input.str();
    auto dec_out = lzw_decode_sv(inp_str);

    //std::cout << "dec_out_str : " << dec_out.str() << std::endl;

    std::string exp_out_str = "babaabaaa";
    //std::string dec_out_str = dec_out.str();

    REQUIRE(exp_out_str.size() == dec_out.size());

    for(unsigned i = 0; i < dec_out.size(); ++i)
        REQUIRE(exp_out_str[i] == dec_out[i]);


    //REQUIRE(exp_out_str == dec_out_str);
}

TEST_CASE("test_array_encoder_long_string", "lzw")
{
    std::string test_filename = "test/shakespear.txt";
    std::ifstream file(test_filename);
    std::string text(std::istreambuf_iterator<char>{file}, {});
    file.close();

    std::stringstream enc_out = lzw_array_encode(text);

    REQUIRE(enc_out.str().size() < text.size());
    //std::cout << "enc_out: " << enc_out.str() << std::endl << std::endl;
    std::cout << "text length    : " <<  text.size() << " characters" << std::endl;
    std::cout << "encoded length : " << enc_out.str().size() << " characters" << std::endl;
    float r = float(enc_out.str().size()) / float(text.size());
    std::cout << "Ratio : " << r << std::endl;
    REQUIRE(r < 1.0);
}



//TEST_CASE("test_lzw_decoder_loop", "lzw")
//{
//    // Get some text
//    unsigned chunk_size = 64;
//    unsigned num_chunks = 8;
//    unsigned text_length = chunk_size * num_chunks;
//
//    std::string test_filename = "test/shakespear.txt";
//    std::ifstream file(test_filename);
//
//    std::string text;
//    std::copy_n(
//            std::istreambuf_iterator<char>(file.rdbuf()), 
//            text_length, 
//            std::back_inserter(text)
//    );
//
//    file.close();
//
//    // Get an encoded version
//    auto enc_text = lzw_encode(text);
//
//    LZWDecoder lzw;
//    for(unsigned c = 0; c < num_chunks-1; ++c)
//    {
//        //std::string chunk = text.substr(c * chunk_size, chunk_size);
//        std::stringstream chunk(text.substr(c * chunk_size, chunk_size));
//
//        std::cout << "Chunk [" << c+1 << "/" << num_chunks << "] : [";
//        // DEBUG: show chunk contents
//        chunk.seekg(0, std::ios::beg);
//        while(chunk && !chunk.eof())
//        {
//            char c;
//            chunk.read(&c, sizeof(char));
//            std::cout << c;
//        }
//        std::cout << "]" << std::endl;
//        chunk.seekg(0, std::ios::beg);
//
//        lzw.decode(chunk);      // TODO: fucked
//    }
//
//    auto out_data = lzw.get_stream();
//    out_data.seekg(0, std::ios::end);
//    size_t out_size = out_data.tellg();
//    out_data.seekg(0, std::ios::beg);
//}
