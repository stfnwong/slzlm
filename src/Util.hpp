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



/*
 * Read a stringstream into a vector. 
 * This implementation doesn't take account the variation in code
 * size over the stream.
 */
template <typename T> std::vector<T> stream_to_vec(std::stringstream& ss)
{
    std::vector<T> out_vec;
    unsigned bytes_read = 0;

    char buf;
    char word_buf[sizeof(T)];

    while(ss)
    {
        if(ss.eof() || ss.fail())
            break;
        if(!ss.get(buf))
            break;

        // How to get the size of the buffer here correct for any T?
        //ss.read(&buf, sizeof(uint8_t));

        // TODO: doesn;t work for other sizes... what is sizeof(T) 
        word_buf[bytes_read % sizeof(T)] = buf;
        if(bytes_read % sizeof(T) == (sizeof(T)-1))
            out_vec.push_back((word_buf[1] << 8 | word_buf[0]));

        //if(bytes_read % sizeof(T) == (sizeof(T)-1))
        //{
        //    //for(unsigned p = sizeof(T); p > 0; --p)
        //    auto w = word_buf[sizeof(T)-1] << (8 * sizeof(T));
        //    for(int p = sizeof(T)-1; p > 0; --p)
        //        w = w | (word_buf[p] << (sizeof(T)-1) * p);
        //    out_vec.push_back(w);
        //    std::cout << "[" << __func__ << "] w: " << w << std::endl;
        //    //out_vec.push_back((word_buf[1] << 8 | word_buf[0]));
        //}
        bytes_read++;
    }

    return out_vec;
}


#endif /*__UTIL_HPP*/
