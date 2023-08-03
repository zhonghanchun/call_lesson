//
// Created by 钟寒春 on 2023/2/10.
//

#ifndef LESSON2_AVLTREE_H
#define LESSON2_AVLTREE_H
#include <iostream>
#include <assert.h>

using namespace std;
namespace zhc {
    template<class K, class V>
    struct AVLTreeNode {
        pair<K, V> _kv;
        AVLTreeNode<K, V> *_left;
        AVLTreeNode<K, V> *_right;
        AVLTreeNode<K, V> *_parent;
        int _bf;

        AVLTreeNode(const pair<K, V> &kv)
                : _kv(kv), _left(nullptr), _right(nullptr), _parent(nullptr), _bf(0) {}
    };

    template<class K, class V>
    class AVLTree {
        typedef AVLTreeNode<K, V> Node;
    public:
        bool insert(const pair<K, V> &kv) {
            if (_root == nullptr) {
                _root = new Node(kv);
                return true;
            }
            Node *cur = _root;
            Node *parent = nullptr;
            while (cur) {
                if (cur->_kv.first < kv.first) {
                    parent = cur;
                    cur = cur->_right;
                } else if (cur->_kv.first > kv.first) {
                    parent = cur;
                    cur = cur->_left;
                } else
                    return false;
            }
            cur = new Node(kv);
            if (parent->_kv.first > kv.first) {
                parent->_left = cur;
                cur->_parent = parent;
            } else {
                parent->_right = cur;
                cur->_parent = parent;
            }
            while (parent) {
                if (cur == parent->_left)
                    parent->_bf--;
                else
                    parent->_bf++;
                if (parent->_bf == 0)
                    break;
                else if (parent->_bf == 1 || parent->_bf == -1) {
                    cur = parent;
                    parent = parent->_parent;
                } else if (parent->_bf == 2 || parent->_bf == -2) {
                    //旋转
                    //1.左旋
                    if (parent->_bf == 2 && parent->_right->_bf == 1) {
                        RotateL(parent);
                        //break;
                    }
                        //2.右旋
                    else if (parent->_bf == -2 && parent->_left->_bf == -1) {
                        RotateR(parent);
                        //break;
                    }
                        //3.双旋
                    else if (parent->_bf == 2 && cur->_bf == -1) {
                        RotateRL(parent);
                        //break;
                    } else if (parent->_bf == -2 && cur->_bf == 1) {
                        RotateLR(parent);
                        //break;
                    } else
                        assert(false);
                    break;
                } else {
                    assert(false);
                }
            }
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
            parent->_bf = subR->_bf = 0;
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
            subL->_bf = parent->_bf = 0;
        }

        void RotateLR(Node *parent) {
            Node *subL = parent->_left;
            Node *subLR = subL->_right;
            int bf = subLR->_bf;
            RotateL(parent->_left);
            RotateR(parent);
            if (bf == 0) {
                parent->_bf = 0;
                subLR->_bf = 0;
                subL->_bf = 0;
            } else if (bf == -1) {
                parent->_bf = 1;
                subL->_bf = 0;
                subLR->_bf = 0;
            } else if (bf == 1) {
                subL->_bf = -1;
                subLR->_bf = 0;
                parent->_bf = 0;
            } else {
                assert(false);
            }
        }

        void RotateRL(Node *parent) {
            Node *subR = parent->_right;
            Node *subRL = subR->_left;
            int bf = subRL->_bf;
            RotateR(parent->_right);
            RotateL(parent);
            if (bf == 1) {
                parent->_bf = -1;
                subR->_bf = 0;
                subRL->_bf = 0;
            } else if (bf == -1) {
                parent->_bf = 0;
                subR->_bf = 1;
                subRL->_bf = 0;
            } else if (bf == 0) {
                parent->_bf = 0;
                subR->_bf = 0;
                subRL->_bf = 0;
            } else {
                assert(false);
            }
        }

        bool IsBalance() {
            return IsBalance(_root);
        }

        bool IsBalance(Node *root) {
            if (root == nullptr)
                return true;
            int leftHight = Hight(root->_left);
            int rightHight = Hight(root->_right);
          //  if (rightHight - leftHight != root->_bf) {
                //cout << "平衡因子异常:" << root->_kv.first << endl;
           // }
            return abs(rightHight - leftHight) < 2
                   && IsBalance(root->_left)
                   && IsBalance(root->_right);
        }

        int Hight(Node *root) {
            if (root == nullptr)
                return 0;
            int left = Hight(root->_left);
            int right = Hight(root->_right);
            return left > right ? left + 1 : right + 1;
        }

        void Inorder() {
            _Inorder(_root);
        }

