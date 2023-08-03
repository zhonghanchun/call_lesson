//
// Created by 钟寒春 on 2023/3/4.
//

#ifndef LESSON2_BITSET_H
#define LESSON2_BITSET_H

#include <iostream>
#include <vector>
using namespace std;
namespace zhc
{
    template<size_t N>
    class bitset
    {
    public:
        bitset()
        {
            _bit.resize(N/8+1);
        }
        void set(size_t x)
        {
            size_t i=x/8;
            size_t j=x%8;
            _bit[i]|=(1<<j);
        }
        bool test(size_t x)
        {
            size_t i=x/8;
            size_t j=x%8;
            return _bit[i]&(~(1<<j));
        }
        void reset(size_t x)
        {
            size_t i=x/8;
            size_t j=x%8;
            _bit[i]&=(~(1<<j));
        }
    private:
        vector<char> _bit;
    };
}


#endif //LESSON2_BITSET_H
