#include <utility>

/*
Constructors
- UniquePointer(std::nullptr_t)
- UniquePointer(T*)

release()
reset(T* = nullptr)
swap(UniquePointer&)
get()
operator*()
operator->()
*/



// $$ What does this deleter do?
template <typename T>
struct CustomDeleter
{
    void operator()(T* pointer) const
    {
        delete pointer;
    }
};

// UniquePointer enforces unique ownership by disabling copy
// Memory freeing is done similar to SharedPointer
template <typename T, typename Deleter = CustomDeleter<T>> // $$ what is the deleter?
class UniquePointer
{
public:
    UniquePointer() noexcept = default; 
    UniquePointer(std::nullptr_t) noexcept : UniquePointer() {}
    UniquePointer(T* pointer) : pointer_{ pointer } {}

    UniquePointer(const UniquePointer&) = delete;
    UniquePointer& operator=(const UniquePointer&) = delete;

    UniquePointer(UniquePointer&& other) noexcept
    {
        _steal_from(other);
    }

    UniquePointer& operator=(UniquePointer&& other) noexcept
    {
        if (this != &other)
        {
            // deletes current `pointer_`
            reset(other.release());
        }

        return *this;
    }

    ~UniquePointer()
    {
        Deleter deleter;
        deleter(pointer_);
    }

    T* release()
    {
        return std::exchange(pointer_, nullptr);
    }

    void reset(T* pointer = nullptr)
    {
        // prevent passing self and double delete
        if (pointer_ != pointer) delete pointer_;

        pointer_ = pointer;
    }

    void swap(UniquePointer& uniquePointer)
    {
        std::swap(pointer_, uniquePointer.pointer_);
    }

    Deleter get_deleter() const
    {
        return { };
    }

    T* get() const { return pointer_; }
    T& operator*() const { return *pointer_; }
    T* operator->() const { return pointer_; }

private:

    void _steal_from(UniquePointer&& other)
    {
        pointer_ = std::exchange(other.pointer_, nullptr);
    }

    T* pointer_{ nullptr };
};

int main()
{
    return 0;
}