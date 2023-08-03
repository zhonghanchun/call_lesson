//
// Created by 钟寒春 on 2023/3/10.
//

#ifndef LESSON2_STRING_H
#define LESSON2_STRING_H
#include <iostream>
using namespace std;
namespace zhc
{
    class string
    {
    public:
        typedef char* iterator;
//        friend istream& operator>>(istream& in, string& s);
//        friend ostream& operator<<(ostream& out, const string& s);
//        friend istream& getline(istream& in, string& s);
        iterator begin()
        {
            return _str;
        }

        iterator end()
        {
            return _str + _size;
        }
        bool empty()const
        {
            return _size==0&&_capacity==0;
        }
        /*	string()
            {
            _str = new char[1];
            _str[0] = '\0';
            _capacity = _size = 0;
            }*/

        // '\0'
        // "\0"
        // ""
        string(const char* str = "")
        {
            _size = strlen(str);
            _capacity = _size;
            _str = new char[_capacity + 1];

            strcpy(_str, str);
        }

        void swap(string& s)
        {
            std::swap(_str, s._str);
            std::swap(_size, s._size);
            std::swap(_capacity, s._capacity);
        }

        // 现代写法
        // s2(s1)
        string(const string& s)
                :_str(nullptr)
                , _size(0)
                , _capacity(0)
        {
            string tmp(s._str); // 构造函数
            //this->swap(tmp);
            swap(tmp);
        }
        string(string&& s)
                :_str(nullptr)
                , _size(0)
                , _capacity(0)
        {
            cout<<"移动拷贝构造"<<endl;
            //string tmp(s._str); // 构造函数
            this->swap(s);
           // swap(tmp);
        }

        // s2(s1)
        // 传统写法
        /*string(const string& s)
        {
        _str = new char[s._capacity + 1];
        _capacity = s._capacity;
        _size = s._size;

        strcpy(_str, s._str);
        }*/

        // s1 = s3;
        /*string& operator=(const string& s)
        {
            if (this != &s)
            {
                char* tmp = new char[s._capacity + 1];
                strcpy(tmp, s._str);

                delete[] _str;
                _str = tmp;

                _size = s._size;
                _capacity = s._capacity;
            }

            return *this;
        }*/

        // s1 = s3;
        //string& operator=(const string& s)
        //{
        //	if (this != &s)
        //	{
        //		//string tmp(s._str);
        //		string tmp(s);
        //		swap(tmp);
        //	}

        //	return *this;
        //}

        // s1 = s3;
        string& operator=(string s)
        {
            swap(s);
            return *this;
        }
        string& operator=(string&& s)
        {
            swap(s);
            return *this;
        }
        ~string()
        {
            delete[] _str;
            _str = nullptr;
            _size = _capacity = 0;
        }

        const char* c_str() const
        {
            return _str;
        }

        size_t size() const
        {
            return _size;
        }

        size_t capacity() const
        {
            return _capacity;
        }

        // 普通对象：可读可写
        char& operator[](size_t pos)
        {
            assert(pos < _size);
            return _str[pos];
        }

        // const对象：只读
        const char& operator[](size_t pos) const
        {
            assert(pos < _size);
            return _str[pos];
        }

        void reserve(size_t n)
        {
            if (n > _capacity)
            {
                char* tmp = new char[n + 1];
                strcpy(tmp, _str);
                delete[] _str;
                _str = tmp;

                _capacity = n;
            }
        }

        void resize(size_t n, char ch = '\0')
        {
            if (n > _size)
            {
                reserve(n);
                for (size_t i = _size; i < n; ++i)
                {
                    _str[i] = ch;
                }

                _size = n;
                _str[_size] = '\0';
            }
            else
            {
                _str[n] = '\0';
                _size = n;
            }
        }

        void push_back(char ch)
        {
            if (_size == _capacity)
            {
                size_t newCapacity = _capacity == 0 ? 4 : _capacity * 2;
                reserve(newCapacity);
            }

            _str[_size] = ch;
            ++_size;
            _str[_size] = '\0';
        }

