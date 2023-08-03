//
// Created by 钟寒春 on 2023/2/20.
//

#ifndef LESSON2_HASHTABLE_H
#define LESSON2_HASHTABLE_H
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;
namespace zhc_test1 {
    enum States {
        EMPTY,
        EXIST,
        DELETE
    };

    template<class K>
    struct HashFunc {
        size_t operator()(const K &k) {
            return (size_t) k;
        }
    };

    template<>
    struct HashFunc<string> {
        size_t operator()(const string &s) {
            size_t n = 0;
            for (auto &ch: s) {
                n = n * 131 + ch;
            }
            return n;
        }
    };

    template<class K, class V>
    struct Hash_data {
        pair<K, V> _kv;
        States _states = EMPTY;
    };

    template<class K, class V, class hash=HashFunc<K>>
    class Hash {
        typedef Hash_data<K, V> data;
    public:
        Hash()
                : _n(0) {
            _hs.resize(10);
        }

        bool insert(const pair<K, V> &kv) {
            hash hf;
            if (find(kv.first)) {
                return false;
            }
            if (_n * 10 / _hs.size() > 7) {
                Hash<K, V,hash> newHash;
                newHash._hs.resize(_hs.size() * 2);
                for (auto &e: _hs) {
                    if (e._states == EXIST)
                        newHash.insert(e._kv);
                }
                _hs.swap(newHash._hs);
            }
            size_t index = hf(kv.first) % _hs.size();
            while (_hs[index]._states == EXIST) {
                index++;
                index %= _hs.size();
            }
            _hs[index]._kv = kv;
            _hs[index]._states=EXIST;
            _n++;
            return true;
        }

        data *find(const K &k) {
            hash hf;
            size_t index = hf(k) % _hs.size();
            while (_hs[index]._states != EMPTY) {
                if (_hs[index]._states == EXIST && _hs[index]._kv.first == k) {
                    return &_hs[index];
                }
                ++index;
                index%=_hs.size();
            }
            return nullptr;
        }

        bool erase(const K &k) {
            data *pos = find(k);
            if (pos) {
                pos->_states = DELETE;
                --_n;
                return true;
            }
            return false;
        }

    private:
        vector<data> _hs;
        size_t _n;
    };
}
void test()
{
    string a[]={"苹果","苹果","苹果","苹果","苹果","香蕉","香蕉","香蕉","西瓜","西瓜"};
    zhc_test1::Hash<string,int> hs;
    for(auto ch:a)
    {
        zhc_test1::Hash_data<string,int>* pos=hs.find(ch);
        if(pos)
        {
            pos->_kv.second++;
        }
        else
        {
            hs.insert(make_pair(ch,1));
        }
    }
}
namespace Hash
{
    template<class K>
    struct HashFunc {
        size_t operator()(const K &k) {
            return (size_t) k;
        }
    };

