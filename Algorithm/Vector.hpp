//
// Created by 钟寒春 on 2023/3/9.
//

#ifndef LESSON2_VECTOR_H
#define LESSON2_VECTOR_H
#include <iostream>
using namespace std;
namespace zhc
{
    template<class T>
    class vector
    {
    public:
        typedef T* iterator;
        typedef const T* const_iterator;

        iterator begin()
        {
            return _start;
        }

        iterator end()
        {
            return _finish;
        }

        const_iterator begin() const
        {
            return _start;
        }

        const_iterator end() const
        {
            return _finish;
        }

        T& operator[](size_t pos)
        {
            assert(pos < size());
            return _start[pos];
        }

        const T& operator[](size_t pos) const
        {
            assert(pos < size());
            return _start[pos];
        }

        vector()
                :_start(nullptr)
                , _finish(nullptr)
                , _endofstorage(nullptr)
        {}

        // v2(v1)
        /*vector(const vector<T>& v)
            :_start(nullptr)
            , _finish(nullptr)
            , _endofstorage(nullptr)
        {
            reserve(v.capacity());
            for (const auto& e : v)
            {
                push_back(e);
            }
        }*/

        //vector<int> v1(10, 1);
        //vector<char> v1(10, 'A');

        vector(int n, const T& val = T())
                :_start(nullptr)
                , _finish(nullptr)
                , _endofstorage(nullptr)
        {
            reserve(n);
            for (int i = 0; i < n; ++i)
            {
                push_back(val);
            }
        }

        vector(size_t n, const T& val = T())
                :_start(nullptr)
                , _finish(nullptr)
                , _endofstorage(nullptr)
        {
            reserve(n);
            for (size_t i = 0; i < n; ++i)
            {
                push_back(val);
            }
        }

        template <class InputIterator>
        vector(InputIterator first, InputIterator last)
                :_start(nullptr)
                , _finish(nullptr)
                , _endofstorage(nullptr)
        {
            while (first != last)
            {
                push_back(*first);
                ++first;
            }
        }

        vector(const vector<T>& v)
                :_start(nullptr)
                , _finish(nullptr)
                , _endofstorage(nullptr)
        {
            vector<T> tmp(v.begin(), v.end());
            swap(tmp);
        }
        vector(vector<T>&& v)
            :_start(nullptr)
            ,_finish(nullptr)
            ,_endofstorage(nullptr)
        {
            cout<<"移动拷贝构造"<<endl;
            this->swap(v);
        }
        // v1 = v2
        // v1 = v1;  // 极少数情况，能保证正确性，所以这里就这样写没什么问题
        vector<T>& operator=(vector<T> v)
        {
            swap(v);
            return *this;
        }
        vector<T>& operator=(vector<T>&& v)
        {
            cout<<"移动赋值"<<endl;
            this->swap(v);
            return *this;
        }

        ~vector()
        {
            delete[] _start;
            _start = _finish = _endofstorage = nullptr;
        }

        // 17:12继续
        void reserve(size_t n)
        {
            if (n > capacity())
            {
                size_t oldSize = size();
                T* tmp = new T[n];

                if (_start)
                {
                    //memcpy(tmp, _start, sizeof(T)*oldSize);
                    for (size_t i = 0; i < oldSize; ++i)
                    {
                        tmp[i] = _start[i];
                    }

                    delete[] _start;
                }

                _start = tmp;
                _finish = tmp + oldSize;
                _endofstorage = _start + n;
            }
        }

        void resize(size_t n, T val = T())
        {
            if (n > capacity())
            {
                reserve(n);
            }

            if (n > size())
            {
                while (_finish < _start + n)
                {
                    *_finish = val;
                    ++_finish;
                }
            }
            else
            {
                _finish = _start + n;
            }
        }

        bool empty() const
        {
            return _finish == _start;
        }

        size_t size() const
        {
            return _finish - _start;
        }

        size_t capacity() const
        {
            return _endofstorage - _start;
        }

        void push_back(const T& x)
        {
            if (_finish == _endofstorage)
            {
                size_t newCapacity = capacity() == 0 ? 4 : capacity() * 2;
                reserve(newCapacity);
            }

            *_finish = x;
            ++_finish;
        }
        void push_back(const T&& x)
        {
            cout<<"常数插入"<<endl;
            if (_finish == _endofstorage)
            {
                size_t newCapacity = capacity() == 0 ? 4 : capacity() * 2;
                reserve(newCapacity);
            }

            *_finish = x;
            ++_finish;
        }

        void pop_back()
        {
            assert(!empty());

            --_finish;
        }

        // 迭代器失效 : 扩容引起，野指针问题
        iterator insert(iterator pos, const T& val)
        {
            assert(pos >= _start);
            assert(pos < _finish);

            if (_finish == _endofstorage)
            {
                size_t len = pos - _start;
                size_t newCapacity = capacity() == 0 ? 4 : capacity() * 2;
                reserve(newCapacity);

                // 扩容会导致pos迭代器失效，需要更新处理一下
                pos = _start + len;
            }

            // 挪动数据
            iterator end = _finish - 1;
            while (end >= pos)
            {
                *(end + 1) = *end;
                --end;
            }

            *pos = val;
            ++_finish;

            return pos;
        }

        iterator erase(iterator pos)
        {
            assert(pos >= _start);
            assert(pos < _finish);


            iterator begin = pos+1;
            while (begin < _finish)
            {
                *(begin-1) = *(begin);
                ++begin;
            }

            --_finish;

            return pos;
        }

        void swap(vector<T>& v)
        {
            std::swap(_start, v._start);
            std::swap(_finish, v._finish);
            std::swap(_endofstorage, v._endofstorage);
        }

        void clear()
        {
            _finish = _start;
        }

    private:
        iterator _start;
        iterator _finish;
        iterator _endofstorage;
    };
    void test_vector1()
    {
        vector<int> v;
        int a=1;
        int b=2;
        int c=3;
        v.push_back(a);
        v.push_back(b);
        v.push_back(c);
        vector<int> v1(move(v));
    }
}

#endif //LESSON2_VECTOR_H
