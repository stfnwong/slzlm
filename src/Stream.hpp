/*
 * STREAM
 * Wrapper over a stream
 *
 */

#ifndef __STREAM_HPP
#define __STREAM_HPP


#include <cstdint>
#include <fstream>
#include <sstream>


struct LZStream
{
    uint32_t offset24;
    uint32_t offset32;
    uint32_t num_codes;
    uint32_t size;
    std::stringstream data;

    public:
        void to_file(const std::string& filename);
        void write(uint32_t value, unsigned code_size);
};



#endif /*__STREAM_HPP*/
