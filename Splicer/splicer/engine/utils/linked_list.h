#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/*

A linked list made up of chained "Nodes".
the LL contains a reference to the front and
back of the list. In order to improve access
times, an internal iterator is used. the
iterator keeps track of the last accessed
element in the list. that way, sequential access
into the list has a O(1) time complexity..

Nodes are heap allocated on creation and freed
when removing.

*/

template<typename T>
class LinkedList
{
    public:
    
    LinkedList();
    ~LinkedList();
    
    // Copy Constructors and equals operator.
    // The internal interal iterator is reset
    // to the front of the list.
    LinkedList(LinkedList<T>& cpy);
    LinkedList(LinkedList<T>&& cpy);
    
    LinkedList<T>& operator=(LinkedList<T>& cpy);
    LinkedList<T>& operator=(LinkedList<T>&& cpy);
    
    unsigned int Size() { return size; }
    
    // Frees all elements in the list
    void Reset();
    
    // Addds an element to the specified index
    void Add(unsigned int idx, T& element);
    // pushes an element to the back of the list
    void PushBack(T& element);
    // pushes an element to the front of the list
    void Push(T& element);
    
    // Removes an element at the specified index.
    // @param element: the removed element is stored here
    void Remove(unsigned int idx, T& element);
    // Removes the specified element.
    // @param element to remove
    void Remove(T& element);
    // Removes an element from the back of the list
    void Pop(T& element);
    
    // Gets an element in the list. Sets the iterator
    // to this index.
    T& Get(unsigned int idx);
    
    private:
    
    struct Node
    {
        Node *prev;
        Node *next;
        
        T data;
    };
    
    struct Iterator
    {
        int current_idx;
        Node *current_node;
    };
    
    Node *front;
    Node *back;
    
    Iterator iter;
    
    unsigned int size;
};

template<typename T>
LinkedList<T>::LinkedList()
: size(0)
, front(nullptr)
, back(nullptr)
, iter({-1, nullptr})
{
}

template<typename T>
LinkedList<T>::~LinkedList()
{
    Node *current = front;
    for (unsigned int i = 0; i < size; ++i)
    {
        Node *tmp = current->next;
        jengine::mm::jfree(current);
        
        current = tmp;
    }
    
    size = 0;
}

template<typename T>
LinkedList<T>::LinkedList(LinkedList<T>& cpy)
: size(cpy)
, front(cpy.front)
, back(cpy.back)
, iter({0, front})
{
}

template<typename T>
LinkedList<T>::LinkedList(LinkedList<T>&& cpy)
: size(cpy)
, front(cpy.front)
, back(cpy.back)
, iter({0, front})
{
}

template<typename T>
LinkedList<T>& LinkedList<T>::operator=(LinkedList<T>& cpy)
{
    size = (cpy);
    front = (cpy.front);
    back = (cpy.back);
    iter = ({0, front});
}

template<typename T>
LinkedList<T>& LinkedList<T>::operator=(LinkedList<T>&& cpy)
{
    size = (cpy);
    front = (cpy.front);
    back = (cpy.back);
    iter = ({0, front});
}

template<typename T>
void LinkedList<T>::Reset()
{
    Node *current = front;
    for (unsigned int i = 0; i < size; ++i)
    {
        Node *tmp = current->next;
        jengine::mm::jfree(current);
        
        current = tmp;
    }
    
    size = 0;
    front = nullptr;
    back = nullptr;
    
    iter = {0};
}

// Addds an element to the specified index
template<typename T>
void LinkedList<T>::Add(unsigned int idx, T& element)
{
    if (idx < 0 || idx > size)
    { // TODO(Dustin): Silent fail.
        return;
    }
    
    Node *added_node = jengine::mm::jalloc<Node>();
    added_node->data = element;
    added_node->next = nullptr;
    added_node->prev = nullptr;
    
    if (idx == 0)
    { // adding to the first node
        if (size == 0)
        { // this is the first node in the list
            front = added_node;
            back  = added_node;
        }
        else
        {
            added_node->next = front;
            front->prev = added_node;
            front = added_node;
        }
    }
    else if (idx == size)
    { // adding to the last node
        added_node->prev = back;
        back->next = added_node;
        back = added_node;
    }
    else
    { // removing from the middle of the list
        Node *tmp = front;
        for (unsigned int i = 0; i < idx - 1; ++i)
            tmp = tmp->next;
        
        added_node->prev = tmp;
        added_node->next = tmp->next;
        tmp->next->prev = added_node;
        tmp->next = added_node;
    }
    
    // update the size
    ++size;
    
    // Since the index of the iterator could have changed after adding an element,
    // reset it
    iter.current_idx = 0;
    iter.current_node = front;
}

