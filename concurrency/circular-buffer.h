#include <memory>

#include <catch2/catch_test_macros.hpp> // For testing.

template <typename T>
class CircularBuffer
{
public:
    CircularBuffer() : CircularBuffer(0) { }
    explicit CircularBuffer(std::size_t capacity) : capacity_{ capacity }, ptr_{ std::make_unique<T[]>(capacity) } { }

    std::shared_ptr<T> read()
    {
        if (size_ == 0)
            return nullptr;

        auto item = std::make_shared<T>(ptr_[read_idx_]);

        increment_read();

        return item;
    }

    // Can also return std::optional.

    bool try_read(T& item)
    {
        if (size_ == 0)
            return false;

        item = ptr_[read_idx_];

        increment_read();

        return true;
    }

    void write(const T& item)
    {
        ptr_[write_idx_] = item;

        increment_write();
    }

    void reset()
    {
        size_ = 0;
        write_idx_ = read_idx_;
    }

    [[nodiscard]] bool full() const 
    {
        return size_ == capacity_;
    }

    [[nodiscard]] bool empty() const
    {
        return size_ == 0;
    }

    [[nodiscard]] std::size_t size() const
    {
        return size_;
    }

    [[nodiscard]] std::size_t capacity() const
    {
        return capacity_;
    }

private:

    void increment_read()
    {
        read_idx_ = (read_idx_ + 1) % capacity_;
        size_--;
    }

    void increment_write()
    {
        // Ensure we are always reading the oldest piece of data.
        auto write_idx = (write_idx_ + 1) % capacity_;
        if (write_idx == read_idx_ && size_ == capacity_ - 1)
            read_idx_ = (read_idx_ + 1) % capacity_;

        write_idx_ = write_idx;
        size_ = std::min(size_ + 1, capacity_);
    }

    std::size_t capacity_{ };
    std::size_t size_{ };
    std::size_t read_idx_{ };
    std::size_t write_idx_{ };
    std::unique_ptr<T[]> ptr_{ nullptr };
};

TEST_CASE( "Buffer is overwritten on subsequent writes.", "[buffer]" ) 
{
    auto buffer = CircularBuffer<int>(2);

    SECTION("Increase buffer size, then deplete it.")
    {
        buffer.write(1);
        buffer.write(2);
        buffer.write(3); // Overwrite item at index 0.
        auto read_one = buffer.read(); // Read oldest item, which is at index 1 (item = 2)
        auto read_two = buffer.read(); // Read next item, which is at index 0 (item = 3)

        REQUIRE( read_one != nullptr );
        REQUIRE( read_two != nullptr );
        REQUIRE( *read_one == 2 );
        REQUIRE( *read_two == 3 );
        REQUIRE( buffer.empty() == true );
        REQUIRE( buffer.size() == 0 );
        REQUIRE( buffer.full() == false );
    }

    SECTION ("Reset the buffer.")
    {
        buffer.write(1);
        buffer.reset();

        REQUIRE( buffer.empty() == true );
        REQUIRE( buffer.size() == 0 );
        REQUIRE( buffer.full() == false );
    }
}