    template<>
    struct HashFunc<string> {
        size_t operator()(const string &s) {
            size_t n = 0;
            for (auto &ch: s) {
                n = n * 131 + ch;
            }
            return n;
        }
    };
    template<class K,class V>
    struct Hash_data
    {
        pair<K,V> _kv;
        Hash_data<K,V>* _next;
        Hash_data(const pair<K,V>& kv)
            :_kv(kv),_next(nullptr)
        {}
    };
    template<class K,class V,class hash=HashFunc<K>>
    class HashTable
    {
        typedef Hash_data<K,V> Node;
    public:
        HashTable()
            :_size(0)
        {
            _stable.resize(10);
        }
        ~HashTable()
        {
            for(size_t i=0;i<_stable.size();i++)
            {
                Node* cur=_stable[i];
                while(cur)
                {
                    Node* next=cur->_next;
                    delete cur;
                    cur=next;
                }
                _stable[i]= nullptr;
            }
        }
        bool insert(const pair<K,V>& kv)
        {
            if(find(kv.first))
            {
                return false;
            }
            hash hf;
            if(_size==_stable.size())
            {
                vector<Node*> newTable;
                newTable.resize(_stable.size()*2, nullptr);
                for(size_t i=0;i<_stable.size();i++)
                {
                    Node* cur=_stable[i];
                    while(cur)
                    {
                        Node* next=cur->_next;
                        size_t index=hf(cur->_kv.first)%newTable.size();
                        cur->_next=newTable[index];
                        newTable[index]=cur;
                        cur=next;
                    }
                    _stable[i]= nullptr;
                }
                _stable.swap(newTable);
            }
            size_t index=hf(kv.first)%_stable.size();
            Node* newnode=new Node(kv);
            newnode->_next=_stable[index];
            _stable[index]=newnode;
            ++_size;
            return true;
        }
        bool erase(const pair<K,V>& kv)
        {
            if(!find(kv.first)) {
                return false;
            }
            hash hf;
            size_t index=hf(kv.first)%_stable.size();
            Node* prev= nullptr;
            Node* cur=_stable[index];
            while(cur)
            {
                if(cur->_kv.first==kv.first)
                {
                    break;
                }
                else
                {
                    prev=cur;
                    cur=cur->_next;
                }
            }
            if(cur==_stable[index])
            {
                _stable[index]=cur->_next;
                delete cur;
                return true;
            }
            else
            {
                prev->_next=cur->_next;
                delete cur;
                return true;
            }
            return false;
        }
        Node* find(const K& key)
        {
            hash hf;
            size_t index=hf(key)%_stable.size();
            Node* cur=_stable[index];
            while(cur)
            {
                if(cur->_kv.first==key)
                {
                    return cur;
                }
                cur=cur->_next;
            }
            return nullptr;
        }
        void Printf()
        {
            for(size_t i=0;i<_stable.size();i++)
            {
                Node* cur=_stable[i];
                while(cur)
                {
                    cout<<cur->_kv.first<<"->"<<cur->_kv.second<<endl;
                    cur=cur->_next;
                }
            }
        }
    private:
        vector<Node*> _stable;
        size_t _size=0;
    };
    void Test()
    {
        vector<int> v{1,2,3,4,5,12,15,13,33,23};
        HashTable<string,int> ht;
        string a[]={"苹果","苹果","苹果","苹果","苹果","香蕉","香蕉","香蕉","西瓜","西瓜"};
        for(auto& str:a)
        {
            Hash_data<string,int>* pos=ht.find(str);
            //auto pos=ht.find(str);
            if(pos== nullptr)
            {
                ht.insert(make_pair(str,1));
            }
            else
            {
                pos->_kv.second++;
            }
        }
        int b;
        cin>>b;
    }
}
namespace zhc_hash
{
    template<class K>
    struct HashFunc {
        size_t operator()(const K &k) {
            return (size_t) k;
        }
    };

    template<>
    struct HashFunc<string> {
        size_t operator()(const string &s) {
            size_t n = 0;
            for (auto &ch: s) {
                n = n * 131 + ch;
            }
            return n;
        }
    };
    template<class T>
    struct Hash_data
    {
        T _data;
        Hash_data<T>* _next;
        Hash_data(const T& data)
                :_data(data),_next(nullptr)
        {}
    };
    template<class K,class T,class hash,class KeyOFT>
    class HashTable;

    template<class K,class T,class Ptr,class Ref,class hash,class KeyOFT>
    struct _const_iterator
    {
        typedef Hash_data<T> Node;
        typedef _const_iterator<K,T,Ptr,Ref,hash,KeyOFT> Self;
        typedef HashTable<K,T,hash,KeyOFT> Ht;
        const Ht* _ht;
        const Node* _node;
        _const_iterator(const Node* node,const Ht* ht)
            :_node(node),_ht(ht)
        {}
        _const_iterator(){}
        Ptr operator*()
        {
            return _node->_data;
        }
        Ref operator->()
        {
            return &_node->_data;
        }

        bool operator!=(const Self& s)const
        {
            return _node!=s._node;
        }
        Self& operator++()
        {
            if(_node->_next)
            {
                _node=_node->_next;
            }
            else
            {
                KeyOFT kot;
                hash hf;
                size_t index=hf(kot(_node->_data))% _ht->_stable.size();
                ++index;
                while(index<_ht->_stable.size())
                {
                    if(_ht->_stable[index])
                    {
                        _node=_ht->_stable[index];
                        break;
                    }
                    else
                    {
                        ++index;
                    }
                }
                if(index==_ht->_stable.size())
                {
                    _node= nullptr;
                }
            }
            return *this;
        }
    };

    template<class K,class T,class Ptr,class Ref,class hash,class KeyOFT>
    struct _Iterator
    {
        typedef Hash_data<T> Node;
        typedef _Iterator<K,T,Ptr,Ref,hash,KeyOFT> Self;
        typedef HashTable<K,T,hash,KeyOFT> Ht;
        Node* _node;
        Ht* _ht;
        _Iterator(Node* node,Ht* ht)
            :_node(node),_ht(ht)
        {}
        _Iterator(){};
        Ptr operator*()
        {
            return _node->_data;
        }
        Ref operator->()
        {
            return &_node->_data;
        }

