/*
 * STREAM
 * Wrapper over a stream
 *
 */


#include "Stream.hpp"


std::vector<uint8_t> LZStream::to_vec(void)
{
    // guess the stream size
    this->data.seekp(0, std::ios::end);
    size_t length = this->data.tellp();
    this->data.seekp(0, std::ios::beg);

    std::vector<uint8_t> out_vec(length);


}

void LZStream::to_file(const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary);

    // write the header 
    file.write(reinterpret_cast<const char*>(&this->offset24), sizeof(uint32_t));
    file.write(reinterpret_cast<const char*>(&this->offset32), sizeof(uint32_t));
    file.write(reinterpret_cast<const char*>(&this->num_codes), sizeof(uint32_t));

    // TODO: do I really have to write each byte? There isn't an interface where I 
    // can just say "consume all the bytes and write them as-is"?
    char c;
    while(this->data)
    {
        if(this->data.eof() || this->data.fail())
            break;

        this->data.read(&c, sizeof(const char));
        file.write(&c, sizeof(const char));
    }
    this->data.seekp(0, std::ios::beg);

    file.close();
}


/*
 * Write a single value into the stream
 */
void LZStream::write(uint32_t value, unsigned code_size)
{
    this->data.write(reinterpret_cast<const char*>(&value), code_size);
}
