/*
 * Dumping ground file
 */


#ifndef __UTIL_HPP
#define __UTIL_HPP


#include <cstdint>
#include <sstream>
#include <vector>


template <typename T> std::vector<T> consume_stream_to_vec(std::stringstream& ss)
{
    std::vector<T> out_vec;
    unsigned bytes_read = 0;

    while(ss)
    {
        if(ss.eof() || ss.fail())
            break;

        char word_buf[2];
        char buf;
        ss.read(&buf, sizeof(uint8_t));
        word_buf[bytes_read % 2] = buf;
        
        if(bytes_read % 2)
            out_vec.push_back((word_buf[1] << 8 | word_buf[0]));
        bytes_read++;
    }

    return out_vec;
}




#endif /*__UTIL_HPP*/
