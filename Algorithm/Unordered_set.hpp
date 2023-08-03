//
// Created by 钟寒春 on 2023/2/25.
//

#ifndef LESSON2_UNORDERED_SET_H
#define LESSON2_UNORDERED_SET_H
#include "HashTable.h"
namespace zhc {
    template<class K, class Hash=zhc_hash::HashFunc<K>>
    class unordered_set {
        struct SetOFT {
            const K &operator()(const K &k) {
                return k;
            }
        };

    public:
        typedef typename zhc_hash::HashTable<K, K, Hash, SetOFT>::iterator iterator;

        iterator begin() {
            return _ht.begin();
        }

        iterator end() {
            return _ht.end();
        }

        pair<iterator, bool> insert(const K &key) {
            return _ht.insert(key);
        }

    private:
        zhc_hash::HashTable<K, K, Hash, SetOFT> _ht;
    };
    void Test2()
    {
        unordered_set<int> us;
        us.insert(13);
        us.insert(3);
        us.insert(23);
        us.insert(5);
        us.insert(5);
        us.insert(6);
        us.insert(15);
        us.insert(223342);
        us.insert(22);

        unordered_set<int>::iterator it = us.begin();
        while (it != us.end())
        {
            cout << *it << " ";
            ++it;
        }
        cout << endl;

        for (auto e : us)
        {
            cout << e << " ";
        }
        cout << endl;
        int c;
        cin>>c;
    }
}
#endif //LESSON2_UNORDERED_SET_H
