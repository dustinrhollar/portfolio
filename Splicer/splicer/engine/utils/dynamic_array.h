#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

template<typename T>
class DynamicArray
{
    public:
    
    DynamicArray(unsigned int cap = 0);
    DynamicArray(unsigned int size,
                 T* data,
                 unsigned int cap = 10);
    
    ~DynamicArray();
    
    DynamicArray(const DynamicArray& cpy);
    DynamicArray(DynamicArray&& cpy);
    
    DynamicArray& operator=(const DynamicArray& other);
    DynamicArray& operator=(DynamicArray&& other);
    
    T& operator[](u64 idx) {return ptr[idx];}
    
    unsigned int Capacity() {return cap;}
    unsigned int Size()     {return size;}
    
    void Reset();
    
    // Add an element at the specified index
    void Add(unsigned int idx, T& element);
    // Push an element to the front of the list
    void Push(T& element);
    // Push an element to the back of the list
    void PushBack(T& element);
    
    // Removes an element at the specified index
    // @param idx index of the element to remove
    // @param element the element that is removed.
    //        this is set by function itself
    void Remove(unsigned int idx, T& element);
    // Removes an element from the back of the list
    // @param element that is removed by the pop operation
    void Pop(T& element);
    
    // Get an element given an index
    T& Get(unsigned int idx);
    // Get the underlying array
    T* GetArray();
    
    void Resize(unsigned new_capacity);
    
    private:
    
    T* ptr;
    unsigned int size;
    unsigned int cap;
};

template<typename T>
DynamicArray<T>::DynamicArray(unsigned int _cap)
: cap(_cap)
, size(0)
{
    if (cap != 0)
        ptr = palloc<T>(cap);
    else
        ptr = nullptr;
}

template<typename T>
DynamicArray<T>::DynamicArray(unsigned int _size,
                              T* data,
                              unsigned int _cap)
: size(_size)
{
    if (_cap < size)
    {
        cap = size;
    }
    else
    {
        cap = _cap;
    }
    
    ptr = palloc<T>(cap);
    for (int i = 0; i < _size; ++i) ptr[i] = data[i];
    //memcpy(ptr, data, sizeof(T)*size);
}

template<typename T>
DynamicArray<T>::~DynamicArray()
{
    Reset();
}

template<typename T>
void DynamicArray<T>::Reset()
{
    size = 0;
    cap = 0;
    
    if (ptr)
        pfree(ptr);
    ptr = nullptr;
}

template<typename T>
DynamicArray<T>::DynamicArray(const DynamicArray& cpy)
: size(cpy.size)
, cap(cpy.cap)
{
    ptr = palloc<T>(cap);
    for (int i = 0; i < size; ++i) ptr[i] = other.ptr[i];
    //memcpy(ptr, cpy.ptr, sizeof(T)*size);
}

template<typename T>
DynamicArray<T>::DynamicArray(DynamicArray&& cpy)
: size(cpy.size)
, cap(cpy.cap)
{
    ptr = other.ptr;
    
    other.size = 0;
    other.cap = 0;
    other.ptr = nullptr;
    
    //ptr = jengine::mm::jalloc<T>(cap);
    //for (int i = 0; i < _size; ++i) ptr[i] = data[i];
    //memcpy(ptr, cpy.ptr, sizeof(T)*size);
}

template<typename T>
DynamicArray<T>& DynamicArray<T>::operator=(const DynamicArray<T>& other)
{
    size = other.size;
    cap  = other.cap;
    
    ptr = palloc<T>(cap);
    for (unsigned i = 0; i < size; ++i) ptr[i] = other.ptr[i];
    //memcpy(ptr, other.ptr, sizeof(T)*size);
    
    return *this;
}

template<typename T>
DynamicArray<T>& DynamicArray<T>::operator=(DynamicArray&& other)
{
    size = other.size;
    cap = other.cap;
    
    ptr = other.ptr;
    
    other.size = 0;
    other.cap = 0;
    other.ptr = nullptr;
    
    return *this;
}

// Add an element at the specified index
template<typename T>
void DynamicArray<T>::Add(unsigned int idx, T& element)
{
    if (idx < 0 || idx >= size)
    { // TODO(Dustin): silent fail, change this
        return;
    }
    
    if (size + 1 >= cap)
    {
        Resize(cap * 2);
    }
    
    for (unsigned int i = size; i > idx; --i)
    {
        ptr[i] = ptr[i-1];
    }
    
    ptr[idx] = element;
    size++;
}

// Push an element to the front of the list
template<typename T>
void DynamicArray<T>::Push(T& element)
{
    if (size + 1 >= cap)
    {
        // amortized O(1) add
        Resize(cap * 2);
    }
    
    for (unsigned int i = size; i > 0; --i)
    {
        ptr[i] = ptr[i-1];
    }
    
    ptr[0] = element;
    ++size;
}

// Push an element to the back of the list
template<typename T>
void DynamicArray<T>::PushBack(T& element)
{
    if (size + 1 >= cap)
    {
        // amortized add
        Resize(cap * 2);
    }
    
    ptr[size++] = element;
}

// Removes an element at the specified index
template<typename T>
void DynamicArray<T>::Remove(unsigned int idx, T& element)
{
    if (idx < 0 || idx >= size)
    { // TODO(Dustin): Silent failt. Fix this
        return;
    }
    
    T removed_element = ptr[idx];
    for (unsigned int i = idx + 1; i < size; ++i)
    {
        ptr[i-1] = ptr[i];
    }
    
    --size;
    element = removed_element;
}

// Removes an element from the back of the list
template<typename T>
void DynamicArray<T>::Pop(T& element)
{
    T removed_element = ptr[size-1];
    size--;
    element = removed_element;
}

// Get an element given an index
template<typename T>
T& DynamicArray<T>::Get(unsigned int idx)
{
    return ptr[idx];
}

// Get the underlying array
template<typename T>
T* DynamicArray<T>::GetArray()
{
    return &ptr[0];
}

template<typename T>
void DynamicArray<T>::Resize(unsigned new_capacity)
{
    // if resize is 0, free the memory of ptr
    if (new_capacity == 0) {
        if (ptr) {
            pfree(ptr);
            ptr = nullptr;
        }
        size = 0;
        cap  = 0;
        return;
    }
    
    // no need to resize, if the request already fits within bounds...
    if (new_capacity < cap)
        return;
    
    T* new_ptr = palloc<T>(new_capacity);
    
    if (ptr) {
        for (unsigned i = 0; i < size; ++i) new_ptr[i] = ptr[i];
        //memcpy(new_ptr, ptr, sizeof(T)*size);
        pfree(ptr);
    }
    
    cap = new_capacity;
    ptr = new_ptr;
}

#endif //DYNAMIC_ARRAY_H
