#include <utility>
#include <iostream>
#include <mutex>
#include <cstddef>

struct ControlBlock
{
    size_t count_{ };
    mutable std::mutex mutex_;
};

template <typename T>
class SharedPointer
{
public:

    SharedPointer() : SharedPointer(nullptr) { }
    SharedPointer(std::nullptr_t) { }
    SharedPointer(T* pointer)
    {
        Initialize(pointer);
    }

    SharedPointer(const SharedPointer& other) noexcept
    {
        CopyFrom(other);
    }

    SharedPointer& operator=(const SharedPointer& other) noexcept
    {
        if (this == &other)
            return *this;

        TryRelease();
        CopyFrom(other);

        return *this;
    }

    SharedPointer(SharedPointer&& other) noexcept
    {
        StealFrom(other);
    }

    SharedPointer& operator=(SharedPointer&& other) noexcept
    {
        if (this == &other)
            return *this;

        TryRelease();
        StealFrom(other);

        return *this;
    }

    ~SharedPointer()
    {
        TryRelease();
    }

    void reset(T* pointer)
    {
        reset();

        if (pointer == nullptr)
            return;

        Initialize(pointer);
    }

    void reset()
    {
        TryRelease();
    }

    size_t get_count() const
    {
        std::scoped_lock lock{ block_->mutex_ };
        return block_->count_;
    }

    T* get() const { return pointer_; }
    T* operator->() const { return pointer_; }
    T& operator*() const { return *pointer_; }
    operator bool() const noexcept { return pointer_ != nullptr; }

private:

    void CopyFrom(const SharedPointer& other) noexcept
    {
        pointer_ = other.pointer_;
        block_ = other.block_;

        if (block_ == nullptr)
            return;

        std::scoped_lock lock{ block_->mutex_ };
        block_->count_++;
    }

    void StealFrom(SharedPointer&& other) noexcept
    {
        pointer_ = std::exchange(other.pointer_, nullptr);
        block_ = std::exchange(other.block_, nullptr);
    }

    void Initialize(T* pointer)
    {
        pointer_ = pointer;
        block_ = new ControlBlock{1};
    }

    void TryRelease()
    {
        if (block_ == nullptr)
            return;
            
        {                      
            std::scoped_lock lock{ block_->mutex_ };

            if (--block_->count_ != 0)
                return;
        }

        delete pointer_;
        pointer_ = nullptr;

        delete block_;
        block_ = nullptr;
    }

    T* pointer_{ nullptr };
    ControlBlock* block_{ nullptr };
};

int main()
{   
    return 0;
}