/*
 * STREAM
 * Wrapper over a stream
 *
 */


#include "Stream.hpp"


void LZStream::to_file(const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary);

    // write the header 
    file.write(reinterpret_cast<const char*>(&this->offset24), sizeof(uint32_t));
    file.write(reinterpret_cast<const char*>(&this->offset32), sizeof(uint32_t));
    file.write(reinterpret_cast<const char*>(&this->num_codes), sizeof(uint32_t));
    file << this->data.str();   // TODO: probably won't work..

    //file.write(reinterpret_cast<const char*>(&
}


/*
 * Write a single value into the stream
 */
void LZStream::write(uint32_t value, unsigned code_size)
{
    this->data.write(reinterpret_cast<const char*>(&value), code_size);
}
