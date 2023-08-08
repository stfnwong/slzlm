/*
 * COMP
 * Compressors
 *
 */

#ifndef __LZW_HPP
#define __LZW_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <sstream>


using lzw_symbol_t = uint8_t;
const constexpr uint32_t LZW_ALPHA_SIZE = 1 << (8 * sizeof(lzw_symbol_t));


/*
 * Encode from a C-style array
 */
unsigned lzw_encode(const uint8_t* inp_data, unsigned inp_length, uint8_t* out_data);

std::vector<uint8_t> lzw_decode(const uint8_t* inp_data, unsigned inp_length);


/*
 * Encoder where the node contains an array rather than a map
 */
std::stringstream lzw_array_encode(const std::string_view data);




#endif /*__LZW_HPP*/
