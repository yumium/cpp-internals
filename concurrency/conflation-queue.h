#include <functional>
#include <unordered_map>
#include <list>
#include <memory>
#include <iostream>
#include <string>

#include <catch2/catch_test_macros.hpp> // For testing.

struct Instrument
{
    Instrument() = default;

    explicit Instrument(int id) 
    : id_{ id } 
    {
    }

    explicit Instrument(int id, std::string data) 
    : id_{ id }
    , data_{ std::move(data) } 
    { 
    }

    int GetId() const { return id_; }
    const std::string& GetData() const { return data_; }

private:
    int id_{ };
    std::string data_{ };
};

template <>
struct std::hash<Instrument>
{
    std::size_t operator()(const Instrument& item) const
    {
        return std::hash<int>{}( item.GetId() );
    }
};


template <typename T, typename Hasher = std::hash<T>>
class ConflationQueue
{
protected:

    using ListIterator = typename std::list<T>::iterator;

public:

    // Choose std::list because we are optimizing for the scenario
    // in which the producer pushes updates faster than the consumer
    // can pop these updates off. The most frequent update is therefore
    // additions, which are O(1) in all-cases, which is better than amortized O(1).

    // Furthermore, since we aren't too concerned with speeds of reads (only speeds)
    // of writes (given the fast consumer), we favor a list over a vector for 
    // the following reasons.

    // a) List growing doesn't invalidate existing iterators.
    // b) Removing from the front is O(1) in all cases, but O(n) with a vector
    // since a vector shifts items down over by 1.

    // Supporting a fast-consumer, which would favor a vector or array-based
    // data-access pattern (contiguous), would preclude the case of needing a
    // conflation queue in the first place.

    void write(const T& item)
    {
        auto hash_value = hasher_(item);
        if (items_.count(hash_value))
        {
            auto item_location = items_.at(hash_value);
            *item_location = item;
        }
        else
        {
            list_.push_back(item);
            auto item_location = list_.end();
            std::advance(item_location, -1);
            items_.insert({ hash_value,  item_location});
            size_++;
        }
    }

    std::shared_ptr<T> read()
    {
        if (size_ == 0)
            return nullptr;

        const auto& item = list_.front();
        auto hash_value = hasher_(item);

        list_.pop_front();
        items_.erase(hash_value);
        size_--;

        return std::make_shared<T>(item);
    }

    // Can also return std::optional.

    bool try_read(T& item)
    {
        if (size_ == 0)
            return false;

        item = list_.front();
        auto hash_value = hasher_(item);

        list_.pop_front();
        items_.erase(hash_value);
        size_--;

        return true;
    }

    [[nodiscard]] bool empty() const
    {
        return size_ == 0;
    }

    [[nodiscard]] std::size_t size() const
    {
        return size_;
    }

    [[nodiscard]] Hasher get_hasher() const
    {
        return hasher_;
    }


private:

    Hasher hasher_;
    std::size_t size_{ };
    std::list<T> list_{ };
    std::unordered_map<std::size_t, ListIterator> items_{ };
};

TEST_CASE( "Write and read from conflation queue.", "[conflation_queue]" ) 
{
    auto queue = ConflationQueue<Instrument>();

    SECTION("Write duplicate item to queue, then read.")
    {
        queue.write(Instrument{ 1, "Hello" });
        queue.write(Instrument{ 1, "There" });
        Instrument item;
        auto can_read = queue.try_read(item);

        REQUIRE( can_read == true );
        REQUIRE( item.GetData() == "There" );
        REQUIRE( queue.empty() == true );
        REQUIRE( queue.size() == 0 );
    }
}