        void append(const char* str)
        {
            size_t len = strlen(str);
            if (_size + len > _capacity)
            {
                reserve(_size + len);
            }

            strcpy(_str + _size, str);
            _size += len;
        }

        string& operator+=(char ch)
        {
            push_back(ch);
            return *this;
        }

        string& operator+=(const char* str)
        {
            append(str);
            return *this;
        }

        string& insert(size_t pos, char ch)
        {
            assert(pos <= _size);

            if (_size == _capacity)
            {
                size_t newCapacity = _capacity == 0 ? 4 : _capacity * 2;
                reserve(newCapacity);
            }

            // 挪动数据
            /*int end = _size;
            while (end >= (int)pos)
            {
            _str[end + 1] = _str[end];
            --end;
            }*/

            size_t end = _size + 1;
            while (end > pos)
            {
                _str[end] = _str[end - 1];
                --end;
            }

            _str[pos] = ch;
            ++_size;

            return *this;
        }

        string& insert(size_t pos, const char* str)
        {
            size_t len = strlen(str);
            if (_size + len > _capacity)
            {
                reserve(_size + len);
            }

            /*	int end = _size;
                while (end >= (int)pos)
                {
                _str[end + len] = _str[end];
                --end;
                }*/

            size_t end = _size + len;
            while (end > pos + len - 1)
            {
                _str[end] = _str[end - len];
                --end;
            }

            strncpy(_str + pos, str, len);
            _size += len;

            return *this;
        }

        // 休息到16：04继续
        string& erase(size_t pos, size_t len = npos)
        {
            assert(pos < _size);

            if (len == npos || pos + len >= _size)
            {
                _str[pos] = '\0';
                _size = pos;
            }
            else
            {
                strcpy(_str + pos, _str + pos + len);
                _size -= len;
            }

            return *this;
        }

        size_t find(char ch, size_t pos = 0) const
        {
            assert(pos < _size);
            while (pos < _size)
            {
                if (_str[pos] == ch)
                {
                    return pos;
                }

                ++pos;
            }

            return npos;
        }

        // kmp
        // 聪明 or 努力
        size_t find(const char* str, size_t pos = 0) const
        {
            assert(pos < _size);
            const char* ptr = strstr(_str + pos, str);
            if (ptr == nullptr)
            {
                return npos;
            }
            else
            {
                return ptr - _str;
            }
        }

        void clear()
        {
            _size = 0;
            _str[0] = '\0';
        }
    private:
        char* _str;
        size_t _size;
        size_t _capacity;

        const static size_t npos = -1;

        /*const static size_t N = 10;
        int a[N];*/
        //const static double x;
    };

    ostream& operator<<(ostream& out, const string& s)
    {
        for (size_t i = 0; i < s.size(); ++i)
        {
            out << s[i];
        }

        return out;
    }

    istream& operator>>(istream& in, string& s)
    {
        s.clear();
        //char ch = in.get();
        //while (ch != ' ' && ch != '\n')
        //{
        //	s += ch;
        //	//in >> ch;
        //	ch = in.get();
        //}

        char buff[128] = {'\0'};
        size_t i = 0;
        char ch = in.get();
        while (ch != ' ' && ch != '\n')
        {
            if (i == 127)
            {
                // 满了
                s += buff;
                i = 0;
            }

            buff[i++] = ch;

            ch = in.get();
        }

        if (i > 0)
        {
            buff[i] = '\0';
            s += buff;
        }

        return in;
    }
    //const double string::x = 1.1;

    /*void func(const string& s)
    {
    for (size_t i = 0; i < s.size(); ++i)
    {
    cout << s[i] << " ";
    }
    cout << s.c_str() << endl;

    string::iterator it1 = s.begin();
    while (it1 != s.end())
    {
    (*it1)--;

    ++it1;
    }
    cout << s.c_str() << endl;

    for (auto ch : s)
    {
    cout << ch << " ";
    }
    cout << endl;
    }*/
    void Test_string()
    {
        string s("1");
        cout<<s.empty()<<endl;
        //string s1;
        //string s2(move(s1));

    }
}
#endif //LESSON2_STRING_H