    private:
        void _Inorder(Node *root) {
            if (root == nullptr)
                return;
            _Inorder(root->_left);
            cout << root->_kv.first << ":" << root->_kv.second << endl;
            _Inorder(root->_right);
        }

    private:
        Node *_root = nullptr;
    };
}
namespace bit {
    template<class K, class V>
    struct AVLTreeNode {
        pair<K, V> _kv;
        AVLTreeNode<K, V> *_left;
        AVLTreeNode<K, V> *_right;
        AVLTreeNode<K, V> *_parent;

        int _bf;  // balance factor

        AVLTreeNode(const pair<K, V> &kv)
                : _kv(kv), _left(nullptr), _right(nullptr), _parent(nullptr), _bf(0) {}
    };

    template<class K, class V>
    struct AVLTree {
        typedef AVLTreeNode<K, V> Node;
    public:
        bool Insert(const pair<K, V> &kv) {
            if (_root == nullptr) {
                _root = new Node(kv);
                return true;
            }

            Node *parent = nullptr;
            Node *cur = _root;
            while (cur) {
                if (cur->_kv.first < kv.first) {
                    parent = cur;
                    cur = cur->_right;
                } else if (cur->_kv.first > kv.first) {
                    parent = cur;
                    cur = cur->_left;
                } else {
                    return false;
                }
            }

            cur = new Node(kv);
            if (parent->_kv.first < kv.first) {
                parent->_right = cur;
                cur->_parent = parent;
            } else {
                parent->_left = cur;
                cur->_parent = parent;
            }

            // 1、更新平衡因子
            while (parent) // parent为空，也就更新到根
            {
                // 新增在右，parent->bf++;
                // 新增在左，parent->bf--;
                if (cur == parent->_left) {
                    parent->_bf--;
                } else {
                    parent->_bf++;
                }

                // 是否继续更新依据：子树的高度是否变化
                // 1、parent->_bf == 0说明之前parent->_bf是 1 或者 -1
                // 说明之前parent一边高一边低，这次插入填上矮的那边，parent所在子树高度不变，不需要继续往上更新
                // 2、parent->_bf == 1 或 -1 说明之前是parent->_bf == 0，两边一样高，现在插入一边更高了，
                // parent所在子树高度变了，继续往上更新
                // 3、parent->_bf == 2 或 -2，说明之前parent->_bf == 1 或者 -1，现在插入严重不平衡，违反规则
                // 就地处理--旋转

                // 旋转：
                // 1、让这颗子树左右高度不超过1
                // 2、旋转过程中继续保持他是搜索树
                // 3、更新调整孩子节点的平衡因子
                // 4、让这颗子树的高度跟插入前保持一致
                if (parent->_bf == 0) {
                    break;
                } else if (parent->_bf == 1 || parent->_bf == -1) {
                    cur = parent;
                    parent = parent->_parent;
                } else if (parent->_bf == 2 || parent->_bf == -2) {
                    // 旋转
                    if (parent->_bf == 2 && cur->_bf == 1) {
                        RotateL(parent);
                    } else if (parent->_bf == -2 && cur->_bf == -1) {
                        RotateR(parent);
                    } else if (parent->_bf == -2 && cur->_bf == 1) {
                        RotateLR(parent);
                    } else if (parent->_bf == 2 && cur->_bf == -1) {
                        RotateRL(parent);
                    } else {
                        assert(false);
                    }

                    break;
                } else {
                    assert(false);
                }
            }

            return true;
        }

        void RotateL(Node *parent) {
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

            parent->_bf = subR->_bf = 0;
        }

        void RotateR(Node *parent) {
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

            subL->_bf = parent->_bf = 0;
        }

        void RotateLR(Node *parent) {
            Node *subL = parent->_left;
            Node *subLR = subL->_right;
            int bf = subLR->_bf;

            RotateL(parent->_left);
            RotateR(parent);

            if (bf == -1) // subLR左子树新增
            {
                subL->_bf = 0;
                parent->_bf = 1;
                subLR->_bf = 0;
            } else if (bf == 1) // subLR右子树新增
            {
                parent->_bf = 0;
                subL->_bf = -1;
                subLR->_bf = 0;
            } else if (bf == 0) // subLR自己就是新增
            {
                parent->_bf = 0;
                subL->_bf = 0;
                subLR->_bf = 0;
            } else {
                assert(false);
            }
        }

        void RotateRL(Node *parent) {
            Node *subR = parent->_right;
            Node *subRL = subR->_left;
            int bf = subRL->_bf;
            RotateR(parent->_right);
            RotateL(parent);

            if (bf == 1) {
                subR->_bf = 0;
                subRL->_bf = 0;
                parent->_bf = -1;
            } else if (bf == -1) {
                subR->_bf = 1;
                subRL->_bf = 0;
                parent->_bf = 0;
            } else if (bf == 0) {
                subR->_bf = 0;
                subRL->_bf = 0;
                parent->_bf = 0;
            } else {
                assert(false);
            }
        }

