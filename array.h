#include <cstddef>
#include <utility>
#include <array>
#include <stdexcept>

template <typename T, std::size_t Size>
class array
{
public:

    array() : ptr_{ new T[Size] } { }
    array(const array& arr)
    {
        ptr_ = new T[Size];

        for (std::size_t i{}; i < Size; ++i)
            ptr_[i] = arr.ptr_[i];
    }
    array& operator=(const array& arr)
    {
        if (this == &arr)
            return *this;

        ptr_ = new T[Size];

        delete[] ptr_;
        ptr_ = nullptr;

        for (std::size_t i{}; i < Size; ++i)
            ptr_[i] = arr.ptr_[i];
    
        return *this;
    }
    array(array&& arr)
    {
        ptr_ = std::exchange(arr.ptr_, nullptr);
    }
    array& operator=(array&& arr)
    {
        if (this == &arr)
            return *this;

        ptr_ = std::exchange(arr.ptr_, nullptr);

        return *this;
    }
    ~array() { delete[] ptr_; }

    T& operator[](std::size_t index)
    {
        return ptr_[index];
    }
    T& at(std::size_t index) const
    {
        if (index >= Size)
            throw std::logic_error("Out of bounds.");
        
        return ptr_[index];
    }
    void fill(const T& value)
    {
        for (std::size_t i{}; i < Size; ++i)
            ptr_[i] = value;
    }
    T& front() const { return *(ptr_[0]); }
    T& back() const { return *(ptr_[Size - 1]); }
    T* data() const { return ptr_; }
    T* begin() const { return ptr_; }
    T* end() const { return ptr_[Size]; }
    constexpr std::size_t size() const { return Size; }
    constexpr bool empty() const { return Size == 0; }

private:

    T* ptr_{nullptr};
};

int main()
{

    return 0;
}