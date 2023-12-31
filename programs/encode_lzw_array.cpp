/*
 * ENCODE
 * Encode a long string for benchmarking
 */

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "LZW.hpp"


void lzw_encode_long_string(const std::string& filename)
{
    std::ifstream file(filename);
    std::string text(std::istreambuf_iterator<char>{file}, {});
    file.close();

    auto t0 = std::chrono::system_clock::now();
    std::stringstream enc = lzw_array_encode(text);
    auto t1 = std::chrono::system_clock::now();

    enc.seekg(0, std::ios::end);
    size_t enc_size = enc.tellg();
    enc.seekg(0, std::ios::beg);

    std::cout << "[" << __func__ << "] input was [" << text.size() << "] characters." << std::endl;
    std::cout << "[" << __func__ << "] output was [" << enc_size << "] characters." << std::endl;

    float r = float(enc_size) / float(text.size());
    std::cout << "[" << __func__ << "] Compression ratio : " << r << std::endl;

    auto delta_t = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    std::cout << "Encoding time was: " << delta_t.count() << " ms." << std::endl;
}


int main(int argc, char *argv[])
{
    const std::string test_filename = "test/shakespear.txt";
    lzw_encode_long_string(test_filename);
    return 0;
}
