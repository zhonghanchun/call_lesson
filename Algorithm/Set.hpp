//
// Created by 钟寒春 on 2023/2/13.
//

#ifndef LESSON2_SET_H
#define LESSON2_SET_H
#include "RBTree.h"
namespace zhc
{
    template<class K>
    class set
    {
        struct setKeyOFt
        {
            const K& operator()(const K& k)
            {
                return k;
            }
        };

    public:
        typedef typename RBTree<K,K,setKeyOFt>::const_iterator iterator;
        typedef typename RBTree<K,K,setKeyOFt>::const_iterator const_iterator;
        typedef typename RBTree<K,K,setKeyOFt>::const_reverse_iterator reverse_iterator;
        typedef typename RBTree<K,K,setKeyOFt>::const_reverse_iterator const_reverse_iterator;
        reverse_iterator rbegin()
        {
            return _t.rbegin();
        }
        reverse_iterator rend()
        {
            return _t.rend();
        }
        const_reverse_iterator rend()const {
            return _t.rbegin();
        }
        const_reverse_iterator rbegin()const
        {
            return _t.rbegin();
        }
        iterator begin()
        {
            return _t.begin();
        }
        iterator end()
        {
            return _t.end();
        }
        const_iterator begin()const
        {
            return _t.begin();
        }
        const_iterator end()const
        {
            return _t.end();
        }
        pair<iterator,bool> insert(const K& key)
        {
            return _t.insert(key);
        }
    private:
        RBTree<K,K,setKeyOFt> _t;
    };
}


#endif //LESSON2_SET_H