        Ptr operator*() const
        {
            return _node->_data;
        }
        Ref operator->()const
        {
            return &_node->_data;
        }
        bool operator!=(const Self& s)const
        {
            return _node!=s._node;
        }
        Self& operator++()
        {
            if(_node->_next)
            {
                _node=_node->_next;
            }
            else
            {
                KeyOFT kot;
                hash hf;
                size_t index=hf(kot(_node->_data))% _ht->_stable.size();
                ++index;
                while(index<_ht->_stable.size())
                {
                    if(_ht->_stable[index])
                    {
                        _node=_ht->_stable[index];
                        break;
                    }
                    else
                    {
                        ++index;
                    }
                }
                if(index==_ht->_stable.size())
                {
                    _node= nullptr;
                }
            }
            return *this;
        }
    };
    template<class K,class T,class hash,class KeyOFT>
    class HashTable
    {
        typedef Hash_data<T> Node;
        //template<class K,class T,class hash,class KeyOFT>
        friend struct _Iterator<K,T,T&,T*,hash,KeyOFT>;
        friend struct _const_iterator<K,T,const T&,const T*,hash,KeyOFT>;
    public:
        typedef _Iterator<K,T,T&,T*,hash,KeyOFT> iterator;
        typedef _const_iterator<K,T,const T&,const T*,hash,KeyOFT> const_iterator;
        iterator begin()
        {
            for(size_t i=0;i<_stable.size();i++)
            {
                if(_stable[i])
                {
                    return iterator(_stable[i],this);
                }
            }
            return iterator(nullptr,this);
        }
        iterator end()
        {
            return iterator(nullptr,this);
        }
        const_iterator begin()const
        {
            for(size_t i=0;i<_stable.size();i++)
            {
                if(_stable[i])
                {
                    return const_iterator(_stable[i],this);
                }
            }
            return const_iterator(nullptr,this);
        }
        const_iterator end()const
        {
            return const_iterator(nullptr,this);
        }

        HashTable()
                :_size(0)
        {
            _stable.resize(10);
        }
        pair<iterator,bool> insert(const T& data)
        {
            KeyOFT kot;
            iterator it=find(kot(data));
            if(it!=end())
            {
                return make_pair(it,false);
            }
            hash hf;
            if(_size==_stable.size())
            {
                vector<Node*> newTable;
                newTable.resize(_stable.size()*2, nullptr);
                for(size_t i=0;i<_stable.size();i++)
                {
                    Node* cur=_stable[i];
                    while(cur)
                    {
                        Node* next=cur->_next;
                        size_t index=hf(kot(cur->_data))%newTable.size();
                        cur->_next=newTable[index];
                        newTable[index]=cur;
                        cur=next;
                    }
                    _stable[i]= nullptr;
                }
                _stable.swap(newTable);
            }
            size_t index=hf(kot(data))%_stable.size();
            Node* newnode=new Node(data);
            newnode->_next=_stable[index];
            _stable[index]=newnode;
            ++_size;
            return make_pair(iterator(newnode,this),false);
        }
        bool erase(const T& data)
        {
            if(!find(kot(data))) {
                return false;
            }
            hash hf;
            size_t index=hf(kot(data))%_stable.size();
            Node* prev= nullptr;
            Node* cur=_stable[index];
            while(cur)
            {
                if(kot(cur->_data)==kot(data))
                {
                    break;
                }
                else
                {
                    prev=cur;
                    cur=cur->_next;
                }
            }
            if(cur==_stable[index])
            {
                _stable[index]=cur->_next;
                delete cur;
                return true;
            }
            else
            {
                prev->_next=cur->_next;
                delete cur;
                return true;
            }
            return false;
        }
        iterator find(const K& key)
        {
            hash hf;
            KeyOFT kot;
            size_t index=hf(key)%_stable.size();
            Node* cur=_stable[index];
            while(cur)
            {
                if(kot(cur->_data) ==key)
                {
                    return iterator(cur,this);
                }
                cur=cur->_next;
            }
            return iterator(nullptr,this);
        }
        void Printf()
        {
            for(size_t i=0;i<_stable.size();i++)
            {
                Node* cur=_stable[i];
                while(cur )
                {
                    cout<<cur->_kv.first<<"->"<<cur->_kv.second<<endl;
                    cur=cur->_next;
                }
            }
        }
    private:
        vector<Node*> _stable;
        size_t _size=0;
    };
}
#endif //LESSON2_HASHTABLE_H
