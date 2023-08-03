//
// Created by 钟寒春 on 2023/3/10.
//

#ifndef LESSON2_BLOOMFILTER_H
#define LESSON2_BLOOMFILTER_H
#include <iostream>
#include <bitset>
#include <string>
using namespace std;
namespace zhc
{
    struct BKDRHash
    {
        size_t operator()(const string& key)
        {
            size_t hash = 0;
            for (auto ch : key)
            {
                hash *= 131;
                hash += ch;
            }
            return hash;
        }
    };

    struct APHash
    {
        size_t operator()(const string& key)
        {
            unsigned int hash = 0;
            int i = 0;

            for (auto ch : key)
            {
                if ((i & 1) == 0)
                {
                    hash ^= ((hash << 7) ^ (ch) ^ (hash >> 3));
                }
                else
                {
                    hash ^= (~((hash << 11) ^ (ch) ^ (hash >> 5)));
                }

                ++i;
            }

            return hash;
        }
    };

    struct DJBHash
    {
        size_t operator()(const string& key)
        {
            unsigned int hash = 5381;

            for (auto ch : key)
            {
                hash += (hash << 5) + ch;
            }

            return hash;
        }
    };

    struct JSHash
    {
        size_t operator()(const string& s)
        {
            size_t hash = 1315423911;
            for (auto ch : s)
            {
                hash ^= ((hash << 5) + ch + (hash >> 2));
            }
            return hash;
        }
    };

    // 假设N是最多存储的数据个数
    // 平均存储一个值，开辟X个位
    template<size_t N,
            size_t X = 6,
            class K = string,
            class HashFunc1 = BKDRHash,
            class HashFunc2 = APHash,
            class HashFunc3 = DJBHash,
            class HashFunc4 = JSHash>
    class BloomFilter
    {
    public:
        void set(const K& key)
        {
            size_t hash1=HashFunc1()(key)%(N*X);
            size_t hash2=HashFunc2()(key)%(N*X);
            size_t hash3=HashFunc3()(key)%(N*X);
            size_t hash4=HashFunc4()(key)%(N*X);
            _bs.set(hash1);
            _bs.set(hash2);
            _bs.set(hash3);
            _bs.set(hash4);
        }
        bool test(const K& key)
        {
            size_t hash1=HashFunc1()(key)%(N*X);
            size_t hash2=HashFunc2()(key)%(N*X);
            size_t hash3=HashFunc3()(key)%(N*X);
            size_t hash4=HashFunc4()(key)%(N*X);
            if(_bs.test(hash1)&&_bs.test(hash2)&&_bs.test(hash3)&&_bs.test(hash4))
            {
                return true;
            }
            return false;
        }

    private:
        bitset<N*X> _bs;
    };
    void test_bloomfilter1(){
        string str[] = { "猪八戒", "孙悟空", "沙悟净", "唐三藏", "白龙马1","1白龙马","白1龙马","白11龙马","1白龙马1" };
        BloomFilter<10> bf;
        for (auto& str : str)
        {
            bf.set(str);
        }
        cout<<bf.test("白龙1马");
        cout<<endl<<endl;
        for (auto& s : str)
        {
            cout << bf.test(s) << endl;
        }
        cout << endl;
    }
}


#endif //LESSON2_BLOOMFILTER_H
