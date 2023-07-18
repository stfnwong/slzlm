/*
 * LZSS
 */

#ifndef __LZSS_HPP
#define __LZSS_HPP

#include <array>
#include <memory>
#include <vector>


constexpr const int INDEX_BIT_COUNT    = 12;
constexpr const int LENGTH_BIT_COUNT   = 4;
constexpr const int WINDOW_SIZE        = (1 << INDEX_BIT_COUNT);
constexpr const int RAW_LOOKAHEAD_SIZE = (1 << LENGTH_BIT_COUNT);
constexpr const int BREAK_EVEN         = ((1 + INDEX_BIT_COUNT + LENGTH_BIT_COUNT) / 9);
constexpr const int LOOKAHEAD_SIZE     = (RAW_LOOKAHEAD_SIZE + BREAK_EVEN);
constexpr const int TREE_ROOT          = WINDOW_SIZE;



static inline int mod_window(int a)
{
    return a & (WINDOW_SIZE - 1);
}



struct LZSNode
{
    int parent;
    int smaller;
    int larger;
};


struct LZSS
{
    std::array<uint8_t, WINDOW_SIZE> window;
    std::array<LZSNode, WINDOW_SIZE+1> tree;

    public:
        LZSS() {}
        // TODO: modify to work with streams
        void encode(const std::vector<uint8_t>& data);
};



#endif /*__LZSS_HPP*/
