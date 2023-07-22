/*
 * Dumping ground file
 */


#ifndef __UTIL_HPP
#define __UTIL_HPP


#include <cstdint>
#include <sstream>
#include <vector>

// TODO: debug only
#include <iostream>



template <typename T> std::vector<T> consume_stream_to_vec(std::stringstream& ss)
{
    std::vector<T> out_vec;
    unsigned bytes_read = 0;

    char buf;
    char word_buf[2];

    while(ss)
    {
        if(ss.eof() || ss.fail())
            break;
        if(!ss.get(buf))
            break;

        // How to get the size of the buffer here correct for any T?
        //ss.read(&buf, sizeof(uint8_t));
        word_buf[bytes_read % 2] = buf;
        
        if(bytes_read % 2)
            out_vec.push_back((word_buf[1] << 8 | word_buf[0]));
        bytes_read++;
    }

    return out_vec;
}




#endif /*__UTIL_HPP*/
