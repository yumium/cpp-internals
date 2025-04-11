#include <utility>

template <typename T>
struct CustomDeleter
{
    void operator()(T* pointer) const
    {
        delete pointer;
    }
};

template <typename T, typename Deleter = CustomDeleter<T>>
class UniquePointer
{
public:
    UniquePointer() {}
    UniquePointer(T* pointer) : pointer_{ pointer }
    {
    }

    UniquePointer(const UniquePointer&) = delete;
    UniquePointer& operator=(const UniquePointer&) = delete;

    UniquePointer(UniquePointer&& other) noexcept
    {
        StealFrom(other);
    }

    UniquePointer& operator=(UniquePointer&& other) noexcept
    {
        if (this == &other)
            return *this;

        StealFrom(other);

        return *this;
    }

    ~UniquePointer()
    {
        Deleter deleter;
        deleter(pointer_);
    }

    T* release()
    {
        auto* temporaryPointer = pointer_;
        pointer_ = nullptr;
        return temporaryPointer;
    }

    void reset(T* pointer)
    {
        delete pointer_;
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

    void StealFrom(UniquePointer&& other)
    {
        pointer_ = std::exchange(other.pointer_, nullptr);
    }

    T* pointer_{ nullptr };
};

int main()
{
    return 0;
}