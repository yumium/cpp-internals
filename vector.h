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
        return size_;
    }
    size_t capacity() const
    {
        return capacity_;
    }
    T* data() const
    {
        return data_;
    }
    Allocator get_allocator() const
    {
        return { };
    }

private:

    void TryExpandCapacity()
    {
        if (size_ == capacity_)
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }

    size_t size_{ };
    size_t capacity_{ };
    T* data_{ nullptr };
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
    if (newCapacity <= capacity_)
        return;

    // 1. Allocate memory.
    Allocator allocator;
    T* allocatedMemory = allocator.allocate(newCapacity);

    // 2. Copy over existing objects.
    if (data_ != nullptr)
    {
        // Can move items, but if moved, and an error is thrown halfway through, we lose the strong
        // exception guarantee I'm aiming to maintain. Let's not worry about that for now.
        for (size_t i{}; i < size_; ++i)
            new (allocatedMemory + i) T{ data_[i] };
     
        // 3. Destruct existing objects.
        for (size_t i{}; i < size_; ++i)
            data_[i].~T();

        // 4. Deallocate existing objects.
        allocator.deallocate(data_, capacity_);
    }

    capacity_ = newCapacity;
    data_ = allocatedMemory;
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::push_back(const T& item)
{
    TryExpandCapacity();

    new (data_ + size_++) T{ item };
}

template <typename T, typename Allocator>
template <typename... Args>
void Vector<T, Allocator>::emplace_back(Args&&... args)
{
    TryExpandCapacity();

    new (data_ + size_++) T(std::forward<Args>(args)...);
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::clear()
{
    if (data_ == nullptr)
        return;

    for (size_t i{}; i < size_; ++i)
        data_[i].~T();

    size_ = 0;
}




int main()
{
    Vector<int> v{1, 2, 3};
    std::cout << v.size() << std::endl;
    return 0;
}