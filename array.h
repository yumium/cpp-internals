#include <cstddef> 
#include <stdexcept>
#include <iterator>
#include <algorithm>

template <typename T, std::size_t Size>
class array
{
public:
    // The C-style array is a public member to allow for aggregate initialization.
    // If providing fewer members, the rest of members are value initialised
    T m_data[Size];

    // Example of Rule of Zero here
    // As no additional resource management, and compiler defaults are correct
    // it's best practice to not declare any custom operators
    // (resources here mean dynamic memory, file handlers, network sockets)

    // --- Element Access ---

    // Provides unchecked access. const and non-const versions.
    constexpr T& operator[](std::size_t index) noexcept
    {
        return m_data[index];
    }

    constexpr const T& operator[](std::size_t index) const noexcept
    {
        return m_data[index];
    }

    // Provides bounds-checked access.
    T& at(std::size_t index)
    {
        if (index >= Size) i{
            throw std::out_of_range("array::at: index out of range");
        }
        return m_data[index];
    }

    constexpr const T& at(std::size_t index) const
    {
        if (index >= Size) {
            throw std::out_of_range("array::at: index out of range");
        }
        return m_data[index];
    }

    constexpr T& front() noexcept
    {
        return m_data[0];
    }

    constexpr const T& front() const noexcept
    {
        return m_data[0];
    }

    constexpr T& back() noexcept
    {
        return m_data[Size - 1];
    }

    constexpr const T& back() const noexcept
    {
        return m_data[Size - 1];
    }

    // Direct access to the underlying C-style array
    constexpr T* data() noexcept
    {
        return m_data;
    }

    constexpr const T* data() const noexcept
    {
        return m_data;
    }

    // --- Iterators ---
    
    // Iterators here are just simple pointers
    // because data is stored contiguously
    constexpr T* begin() noexcept { return data(); }
    constexpr const T* begin() const noexcept { return data(); }
    constexpr const T* cbegin() const noexcept { return begin(); }

    constexpr T* end() noexcept { return data() + Size; }
    constexpr const T* end() const noexcept { return data() + Size; }
    constexpr const T* cend() const noexcept { return end(); }
    
    // Reverse iterators
    using reverse_iterator = std::reverse_iterator<T*>;
    using const_reverse_iterator = std::reverse_iterator<const T*>;

    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    // --- Capacity ---

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return Size == 0;
    }

    constexpr std::size_t size() const noexcept
    {
        return Size;
    }

    constexpr std::size_t max_size() const noexcept
    {
        return Size;
    }

    // --- Operations ---

    void fill(const T& value)
    {
        for (std::size_t i = 0; i < Size; ++i) {
            m_data[i] = value;
        }
    }

    void swap(array& other) noexcept
    {
        std::swap_ranges(begin(), end(), other.begin());
    }
};

// --- Example Usage ---

#include <iostream>

int main()
{
    // Aggregate initialization - this works because we have no user-defined constructors
    array<int, 5> my_arr = {10, 20, 30, 40, 50};

    // Use a range-based for loop thanks to begin() and end()
    std::cout << "Elements: ";
    for (const auto& val : my_arr) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // Access elements
    my_arr[1] = 25;
    std::cout << "Element at index 1: " << my_arr.at(1) << std::endl;
    std::cout << "Front element: " << my_arr.front() << std::endl;
    std::cout << "Back element: " << my_arr.back() << std::endl;

    // Use a reverse iterator
    std::cout << "Elements in reverse: ";
    for (auto it = my_arr.rbegin(); it != my_arr.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // Use fill
    my_arr.fill(7);
    std::cout << "After filling with 7: ";
    for (const auto& val : my_arr) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    return 0;
}