// pushes an element to the back of the list
template<typename T>
void LinkedList<T>::PushBack(T& element)
{
    Add(size, element);
}

// pushes an element to the front of the list
template<typename T>
void LinkedList<T>::Push(T& element)
{
    Add(0, element);
}

template<typename T>
void LinkedList<T>::Remove(T& element)
{
    Node *removed_node = nullptr;
    if (element == front->data)
    {
        removed_node = front;
        front = front->next;
        if (front)
        {
            front->prev = nullptr;
        }
        
        size--;
    }
    else if (element == back->data)
    {
        removed_node = back;
        back = back->prev;
        if (back)
        {
            back->next = nullptr;
        }
        
        size--;
    }
    else
    {
        Node *iter = front;
        while (iter->next)
        {
            if (iter->next->data == element)
            {
                removed_node = iter->next;
                
                iter->next = iter->next->next;
                if (iter->next)
                {
                    iter->next->prev = iter;
                }
                
                size--;
                break;
            }
            
            iter = iter->next;
        }
    }
    
    jengine::mm::jfree(removed_node);
    
    // Since the index of the iterator could have changed after adding an element,
    // reset it
    iter.current_idx = 0;
    iter.current_node = front;
}

// Removes an element at the specified index.
// @param element: the removed element is stored here
template<typename T>
void LinkedList<T>::Remove(unsigned int idx, T& removed_element)
{
    if (idx < 0 || idx > size || front == nullptr)
    { // TODO(Dustin): Silent fail.
        return;
    }
    
    Node *removed_node = nullptr;
    
    if (idx == 0)
    { // removing the first node
        removed_node = front;
        
        if (size == 1)
        { // this was the only node in the list
            back = nullptr;
            front = nullptr;
        }
        else
        {
            front = front->next;
            if (front)
            {
                front->prev = nullptr;
            }
        }
    }
    else if (idx == size - 1)
    { // removing the last node
        removed_node = back;
        
        back = back->prev;
        if (back->prev)
        {
            back->next = nullptr;
        }
    }
    else
    { // removing from the middle of the list
        Node *tmp = front;
        for (unsigned int i = 0; i < idx - 1; ++i)
            tmp = tmp->next;
        
        removed_node = tmp->next;
        
        tmp->next = tmp->next->next;
        if (tmp->next)
        {
            tmp->next->prev = tmp;
        }
    }
    
    // free up the removed node
    removed_element = removed_node->data;
    jengine::mm::jfree(removed_node);
    --size;
    
    // Since the index of the iterator could have changed after adding an element,
    // reset it
    iter.current_idx = 0;
    iter.current_node = front;
}

// Removes an element from the back of the list
template<typename T>
void LinkedList<T>::Pop(T& element)
{
    Remove(size-1, element);
}

template<typename T>
T& LinkedList<T>::Get(unsigned int idx)
{
    /*
Get attempts to find the quickest path to the index.
These are the possible scenarios
1. iter.idx == idx
2. (idx < iter.idx) && close to iter.idx
3. (idx < iter.idx) && close to front
4. (idx > iter.idx) && close to iter.idx
5. (idx > iter.idx) && close to back
*/
    
    // retrieved node
    Node *tmp = nullptr;
    
    // Find the node to retrieve
    if (iter.current_idx == idx)
    { // Case 1: Iterator is already at the idx
        tmp = iter.current_node;
    }
    else if (idx < iter.current_idx)
    { // idx is between iterator and front node
        if (iter.current_idx - idx <= idx)
        { // Case 2: idx is close to iterator
            tmp = iter.current_node;
            for (int i = iter.current_idx; i > idx; --i)
            {
                tmp = tmp->prev;
            }
        }
        else
        { // Case 3: idx is close the front
            tmp = front;
            for (int i = 0; i < idx; ++i)
            {
                tmp = tmp->next;
            }
        }
    }
    else
    { // idx is between iterator and back node
        if (idx - iter.current_idx <= (size-1) - idx)
        { // Case 4: idx is close to the iterator
            tmp = iter.current_node;
            for (int i = iter.current_idx; i < idx; ++i)
            {
                tmp = tmp->next;
            }
        }
        else
        { // Case 5: idx is close to the back
            tmp = back;
            for (int i = size-1; i > idx; --i)
            {
                tmp = tmp->prev;
            }
        }
    }
    
    // update the iterator
    if (idx+1 != size)
    {
        iter.current_idx = idx+1;
        iter.current_node = tmp->next;
    }
    else
    { // we've hit the end of the list, so wrap back to the front
        iter.current_idx = 0;
        iter.current_node = front;
    }
    
    return tmp->data;
}

#endif //LINKED_LIST_H
