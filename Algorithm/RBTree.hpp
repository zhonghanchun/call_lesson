//
// Created by 钟寒春 on 2023/2/13.
//

#ifndef LESSON2_RBTREE_H
#define LESSON2_RBTREE_H
#include <iostream>
#include <assert.h>
using namespace std;
enum Color
{
    RED,
    BLACK
};
template<class T>
struct RBTreeNode
{
    RBTreeNode<T>* _left;
    RBTreeNode<T>* _right;
    RBTreeNode<T>* _parent;
    T _data;
    Color _col;
    RBTreeNode(const T& data)
            :_left(nullptr),_right(nullptr),_parent(nullptr),_col(RED),_data(data)
    {}
};
template<class T,class Ref,class Ptr>
struct _reverse_iterator
{
    typedef RBTreeNode<T> Node;
    typedef _reverse_iterator<T,Ref,Ptr> Self;
    typedef _reverse_iterator<T,T&,T*> reverse_iteratior;
    Node* _node;
    _reverse_iterator(const reverse_iteratior& s)
        :_node(s._node)
    {}
    _reverse_iterator(Node* node)
        :_node(node)   
    {}
    Ref operator*()
    {
        return _node->_data;
    }
    Ptr operator->()
    {
        return &_node->_data;
    }
    Ref operator*()const 
    {
        return _node->_data;
    }
    Ptr operator->()const
    {
        return &_node->_data;
    }
    bool operator!=(const Self& s)const 
    {
        return _node!=s._node;
    }
    Self& operator++()
    {
        if(_node->_left)
        {
            Node* right=_node->_left;
            while(right&&right->_right)
            {
                right=right->_right;
            }
            _node=right;
        }
        else 
        {
            Node* parent=_node->_parent;
            Node* cur=_node;
            while(parent&&parent->_left==cur)
            {
                parent=parent->_parent;
                cur=cur->_parent;
            }
            _node=parent;
        }
        return *this;
    }
     Self& operator--()
    {
        if(_node->right)
        {
            Node* left=_node->_right;
            while(left&&left->_left)
            {
                left=left->_left;
            }            
            _node=left;
        }
        else
        {
            Node* parent=_node->_parent;
            Node* cur=_node;
            while(parent&&parent->_right==cur)
            {
                parent=parent->_parent;
                cur=cur->_parent;
            }
            _node=parent;
        }
        return *this;
    }

};
template<class T,class Ref,class Ptr>
struct RBTree_iterator
{
    typedef RBTreeNode<T> Node;
    typedef RBTree_iterator<T,Ref,Ptr> Self;
    typedef RBTree_iterator<T,T&,T*> iterator;
    RBTree_iterator(const iterator& s)
        :_node(s._node)
    {}
    Node* _node;
    //KeyOFT kot;
    RBTree_iterator(Node* node)
        :_node(node)
    {}
    Ref operator*()
    {
        return _node->_data;
    }
    Ptr operator->()
    {
        return &_node->_data;
    }
    Ref operator*()const
    {
        return _node->_data;
    }
    Ptr operator->()const
    {
        return &_node->_data;
    }
    Self& operator++()
    {
        if(_node->_right)
        {
            Node* left=_node->_right;
            while(left&&left->_left)
            {
                left=left->_left;
            }
            _node=left;
        }
        else
        {
            Node* parent=_node->_parent;
            Node* cur=_node;
            while(parent&&parent->_right==cur)
            {
                parent=parent->_parent;
                cur=cur->_parent;
            }
            _node=parent;
        }
        return *this;
    }
    Self& operator--()
    {
        if(_node->_left)
        {
            Node* right=_node->_left;
            while(right&&right->_right)
            {
                right=right->_right;
            }
            _node=right;
        }
        else
        {
            Node* parent=_node->_parent;
            Node* cur=_node;
            while(parent&&parent->_left==cur)
            {
                parent=parent->_parent;
                cur=cur->_parent;
            }
            _node=parent;
        }
        return *this;
    }
    bool operator!=(const Self& s)const
    {
        return _node!=s._node;
    }
};
template <class K,class T,class KeyOFT>
class RBTree
{
public:
    typedef RBTreeNode<T> Node;
    typedef RBTree_iterator<T,T&,T*> iterator;
    typedef RBTree_iterator<T,const T&,const T*> const_iterator;
    typedef _reverse_iterator<T,T&,T*> reverse_iterator;
    typedef _reverse_iterator<T,const T&,const T*> const_reverse_iterator;
    reverse_iterator rbegin()
    {
        Node* right=_root;
        while(right&&right->_right)
        {
            right=right->_right;
        }
        return reverse_iterator(right);
    }
    reverse_iterator rend()
    {
        return reverse_iterator(nullptr);
    }
    const_reverse_iterator rbegin()const
    {
        Node* right=_root;
        while(right&&right->_right)
        {
            right=right->_right;
        }
        return const_reverse_iterator(right);
    }
    const_reverse_iterator rend()const
    {
        return const_reverse_iterator(nullptr);
    }
    const_iterator begin()const
    {
        Node* left=_root;
        while(left&&left->_left)
            left=left->_left;
        return const_iterator(left);
    }

