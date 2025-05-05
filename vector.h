#include <iostream>
#include <memory>
#include <new>
#include <cstddef>
#include <initializer_list>
#include <algorithm>

template <typename T, typename Allocator = std::allocator<T>>
class Vector
{
public:
    Vector(size_t count, const T& item);
    Vector(const std::initializer_list<T> items);
    void reserve(size_t capacity);
    void push_back(const T& item);
    template <typename... Args>
    void emplace_back(Args&&... args);
    void clear();
    size_t size() const
    {
        return _size;
    }
    size_t capacity() const
    {
        return _capacity;
    }
    T* data() const
    {
        return _data;
    }
    Allocator get_allocator() const
    {
        return { };
    }

private:

    void _try_expand_capacity()
    {
        if (_size == _capacity)
            reserve(_capacity == 0 ? 1 : _capacity * 2);
    }

    size_t _size{ };
    size_t _capacity{ };
    T* _data{ nullptr };
};

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(size_t count, const T& item)
{
    reserve(count);

    for (size_t i{}; i < count; ++i)
    {
        push_back(item);
    }
}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(const std::initializer_list<T> items)
{
    reserve(items.size());

    for (auto&& item : items)
    {
        push_back(item);
    }
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::reserve(size_t newCapacity)
{
    if (newCapacity <= _capacity)
        return;

    // 1. Allocate memory.
    Allocator allocator;
    T* allocatedMemory = allocator.allocate(newCapacity);

    // 2. Copy over existing objects.
    if (_data != nullptr)
    {
        // Can move items, but if moved, and an error is thrown halfway through, we lose the strong
        // exception guarantee I'm aiming to maintain. Let's not worry about that for now.
        for (size_t i{}; i < _size; ++i)
            new (allocatedMemory + i) T{ _data[i] };
     
        // 3. Destruct existing objects.
        for (size_t i{}; i < _size; ++i)
            _data[i].~T();

        // 4. Deallocate existing objects.
        allocator.deallocate(_data, _capacity);
    }

    _capacity = newCapacity;
    _data = allocatedMemory;
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::push_back(const T& item)
{
    _try_expand_capacity();

    new (_data + _size++) T{ item };
}

template <typename T, typename Allocator>
template <typename... Args>
void Vector<T, Allocator>::emplace_back(Args&&... args)
{
    _try_expand_capacity();

    new (_data + _size++) T(std::forward<Args>(args)...);
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::clear()
{
    if (_data == nullptr)
        return;

    for (size_t i{}; i < _size; ++i)
        _data[i].~T();

    _size = 0;
}




int main()
{
    Vector<int> v{1, 2, 3};
    std::cout << v.size() << std::endl;
    return 0;
}