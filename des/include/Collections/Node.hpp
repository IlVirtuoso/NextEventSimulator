
#include <algorithm>
#include <concepts>
#include <functional>

template <typename T>
concept Comparable = requires(T a) {
    {
        a == a
    };
};

template <class T>
    requires Comparable<T>
class DoubleLinkedList;

template <typename T> class Node
{
    friend class DoubleLinkedList<T>;

  private:
    T _val;

  protected:
    Node *_previous;
    Node *_next;

  public:
    Node(T value);

    Node(Node &&);

    Node(Node &);

    ~Node();

    void operator>>(Node &next);

    void operator<<(Node &previous);

    T &Value();

    const T &const_value() const
    {
        return _val;
    }

    T &operator*()
    {
        return Value();
    }

    Node<T> *Next() const
    {
        return _next;
    }

    Node<T> *Previous() const
    {
        return _previous;
    }
};

template <typename T> inline Node<T>::Node(T value) : _val{value}, _next{nullptr}, _previous{nullptr}
{
}

template <typename T> inline Node<T>::Node(Node &&n) : _val{n._val}
{
    n.~Node();
}

template <typename T> inline Node<T>::Node(Node &n) : _val{n._val}
{
}

template <typename T> inline Node<T>::~Node()
{
    if (_next != nullptr)
        _next->_previous = nullptr;
    if (_previous != nullptr)
        _previous->_next = nullptr;
    _next = nullptr;
    _previous = nullptr;
}

template <typename T> inline void Node<T>::operator>>(Node &next)
{
    _next = &next;
    next._previous = this;
}

template <typename T> inline void Node<T>::operator<<(Node &previous)
{
    _previous = &previous;
    previous._next = this;
}

template <typename T> inline T &Node<T>::Value()
{
    return _val;
}

template <typename T> class NodeIterator
{
  private:
    Node<T> *_currentNode;

  public:
    NodeIterator &Next(std::function<bool(const T &)> predicate)
    {
        _currentNode = _currentNode->Next();
        while (!predicate(_currentNode->Value()) && _currentNode != nullptr)
        {
            _currentNode = _currentNode->Next();
        }
        return *this;
    }

    NodeIterator &Last(std::function<bool(const T &)> predicate)
    {
        Node<T> *last = nullptr;

        while (_currentNode != nullptr)
        {
            if (predicate(_currentNode->Value()))
                last = _currentNode;
            _currentNode = _currentNode->Next();
        }
        _currentNode = last;
        return *this;
    }

    NodeIterator &operator++(int)
    {
        _currentNode = _currentNode->Next();
        return *this;
    }

    NodeIterator &operator--(int)
    {
        _currentNode = _currentNode->Previous();
        return *this;
    }

    NodeIterator &operator+(int offset)
    {
        for (int i = 0; i < offset; i++)
            _currentNode = _currentNode->Next();
        return *this;
    }

    NodeIterator &operator-(int offset)
    {
        for (int i = 0; i < offset; i++)
            _currentNode = _currentNode->Previous();
        return *this;
    }

    T operator*()
    {
        return _currentNode->Value();
    }

    const T &const_value() const
    {
        return _currentNode->Value();
    }

    const Node<T> *const_node() const
    {
        return _currentNode;
    }

    Node<T> *operator()()
    {
        return _currentNode;
    }

    NodeIterator(Node<T> *reference) : _currentNode{reference}
    {
    }

    inline bool operator==(const NodeIterator &itr)
    {
        return _currentNode == itr.const_node();
    }

    inline bool operator!=(const NodeIterator &itr)
    {
        return _currentNode != itr.const_node();
    }
};
