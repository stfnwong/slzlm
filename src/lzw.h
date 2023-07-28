/*
 * LZW
 * C implementation of LZW algorithm
 *
 */

#ifndef __LZW_H
#define __LZW_H

#include <stdlib.h>

#define LZW_ALPHA_SIZE 256


/*
 * Inputs:
 *
 * uint8_t* input - pointer to array of bytes to encode
 * ssize_t   input_len - length of input array
 * uint8_t* output - pointer to encoded array;
 *
 * Returns:
 *  ssize_t number of bytes in encoded strea
 *
 */
ssize_t lzw_encode(const uint8_t* input, size_t input_len, uint8_t* output);
ssize_t lzw_decode(const uint8_t* input, size_t input_len, uint8_t* output);


#endif /*__LZW_H*/