    const_iterator end()const
    {
        return const_iterator(nullptr);
    }

    iterator begin() {
        Node* left=_root;
        while(left&&left->_left)
            left=left->_left;
        return iterator(left);
    }
    iterator end()
    {
        return iterator(nullptr);
    }

    RBTree()
    {
        _root=nullptr;
    }
    pair<iterator, bool> insert(const T& data)
    {
        if(_root== nullptr)
        {
            _root=new Node(data);
            _root->_col=BLACK;
            return make_pair(iterator(_root),true);
        }
        KeyOFT kot;
        Node* parent= nullptr;
        Node* cur=_root;
        while(cur)
        {
            if(kot(cur->_data) > kot(data))
            {
                parent=cur;
                cur=cur->_left;
            }
            else if(kot(cur->_data) < kot(data))
            {
                parent=cur;
                cur=cur->_right;
            }
            else
                return make_pair(iterator(cur),false);
        }
        cur=new Node(data);
        Node* newnode=cur;
        if(kot(parent->_data) > kot(data))
        {
            parent->_left=cur;
            cur->_parent=parent;
        }
        else
        {
            parent->_right=cur;
            cur->_parent=parent;
        }
        while(parent&&parent->_col==RED)
        {
            Node* grandfater=parent->_parent;
            Node* uncle= nullptr;
            if(grandfater&&grandfater->_left==parent)
            {
                uncle=grandfater->_right;
                if(uncle&&uncle->_col==RED)
                {
                    parent->_col=uncle->_col=BLACK;
                    grandfater->_col=RED;
                    cur=grandfater;
                    parent=cur->_parent;
                }
                else {
                    if (parent->_left == cur) {
                        RotateR(grandfater);
                        parent->_col = BLACK;
                        cur->_col = grandfater->_col = RED;
                    } else {
                        RotateL(parent);
                        RotateR(grandfater);
                        cur->_col = BLACK;
                        parent->_col = grandfater->_col = RED;
                    }
                    break;
                }
            }
            else if(grandfater&&grandfater->_right==parent)
            {
                uncle=grandfater->_left;
                if(uncle&&uncle->_col==RED)
                {
                    uncle->_col=parent->_col=BLACK;
                    grandfater->_col=RED;
                    cur=grandfater;
                    parent=cur->_parent;
                }
                else
                {
                    if(parent->_right==cur)
                    {
                        RotateL(grandfater);
                        parent->_col=BLACK;
                        cur->_col=grandfater->_col=RED;
                    }
                    else
                    {
                        RotateR(parent);
                        RotateL(grandfater);
                        cur->_col=BLACK;
                        parent->_col=grandfater->_col=RED;
                    }
                    break;
                }
            }
            else
            {
                assert(false);
            }
        }

        _root->_col=BLACK;
        return make_pair(iterator(newnode),true);
    }

