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

        word_buf[bytes_read % sizeof(T)] = buf;
        if(bytes_read % sizeof(T) == (sizeof(T)-1))
        {
            T w = 0;
            for(int p = sizeof(T)-1; p >= 0; p--)
                w = w | (word_buf[p] << (p * 8));
            out_vec.push_back(w);
        }

        bytes_read++;
    }

    ss.seekg(0, std::ios::beg);

    return out_vec;
}


template <typename T> std::stringstream vec_to_stream(const std::vector<T>& vec)
{
    std::stringstream ss;

    for(unsigned i = 0; i < vec.size(); ++i)
    {
        ss.write(reinterpret_cast<const char*>(&vec[i]), sizeof(T));
    }

    ss.seekg(0, std::ios::beg);

    return ss;
}


#endif /*__UTIL_HPP*/