        void Inorder() {
            _Inorder(_root);
        }

        void _Inorder(Node *root) {
            if (root == nullptr)
                return;

            _Inorder(root->_left);
            cout << root->_kv.first << ":" << root->_kv.second << endl;
            _Inorder(root->_right);
        }

        int Height(Node *root) {
            if (root == nullptr)
                return 0;

            int lh = Height(root->_left);
            int rh = Height(root->_right);

            return lh > rh ? lh + 1 : rh + 1;
        }

        bool IsBalance() {
            return IsBalance(_root);
        }

        bool IsBalance(Node *root) {
            if (root == nullptr) {
                return true;
            }

            int leftHeight = Height(root->_left);
            int rightHeight = Height(root->_right);

            if (rightHeight - leftHeight != root->_bf) {
                cout << root->_kv.first << "平衡因子异常" << endl;
                return false;
            }

            return abs(rightHeight - leftHeight) < 2
                   && IsBalance(root->_left)
                   && IsBalance(root->_right);
        }

    private:
        Node *_root = nullptr;
    };
}
#endif //LESSON2_AVLTREE_H
/*namespace bit
{
    template<class K,class V>
    struct AVLTreeNode
    {
        AVLTreeNode<K, V>* _left;
        AVLTreeNode<K, V>* _right;
        AVLTreeNode<K, V>* _parent;

        // 右子树的高度-左子树的高度
        int _bf; // 平衡因子  balance factor

        pair<K, V> _kv;

        AVLTreeNode(const pair<K, V>& kv)
                :_left(nullptr)
                , _right(nullptr)
                , _parent(nullptr)
                , _bf(0)
                , _kv(kv)
        {}
    };

    template<class K, class V>
    class AVLTree
    {
        typedef AVLTreeNode<K, V> Node;
    public:
        AVLTree()
                :_root(nullptr)
        {}

        // 拷贝构造和赋值需要实现深拷贝

        void _Destory(Node* root)
        {
            if (root == nullptr)
            {
                return;
            }

            _Destory(root->_left);
            _Destory(root->_right);
            delete root;
        }

        ~AVLTree()
        {
            _Destory(_root);
            _root = nullptr;
        }

        V& operator[](const K& key)
        {
            pair<Node*, bool> ret = Insert(make_pair(key, V()));
            return ret.first->_kv.second;
        }

        pair<Node*, bool> Insert(const pair<K, V>& kv)
        {
            if (_root == nullptr)
            {
                _root = new Node(kv);
                return make_pair(_root, true);
            }

            // 找到存储位置，把数据插入进去
            Node* parent = _root, *cur = _root;
            while (cur)
            {
                if (cur->_kv.first > kv.first)
                {
                    parent = cur;
                    cur = cur->_left;
                }
                else if (cur->_kv.first < kv.first)
                {
                    parent = cur;
                    cur = cur->_right;
                }
                else
                {
                    return make_pair(cur, false);
                }
            }

            cur = new Node(kv);
            Node* newnode = cur;
            if (parent->_kv.first < kv.first)
            {
                parent->_right = cur;
                cur->_parent = parent;
            }
            else
            {
                parent->_left = cur;
                cur->_parent = parent;
            }

            // 控制平衡
            // 1、更新平衡因子
            // 2、如果出现不平衡，则需要旋转
            //while (parent)
            while (cur != _root)
            {
                if (parent->_left == cur)
                {
                    parent->_bf--;
                }
                else
                {
                    parent->_bf++;
                }

                if (parent->_bf == 0)
                {
                    break;
                }
                else if (parent->_bf == 1 || parent->_bf == -1)
                {
                    // parent所在的子树高度变了，会影响parent->parent
                    // 继续往上更新
                    cur = parent;
                    parent = parent->_parent;
                }
                else if (parent->_bf == 2 || parent->_bf == -2)
                {
                    //parent所在子树已经不平衡，需要旋转处理一下
                    if (parent->_bf  == -2)
                    {
                        if (cur->_bf == -1)
                        {
                            // 右单旋
                            RotateR(parent);
                        }
                        else // cur->_bf == 1
                        {
                            RotateLR(parent);
                        }
                    }
                    else // parent->_bf  == 2
                    {
                        if (cur->_bf == 1)
                        {
                            // 左单旋
                            RotateL(parent);
                        }
                        else // cur->_bf == -1
                        {
                            RotateRL(parent);
                        }
                    }

                    break;
                }
                else
                {
                    // 插入节点之前，树已经不平衡了，或者bf出错。需要检查其他逻辑
                    assert(false);
                }
            }

            return make_pair(newnode, true);
        }

        void RotateLR(Node* parent)
        {
            Node* subL = parent->_left;
            Node* subLR = subL->_right;
            int bf = subLR->_bf;

            RotateL(parent->_left);
            RotateR(parent);

            // ...平衡因子调节还需要具体分析
            if (bf == -1)
            {
                subL->_bf = 0;
                parent->_bf = 1;
                subLR->_bf = 0;
            }
            else if (bf == 1)
            {
                parent->_bf = 0;
                subL->_bf = -1;
                subLR->_bf = 0;
            }
            else if (bf == 0)
            {
                parent->_bf = 0;
                subL->_bf = 0;
                subLR->_bf = 0;
            }
            else
            {
                assert(false);
            }
        }

        void RotateRL(Node* parent)
        {
            Node* subR = parent->_right;
            Node* subRL = subR->_left;
            int bf = subRL->_bf;

            RotateR(parent->_right);
            RotateL(parent);

            // 平衡因子更新
            if (bf == 1)
            {
                subR->_bf = 0;
                parent->_bf = -1;
                subRL->_bf = 0;
            }
            else if (bf == -1)
            {
                parent->_bf = 0;
                subR->_bf = 1;
                subRL->_bf = 0;
            }
            else if (bf == 0)
            {
                parent->_bf = 0;
                subR->_bf = 0;
                subRL->_bf = 0;
            }
            else
            {
                assert(false);
            }
        }

        void RotateL(Node* parent)
        {
            Node* subR = parent->_right;
            Node* subRL = subR->_left;

            parent->_right = subRL;
            if (subRL)
            {
                subRL->_parent = parent;
            }

            subR->_left = parent;
            Node* parentParent = parent->_parent;
            parent->_parent = subR;

            if (parent == _root)
            {
                _root = subR;
                _root->_parent = nullptr;
            }
            else
            {
                if (parentParent->_left == parent)
                {
                    parentParent->_left = subR;
                }
                else
                {
                    parentParent->_right = subR;
                }
                subR->_parent = parentParent;
            }

            parent->_bf = subR->_bf = 0;
        }

        void RotateR(Node* parent)
        {
            Node* subL = parent->_left;
            Node* subLR = subL->_right;

            parent->_left = subLR;
            if (subLR)
                subLR->_parent = parent;

            subL->_right = parent;
            Node* parentParent = parent->_parent;
            parent->_parent = subL;

            if (parent == _root)
            {
                _root = subL;
                _root->_parent = nullptr;
            }
            else
            {
                if (parentParent->_left == parent)
                    parentParent->_left = subL;
                else
                    parentParent->_right = subL;

                subL->_parent = parentParent;
            }

            subL->_bf = parent->_bf = 0;
        }

        Node* Find(const K& key)
        {
            Node* cur = _root;
            while (cur)
            {
                if (cur->_kv.first <  key)
                {
                    cur = cur->_right;
                }
                else if (cur->_kv.first > key)
                {
                    cur = cur->_left;
                }
                else
                {
                    return cur;
                }
            }

            return nullptr;
        }

        // 1、工作中会用的（AVL树不会自己写，这里通过插入深入理解一下他的性质就够了）
        // 2、校招会考的（基本不会问删除的细节）
        // 有兴趣的可以下去实现一下。
        bool Erase(const K& key)
        {
            return false;
        }

        void _InOrder(Node* root)
        {
            if (root == nullptr)
            {
                return;
            }

            _InOrder(root->_left);
            cout << root->_kv.first << ":"<<root->_kv.second<<endl;
            _InOrder(root->_right);
        }

        void InOrder()
        {
            _InOrder(_root);
        }

        int _Height(Node* root)
        {
            if (root == nullptr)
            {
                return 0;
            }

            int leftHeight = _Height(root->_left);
            int rightHeight = _Height(root->_right);

            return rightHeight > leftHeight ? rightHeight + 1 : leftHeight + 1;
        }

        bool _IsBalance(Node* root)
        {
            if (root == nullptr)
            {
                return true;
            }

            int leftHeight = _Height(root->_left);
            int rightHeight = _Height(root->_right);

            // 检查一下平衡因子是否正确
            if (rightHeight - leftHeight != root->_bf)
            {
                cout << "平衡因子异常:"<<root->_kv.first<<endl;
                return false;
            }

            return abs(rightHeight - leftHeight) < 2
                   && _IsBalance(root->_left)
                   && _IsBalance(root->_right);
        }
        bool IsAVLTree()
        {
            return _IsBalance(_root);
        }
    private:
        Node* _root;
    };
}*/