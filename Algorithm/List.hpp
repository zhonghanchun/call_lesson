#pragma once
#include <iostream>
#include <assert.h>

namespace zhc
{
	template<class T>
	struct list_node
	{
		list_node<T>* _next;
		list_node<T>* _prev;
		T _data;

		list_node(const T& x)
			:_next(nullptr)
			, _prev(nullptr)
			, _data(x)
		{}
	};

	// 同一个类模板实例化出的两个类型
	// typedef __list_iterator<T, T&, T*> iterator;
	// typedef __list_iterator<T, const T&, const T*> const_iterator;
	template<class T, class Ref, class Ptr>
	struct __list_iterator
	{
		typedef list_node<T> node;
		typedef __list_iterator<T, Ref, Ptr> Self;
		node* _pnode;

		__list_iterator(node* p)
			:_pnode(p)
		{}

		Ptr operator->()
		{
			return &_pnode->_data;
		}

		// iterator it
		// *it
		// ++it;
		Ref operator*()
		{
			return _pnode->_data;
		}

		// const iterator cit
		// *cit
		// ++cit 这样的话，可以解引用，但是不能++
		/*const T& operator*() const
		{
		return _pnode->_data;
		}*/

		// ++it
		Self& operator++()
		{
			_pnode = _pnode->_next;
			return *this;
		}

		// it++
		Self operator++(int)
		{
			Self tmp(*this);
			_pnode = _pnode->_next;		
			return tmp;
		}

		Self& operator--()
		{
			_pnode = _pnode->_prev;
			return *this;
		}

		Self operator--(int)
		{
			Self tmp(*this);
			_pnode = _pnode->_prev;
			return tmp;
		}

		bool operator!=(const Self& it) const
		{
			return _pnode != it._pnode;
		}

		bool operator==(const Self& it) const
		{
			return _pnode == it._pnode;
		}
	};

	// 跟普通迭代器的区别：遍历，不能用*it修改数据
	/*template<class T>
	struct __list_const_iterator
	{
		typedef list_node<T> node;
		node* _pnode;

		__list_const_iterator(node* p)
			:_pnode(p)
		{}

		const T& operator*()
		{
			return _pnode->_data;
		}

		__list_const_iterator<T>& operator++()
		{
			_pnode = _pnode->_next;
			return *this;
		}

		__list_const_iterator<T>& operator--()
		{
			_pnode = _pnode->_prev;
			return *this;
		}

		bool operator!=(const __list_const_iterator<T>& it)
		{
			return _pnode != it._pnode;
		}
	};*/

	//vector<int>
	//vector<string>
	//vector<vector<int>>

	// 类名  类型
	// 普通类  类名 等价于 类型
	// 类模板  类名  不等价于  类型
	// 如：list模板 类名list  类型list<T> 
	// 类模板里面可以用类名代表类型，但是建议不要那么用
	template<class T>
	class list
	{
		typedef list_node<T> node;
	public:
		typedef __list_iterator<T, T&, T*> iterator;
		//typedef __list_const_iterator<T> const_iterator;
		typedef __list_iterator<T, const T&, const T*> const_iterator;

		const_iterator begin() const
		{
			return const_iterator(_head->_next);
		}

		const_iterator end() const
		{
			return const_iterator(_head);
		}

		iterator begin()
		{
			return iterator(_head->_next);
		}

		iterator end()
		{
			//iterator it(_head);
			//return it;
			return iterator(_head);
		}

		void empty_initialize()
		{
			_head = new node(T());
			_head->_next = _head;
			_head->_prev = _head;

			_size = 0;
		}

		list()
		{
			empty_initialize();
		}

		//list(const list<T>& lt)
		//{
		//	empty_initialize();

		//	for (const auto& e : lt)
		//	{
		//		push_back(e);
		//	}
		//}

		//// lt1 = lt3
		//list<T>& operator=(const list<T>& lt)
		//{
		//	if (this != &lt)
		//	{
		//		clear();
		//		for (const auto& e : lt)
		//		{
		//			push_back(e);
		//		}
		//	}

		//	return *this;
		//}

		template <class InputIterator>  
		list(InputIterator first, InputIterator last)
		{
			empty_initialize();
			while (first != last)
			{
				push_back(*first);
				++first;
			}
		}

		void swap(list<T>& lt)
		{
			std::swap(_head, lt._head);
			std::swap(_size, lt._size);
		}

		// 现在写法
		// lt2(lt1)
		list(const list<T>& lt)
		//list(const list& lt) // 不建议
		{
			empty_initialize();

			list<T> tmp(lt.begin(), lt.end());
			swap(tmp);
		}

		// lt3 = lt1
		list<T>& operator=(list<T> lt)
		//list& operator=(list lt) // 不建议
		{
			swap(lt);
			return *this;
		}

		size_t size() const
		{
			return _size;
		}

		bool empty() const
		{
			return _size == 0;
		}

		~list()
		{
			clear();

			delete _head;
			_head = nullptr;
		}

		void clear()
		{
			iterator it = begin();
			while (it != end())
			{
				it = erase(it);
			}
		}

		void push_back(const T& x)
		{
			//node* newnode = new node(x);
			//node* tail = _head->_prev;
			//// _head         tail   newnode
			//tail->_next = newnode;
			//newnode->_prev = tail;
			//newnode->_next = _head;
			//_head->_prev = newnode;

			insert(end(), x);
		}

		void push_front(const T& x)
		{
			insert(begin(), x);
		}

		void pop_front()
		{
			erase(begin());
		}

		void pop_back()
		{
			erase(--end());
		}
		
		iterator insert(iterator pos, const T& x)
		{
			node* newnode = new node(x);
			node* cur = pos._pnode;
			node* prev = cur->_prev;

			// prev newnode cur
			prev->_next = newnode;
			newnode->_prev = prev;
			newnode->_next = cur;
			cur->_prev = newnode;

			++_size;

			return iterator(newnode);
		}

		iterator erase(iterator pos)
		{
			assert(pos != end());

			node* prev = pos._pnode->_prev;
			node* next = pos._pnode->_next;

			prev->_next = next;
			next->_prev = prev;

			delete pos._pnode;
			--_size;

			return iterator(next);
		}

	private:
		node* _head;
		size_t _size;
	};
}
