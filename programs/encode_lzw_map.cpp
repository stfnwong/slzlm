/*
 * ENCODE
 * Encode a long string for benchmarking
 */

#include <chrono>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

#include "LZW.hpp"


void lzw_encode_long_string(const std::string& filename)
{
    std::ifstream file(filename);
    if(!file)
    {
        std::cerr << "Failed to open file [" << filename << "]" << std::endl;
        return;
    }

    auto inp_data = std::vector<uint8_t>(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
    );
    file.close();

    auto t0 = std::chrono::system_clock::now();

    uint8_t* out_data = new uint8_t[inp_data.size()];
    unsigned enc_size = lzw_encode(inp_data.data(), inp_data.size(), out_data);

    auto t1 = std::chrono::system_clock::now();

    std::cout << "[" << __func__ << "] input was [" << inp_data.size() << "] characters." << std::endl;
    std::cout << "[" << __func__ << "] output was [" << enc_size << "] characters." << std::endl;

    float r = float(enc_size) / float(inp_data.size());
    std::cout << "[" << __func__ << "] Compression ratio : " << r << std::endl;

    auto delta_t = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    std::cout << "Encoding time was: " << delta_t.count() << " ms." << std::endl;

    // TODO: Some more stats from header (eg: num_codes)?
    
    delete[] out_data;
}


int main(int argc, char *argv[])
{
    const std::string test_filename = "test/shakespear.txt";
    lzw_encode_long_string(test_filename);
    return 0;
}
