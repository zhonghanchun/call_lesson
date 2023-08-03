//
// Created by 钟寒春 on 2023/3/25.
//

#ifndef LESSON2_SMARTPTR_H
#define LESSON2_SMARTPTR_H

#include <iostream>
#include <mutex>
using namespace std;
namespace zhc
{
    template<class T>
    class auto_ptr
    {
    public:
        auto_ptr(T* ptr):_ptr(ptr)
        {}
        auto_ptr<T> operator=(auto_ptr<T>& sp)
        {
            _ptr=sp._ptr;
        }
        auto_ptr(auto_ptr<T>& sp):_ptr(sp._ptr)
        {}
        ~auto_ptr()
        {
            cout<<"auto_ptr"<<endl;
            delete _ptr;
        }
        T& operator*()
        {
            return *_ptr;
        }
        T* operator->()
        {
            return _ptr;
        }
        T& operator[](size_t pos)
        {
            return _ptr[pos];
        }
    private:
        T* _ptr;
    };

    template<class T>
    class unique_ptr
    {
    public:
        unique_ptr(T* ptr):_ptr(ptr)
        {}
        unique_ptr(const unique_ptr<T>& ps)=delete;
        unique_ptr<T>& operator=(const unique_ptr<T>& ps)=delete;
        ~unique_ptr()
        {
            cout<<"unique_ptr delete"<<endl;
            delete _ptr;
        }
        T& operator*()
        {
            return *_ptr;
        }
        T* operator->()
        {
            return _ptr;
        }
        T& operator[](size_t pos)
        {
            return _ptr[pos];
        }
    private:
        T* _ptr;
    };

    template<class T>
    class DelFash
    {
    public:
        void operator()(T* del)
        {
            delete del;
        }
    };

    template<class T,class Del=DelFash<T>>
    class shared_ptr
    {
    public:
        shared_ptr(T* ptr= nullptr)
            :_ptr(ptr)
            ,_count_ptr(new int(1))
        {}
        shared_ptr(shared_ptr<T>& ps)
            :_ptr(ps._ptr)
            ,_count_ptr(ps._count_ptr)
        {
            (*_count_ptr)++;
        }
        shared_ptr<T>& operator=(shared_ptr<T>& ps)
        {
            if(_ptr!=ps._ptr) {
                if (--(*_count_ptr) == 0) {
                    //delete _ptr;
                    _del(_ptr);
                    delete _count_ptr;
                }
                _ptr = ps._ptr;
                _count_ptr = ps._count_ptr;
                (*_count_ptr)++;
            }
            return *this;
        }
        ~shared_ptr()
        {
            if(--(*_count_ptr)==0)
            {
                _del(_ptr);
                //delete _ptr;
                delete _count_ptr;
            }
        }
        size_t count()
        {
            return *_count_ptr;
        }

        T& operator*()
        {
            return *_ptr;
        }
        T* operator->()
        {
            return _ptr;
        }
        T& operator[](size_t pos)
        {
            return _ptr[pos];
        }
        T* get()
        {
            return _ptr;
        }
    private:
        T* _ptr;
        int* _count_ptr;
        Del _del;
    };

    template<class T>
    class weak_ptr
    {
    public:
        weak_ptr():_ptr(nullptr)
        {}
        weak_ptr(const shared_ptr<T>& ps):_ptr(ps.get())
        {}
        weak_ptr<T>& operator=(const shared_ptr<T>& ps)
        {
            _ptr=ps.get();
        }
        // 像指针一样
        T& operator*()
        {
            return *_ptr;
        }

        T* operator->()
        {
            return _ptr;
        }
    public:
        T*  _ptr;
    };

    //加锁的智能指针

    template<class T,class Del=DelFash<T>>
    class shared_mutex_ptr
    {
    public:
        shared_mutex_ptr(T* ptr= nullptr)
                :_ptr(ptr)
                ,_count_ptr(new int(1))
                ,_mtx(new mutex)
        {}
        shared_mutex_ptr(shared_mutex_ptr<T>& ps)
                :_ptr(ps._ptr)
                ,_count_ptr(ps._count_ptr)
                ,_mtx(ps._mtx)
        {
            _mtx->lock();
            (*_count_ptr)++;
            _mtx->unlock();
        }
        shared_mutex_ptr<T>& operator=(shared_mutex_ptr<T>& ps)
        {
            if(_ptr!=ps._ptr) {
                if (--(*_count_ptr) == 0) {
                    //delete _ptr;
                    _del(_ptr);
                    delete _count_ptr;
                }
                _ptr = ps._ptr;
                _count_ptr = ps._count_ptr;
                _mtx=ps._mtx;
                _mtx->lock();
                (*_count_ptr)++;
                _mtx->unlock();
            }
            return *this;
        }
        ~shared_mutex_ptr()
        {
            if(--(*_count_ptr)==0)
            {
                _del(_ptr);
                //delete _ptr;
                delete _count_ptr;
            }
        }
        size_t count()
        {
            return *_count_ptr;
        }

        T& operator*()
        {
            return *_ptr;
        }
        T* operator->()
        {
            return _ptr;
        }
        T& operator[](size_t pos)
        {
            return _ptr[pos];
        }
    private:
        T* _ptr;
        int* _count_ptr;
        mutex* _mtx;
        Del _del;
    };
}



#endif //LESSON2_SMARTPTR_H
