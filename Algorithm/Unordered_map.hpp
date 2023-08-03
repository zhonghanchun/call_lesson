//
// Created by 钟寒春 on 2023/2/25.
//

#ifndef LESSON2_UNORDERED_MAP_H
#define LESSON2_UNORDERED_MAP_H
#include "HashTable.h"
namespace zhc
{
    template<class K,class V,class Hash=zhc_hash::HashFunc<K>>
    class unordered_map
    {
        struct MapOFT
        {
            const K& operator()(const pair<K,V>& kv)
            {
                return kv.first;
            }
        };
    //typedef typename zhc_hash::HashTable<K,pair<const K,V>,Hash,MapOFT>::iterator iterator;
    public:
        typedef typename zhc_hash::HashTable<K,pair<const K,V>,Hash,MapOFT>::iterator iterator;
        typedef typename zhc_hash::HashTable<K,pair<const K,V>,Hash,MapOFT>::const_iterator const_iterator;

        const_iterator begin()const
        {
            return _ht.begin();
        }
        const_iterator end()const
        {
            return _ht.end();
        }
        iterator begin()
        {
            return _ht.begin();
        }
        iterator end()
        {
            return _ht.end();
        }
        pair<iterator,bool> insert(const pair<K,V>& kv)
        {
            return _ht.insert(kv);
        }
        V& operator[](const K& key)
        {
            pair<iterator,bool> ret=_ht.insert(make_pair(key,V()));
            return ret.first->second;
        }
    private:
        zhc_hash::HashTable<K,pair<const K,V>,Hash,MapOFT> _ht;
    };
    void Func(const unordered_map<string,int>& mp )
    {
        unordered_map<string,int>::const_iterator it=mp.begin();
        while(it!=mp.end())
        {
            cout<<it->first<<"->"<<it->second<<endl;
            ++it;
        }
    }
    void Test1()
    {
        string a[]={"苹果","苹果","苹果","苹果","苹果","香蕉","香蕉","香蕉","西瓜","西瓜"};
        unordered_map<string,int> Mp;
        for(auto& str : a)
        {
            Mp[str]++;
        }
        for(auto& e : Mp)
        {
            cout<<e.first<<"->"<<e.second<<endl;
        }
        unordered_map<string,int>::iterator it=Mp.begin();
        while(it!=Mp.end())
        {
            cout<<it->first<<"->"<<it->second<<endl;
            ++it;
        }
        cout<<endl;
        Func(Mp);
    }
}
#endif //LESSON2_UNORDERED_MAP_H
