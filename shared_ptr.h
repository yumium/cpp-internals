#include <utility>
#include <iostream>
#include <mutex>
#include <cstddef>

struct ControlBlock
{
    size_t _count{ };
    mutable std::mutex _mutex; // why mutable?
};

// SharedPointer hold internally the pointer and points to a `count` object shared by other
// SharedPointer objects holding the same underlying pointer
template <typename T>
class SharedPointer
{
public:
    // Constructors
    SharedPointer() : SharedPointer(nullptr) { }
    SharedPointer(std::nullptr_t) { }
    SharedPointer(T* pointer)
    {
        _initialise(pointer);
    }

    // Copy constructor
    SharedPointer(const SharedPointer& other) noexcept
    {
        _copy_from(other);
    }

    // Copy assignment
    SharedPointer& operator=(const SharedPointer& other) noexcept
    {
        if (this == &other)
            return *this;

        _try_release();
        _copy_from(other);

        return *this;
    }

    // Move constructor
    SharedPointer(SharedPointer&& other) noexcept
    {
        _steal_from(other);
    }

    // Move assignment
    SharedPointer& operator=(SharedPointer&& other) noexcept
    {
        if (this == &other)
            return *this;

        _try_release();
        _steal_from(other);

        return *this;
    }

    // Destructor
    ~SharedPointer()
    {
        _try_release();
    }

    void reset(T* pointer)
    {
        reset();

        if (pointer == nullptr)
            return;

        _initialise(pointer);
    }

    void reset()
    {
        _try_release();
    }

    size_t get_count() const
    {
        std::scoped_lock lock{ _block->_mutex };
        return _block->_count;
    }

    T* get() const { return _pointer; }
    T* operator->() const { return _pointer; }
    T& operator*() const { return *_pointer; }
    operator bool() const noexcept { return _pointer != nullptr; }

private:

    void _copy_from(const SharedPointer& other) noexcept
    {
        _pointer = other._pointer;
        _block = other._block;

        if (_block == nullptr)
            return;

        std::scoped_lock lock{ _block->_mutex };
        _block->_count++;
    }

    void _steal_from(SharedPointer&& other) noexcept
    {
        _pointer = std::exchange(other._pointer, nullptr);
        _block = std::exchange(other._block, nullptr);
    }

    void _initialise(T* pointer)
    {
        _pointer = pointer;
        _block = new ControlBlock{1};
    }

    void _try_release()
    {
        if (_block == nullptr)
            return;
            
        {                      
            // TODO: what is std::scoped_lock?
            std::scoped_lock lock{ _block->_mutex };

            if (--_block->_count != 0)
                return;
        }

        delete _pointer;
        _pointer = nullptr;

        delete _block;
        _block = nullptr;
    }

    T* _pointer{ nullptr };
    ControlBlock* _block{ nullptr };
};

int main()
{   
    return 0;
}