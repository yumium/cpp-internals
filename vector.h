#include <iostream>
#include <memory>
#include <new>
#include <cstddef>
#include <initializer_list>
#include <algorithm>

/*
Constructors
- Vector(size_t, const T&)
- Vector(const std::initializer_list<T>)

reserve(size_t)
push_back(const T&)

template<typename... Args>
emplace_back(Args&&... args)
clear()
size()
capacity()
data()
get_allocator()
*/

template <typename T, typename Allocator = std::allocator<T>> // TODO: look more into std::allocator
class Vector
{
public:
    Vector() = default;
    Vector(size_t count, const T& item);
    Vector(const std::initializer_list<T> items);
    Vector(const Vector& other);
    Vector& operator=(const Vector& other);
    Vector(Vector&& other) noexcept;
    Vector& operator=(Vector&& other) noexcept;
    ~Vector();
    void reserve(size_t capacity);
    void push_back(const T& item);

    template <typename... Args>
    void emplace_back(Args&&... args);
    void clear();
    size_t size() const { return _size; }
    size_t capacity() const { return _capacity; }
    T* data() const { return _data; }
    Allocator get_allocator() const { return { }; }

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

    for (size_t i = 0; i < count; ++i)
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

// Copy Constructor
template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(const Vector& other)
    : _size(other._size), _capacity(other._capacity)
{
    Allocator allocator;
    _data = allocator.allocate(_capacity);
    for (size_t i = 0; i < _size; ++i) {
        new (_data + i) T(other._data[i]);
    }
}

// Copy Assignment Operator (simplified version)
template <typename T, typename Allocator>
Vector<T, Allocator>& Vector<T, Allocator>::operator=(const Vector& other)
{
    if (this == &other) return *this;

    // A more robust implementation would use the copy-and-swap idiom
    clear();
    Allocator allocator;
    allocator.deallocate(_data, _capacity);

    _size = other._size;
    _capacity = other._capacity;
    _data = allocator.allocate(_capacity);
    for (size_t i = 0; i < _size; ++i) {
        new (_data + i) T(other._data[i]);
    }
    return *this;
}

// Move Constructor
template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(Vector&& other) noexcept
    : _size(other._size), _capacity(other._capacity), _data(other._data)
{
    // Leave the moved-from object in a valid, empty state
    other._data = nullptr;
    other._size = 0;
    other._capacity = 0;
}

// Move Assignment Operator
template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(Vector&& other) noexcept // signature might be wrong
{
    if (this == &other) return *this;

    clear();
    Allocator allocator;
    allocator.deallocate(_data, _capacity);

    _data = other._data;
    _size = other._size;
    _capacity = other._capacity;

    other._data = nullptr;
    other._size = 0;
    other._capacity = 0;

    return *this;
}

template <typename T, typename Allocator>
Vector<T, Allocator>::~Vector()
{
    clear(); // Calls destructors on all elements
    if (_data) {
        Allocator allocator;
        allocator.deallocate(_data, _capacity);
    }
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::reserve(size_t new_capacity)
{
    if (new_capacity <= _capacity)
        return;

    // 1. Allocate memory.
    Allocator allocator;
    T* allocated_memory = allocator.allocate(new_capacity);

    // 2. Copy over existing objects.
    if (_data != nullptr)
    {
        // Can move items, but if moved, and an error is thrown halfway through, we lose the strong
        // exception guarantee I'm aiming to maintain. Let's not worry about that for now.
        for (size_t i = 0; i < _size; ++i)
            new (allocated_memory + i) T{ _data[i] };
     
        // 3. Destruct existing objects.
        for (size_t i{}; i < _size; ++i)
            _data[i].~T();

        // 4. Deallocate existing objects.
        allocator.deallocate(_data, _capacity);
    }

    _capacity = new_capacity;
    _data = allocated_memory;
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