    void RotateL(Node *parent) {
        Node *gparent = parent->_parent;
        Node *subR = parent->_right;
        Node *subRL = subR->_left;
        parent->_right = subRL;
        if (subRL)
            subRL->_parent = parent;
        Node *ppNode = parent->_parent;
        subR->_left = parent;
        parent->_parent = subR;
        if (ppNode == nullptr) {
            _root = subR;
            _root->_parent = nullptr;
        } else {
            if (ppNode->_left == parent) {
                ppNode->_left = subR;
            } else {
                ppNode->_right = subR;
            }

            subR->_parent = ppNode;
        }
    }

    void RotateR(Node *parent) {
        Node *gparent = parent->_parent;
        Node *subL = parent->_left;
        Node *subLR = subL->_right;
        parent->_left = subLR;
        if (subLR) {
            subLR->_parent = parent;
        }
        Node *ppNode = parent->_parent;
        subL->_right = parent;
        parent->_parent = subL;
        //if (_root == parent)
        if (ppNode == nullptr) {
            _root = subL;
            _root->_parent = nullptr;
        } else {
            if (ppNode->_left == parent) {
                ppNode->_left = subL;
            } else {
                ppNode->_right = subL;
            }

            subL->_parent = ppNode;
        }
    }
    bool IsRBTree()
    {
        if(_root== nullptr)
            return true;
        if(_root->_col == RED)
            return false;
        int ref=0;
        Node* left=_root;
        while(left)
        {
            if(left->_col==BLACK)
                ++ref;
            left=left->_left;
        }

        return check(_root,0,ref);
    }
    bool check(Node* root,int BlackNum,const int ref)
    {
        if(root== nullptr)
        {
            if(BlackNum!=ref) {
                cout<<"黑色路径有问题"<<endl;
                return false;
            }
            return true;
        }
        if(root->_col==RED&&root->_parent->_col==RED) {
            cout<<"红色有连续问题"<<endl;
            return false;
        }
        if(root->_col==BLACK)
            BlackNum++;
        return check(root->_left,BlackNum,ref)
               && check(root->_right,BlackNum,ref);
    }
    void Inorder()
    {
        _Inorder(_root);
    }
private:
    void _Inorder(Node* root)
    {
        if(root== nullptr)
            return;
        _Inorder(root->_left);
        cout<<root->_kv.first<<"->"<<root->_kv.second<<endl;
        _Inorder(root->_right);
    }
private:
    Node* _root;
};
/*
enum Color
{
    RED,
    BLACK
};
template<class K,class V>
struct RBTreeNode
{
    RBTreeNode<K,V>* _left;
    RBTreeNode<K,V>* _right;
    RBTreeNode<K,V>* _parent;
    pair<K,V> _kv;
    Color _col;
    RBTreeNode(const pair<K,V>& kv)
    :_left(nullptr),_right(nullptr),_parent(nullptr),_col(RED),_kv(kv)
    {}
};
template <class K,class V>
class RBTree
{
    typedef RBTreeNode<K,V> Node;
public:
    RBTree()
    {
        _root=nullptr;
    }
    bool insert(const pair<K,V>& kv)
    {
        if(_root== nullptr)
        {
            _root=new Node(kv);
            _root->_col=BLACK;
            return true;
        }
        Node* parent= nullptr;
        Node* cur=_root;
        while(cur)
        {
            if(cur->_kv.first>kv.first)
            {
                parent=cur;
                cur=cur->_left;
            }
            else if(cur->_kv.first<kv.first)
            {
                parent=cur;
                cur=cur->_right;
            }
            else
                return false;
        }
        cur=new Node(kv);
        if(parent->_kv.first>kv.first)
        {
            parent->_left=cur;
            cur->_parent=parent;
        }
        else
        {
            parent->_right=cur;
            cur->_parent=parent;
        }
        while(parent&&parent->_col==RED)
        {
            Node* grandfater=parent->_parent;
            Node* uncle= nullptr;
            if(grandfater&&grandfater->_left==parent)
            {
                uncle=grandfater->_right;
                if(uncle&&uncle->_col==RED)
                {
                    parent->_col=uncle->_col=BLACK;
                    grandfater->_col=RED;
                    cur=grandfater;
                    parent=cur->_parent;
                }
                else {
                    if (parent->_left == cur) {
                        RotateR(grandfater);
                        parent->_col = BLACK;
                        cur->_col = grandfater->_col = RED;
                    } else {
                        RotateL(parent);
                        RotateR(grandfater);
                        cur->_col = BLACK;
                        parent->_col = grandfater->_col = RED;
                    }
                    break;
                }
            }
            else if(grandfater&&grandfater->_right==parent)
            {
                uncle=grandfater->_left;
                if(uncle&&uncle->_col==RED)
                {
                    uncle->_col=parent->_col=BLACK;
                    grandfater->_col=RED;
                    cur=grandfater;
                    parent=cur->_parent;
                }
                else
                {
                    if(parent->_right==cur)
                    {
                        RotateL(grandfater);
                        parent->_col=BLACK;
                        cur->_col=grandfater->_col=RED;
                    }
                    else
                    {
                        RotateR(parent);
                        RotateL(grandfater);
                        cur->_col=BLACK;
                        parent->_col=grandfater->_col=RED;
                    }
                    break;
                }
            }
            else
            {
                assert(false);
            }
        }

        _root->_col=BLACK;
        return true;
    }

    void RotateL(Node *parent) {
        Node *gparent = parent->_parent;
        Node *subR = parent->_right;
        Node *subRL = subR->_left;
        parent->_right = subRL;
        if (subRL)
            subRL->_parent = parent;
        Node *ppNode = parent->_parent;
        subR->_left = parent;
        parent->_parent = subR;
        if (ppNode == nullptr) {
            _root = subR;
            _root->_parent = nullptr;
        } else {
            if (ppNode->_left == parent) {
                ppNode->_left = subR;
            } else {
                ppNode->_right = subR;
            }

            subR->_parent = ppNode;
        }
    }

    void RotateR(Node *parent) {
        Node *gparent = parent->_parent;
        Node *subL = parent->_left;
        Node *subLR = subL->_right;
        parent->_left = subLR;
        if (subLR) {
            subLR->_parent = parent;
        }
        Node *ppNode = parent->_parent;
        subL->_right = parent;
        parent->_parent = subL;
        //if (_root == parent)
        if (ppNode == nullptr) {
            _root = subL;
            _root->_parent = nullptr;
        } else {
            if (ppNode->_left == parent) {
                ppNode->_left = subL;
            } else {
                ppNode->_right = subL;
            }

            subL->_parent = ppNode;
        }
    }
    bool IsRBTree()
    {
        if(_root== nullptr)
            return true;
        if(_root->_col == RED)
            return false;
        int ref=0;
        Node* left=_root;
        while(left)
        {
            if(left->_col==BLACK)
                ++ref;
            left=left->_left;
        }

        return check(_root,0,ref);
    }
    bool check(Node* root,int BlackNum,const int ref)
    {
        if(root== nullptr)
        {
            if(BlackNum!=ref) {
                cout<<"黑色路径有问题"<<endl;
                return false;
            }
            return true;
        }
        if(root->_col==RED&&root->_parent->_col==RED) {
            cout<<"红色有连续问题"<<endl;
            return false;
        }
        if(root->_col==BLACK)
            BlackNum++;
        return check(root->_left,BlackNum,ref)
                && check(root->_right,BlackNum,ref);
    }
    void Inorder()
    {
        _Inorder(_root);
    }
private:
    void _Inorder(Node* root)
    {
        if(root== nullptr)
            return;
        _Inorder(root->_left);
        cout<<root->_kv.first<<"->"<<root->_kv.second<<endl;
        _Inorder(root->_right);
    }
private:
    Node* _root;
};
*/
#endif //LESSON2_RBTREE_H
