//
// Created by 钟寒春 on 2023/2/13.
//

#ifndef LESSON2_MAP_H
#define LESSON2_MAP_H
#include "RBTree.h"
namespace zhc
{
    template <class K,class V>
    class map
    {
        struct mapKeyOft
        {
            const V& operator()(const pair<const K,V>& v)
            {
                return v.first;
            }
        };
    public:
        typedef typename RBTree<K,pair<const K,V>,mapKeyOft>::iterator iterator;
        typedef typename RBTree<K,pair<const K,V>,mapKeyOft>::const_iterator const_iterator;
        typedef typename RBTree<K,pair<const K,V>,mapKeyOft>::reverse_iterator reverse_iterator;
        typedef typename RBTree<K,pair<const K,V>,mapKeyOft>::const_reverse_iterator const_reverse_iterator;
        reverse_iterator rbegin()
        {
            return _t.rbegin();
        }
        reverse_iterator rend()
        {
            return _t.rend();
        }
        const_reverse_iterator rbegin()const
        {
            return _t.rbegin();
        }
        const_reverse_iterator rend()const
        {
            return _t.rend();
        }
        iterator begin()
        {
            return _t.begin();
        }
        iterator end()
        {
           return  _t.end();
        }
        const_iterator begin()const
        {
            return _t.begin();
        }
        const_iterator end()const
        {
            return _t.end();
        }
        pair<iterator,bool> insert(const pair<const K,V>& kv)
        {
            return _t.insert(kv);
        }
        V& operator[](const K& kv)
        {
            pair<iterator,bool> res= insert(make_pair(kv,V()));
            return res.first->second;
        }
    private:
        RBTree<K,pair<const K,V>,mapKeyOft> _t;
    };
}


#endif //LESSON2_MAP_H
