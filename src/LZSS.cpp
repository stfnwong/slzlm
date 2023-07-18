/*
 * LZSS
 */



#include "LZSS.hpp"



void LZSS::encode(const std::vector<uint8_t>& data)
{
    int cur_pos = 0;


    for(int i = 0; i < LOOKAHEAD_SIZE; ++i)
        this->window[cur_pos + i] = data[i];  // TODO: fucked
}
