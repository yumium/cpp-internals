#include <utility>
#include <iostream>
#include <mutex>
#include <cstddef>

/*
Constructor
- SharedPointer(std::nullptr_t)
- SharedPointer(T*)

reset(T*)
reset()
get_count()
get()
operator->()
operator*()
*/

struct ControlBlock
{
    size_t _count{ };
    mutable std::mutex _mutex; // better with std::atomic<size_t>
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
        SharedPointer temp(pointer);

        std::swap(_pointer, temp._pointer);
        std::swap(_block, temp._block);

        // old data freed when `temp` goes out of scope
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
        if (_pointer != nullptr) {
            _block = new ControlBlock{1};
        } // don't allocate CB for nullptr
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

/*
To add make_shared (to have single allocation) we need to modify control block slightly

// A base class for the control block to allow for type erasure
struct ControlBlockBase
{
    std::atomic<size_t> _strong_count{0};

    virtual ~ControlBlockBase() = default; // Essential for polymorphism!

    // Pure virtual function to destroy the managed object
    virtual void destroy_object() = 0;
};

// A derived control block that knows the type T of the object it manages
template <typename T>
struct ControlBlockImpl : public ControlBlockBase
{
    // We construct the T object in place within a buffer inside this class
    // std::aligned_storage_t ensures memory is correctly aligned for T
    std::aligned_storage_t<sizeof(T), alignof(T)> _storage;

    // This method will be called to destruct the T object
    void destroy_object() override
    {
        // Cast the storage back to a T* and call the destructor explicitly
        reinterpret_cast<T*>(&_storage)->~T();
    }
};

template <typename T, typename... Args>
SharedPointer<T> make_shared(Args&&... args)
{
    // 1. Perform the SINGLE heap allocation.
    // We allocate our combined object that holds both the control block logic
    // and the storage for T.
    auto* block_impl = new ControlBlockImpl<T>();

    // 2. Construct the T object in the aligned storage within the control block.
    // This is "placement new". We pass the arguments perfectly forwarded.
    T* pointer = new (&block_impl->_storage) T(std::forward<Args>(args)...);

    // 3. Set the initial reference count.
    block_impl->_strong_count = 1;

    // 4. Return a SharedPointer using the private constructor.
    // It now shares ownership of the single allocated block.
    return SharedPointer<T>(pointer, block_impl);
}
*/
