/*
 * LZ77
 */

#include <cstdint>

#include "LZ77.hpp"



// Note that this backref will make the overall stream much larger
struct Backref
{
    uint16_t offset;
    uint16_t length;
    uint16_t value;
};




std::stringstream lz77_encode(const std::string_view data)
{
    std::stringstream out;

    return out;
}
