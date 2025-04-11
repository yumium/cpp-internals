#include <unordered_map>
#include <list>
#include <iterator>

#include <catch2/catch_test_macros.hpp> // For testing.


template <typename Key, typename Value>
class LeastRecentlyUsedCache
{
protected:

    using ListIterator = typename std::list<Key>::iterator;

public:

    LeastRecentlyUsedCache() : LeastRecentlyUsedCache(1) { }
    explicit LeastRecentlyUsedCache(std::size_t capacity)
    : capacity_{ capacity } 
    { 
    }

    // Can also use std::optional.

    bool try_get(const Key& key, Value& value)
    {
        if (!items_.count(key))
            return false;

        on_item_accessed(key);

        value = items_.at(key).first;

        return true;
    }

    void add(const Key& key, Value& value)
    {
        if (items_.size() == capacity_)
        {
            auto least_recently_used = keys_.front();
            keys_.pop_front();
            items_.erase(least_recently_used);
        }

        if (items_.count(key))
        {
            const auto& [value, iterator] = items_.at(key);
            keys_.erase(iterator);
            items_.erase(key);
        }

        keys_.push_back(key);
        auto end_iterator = keys_.end();
        std::advance(end_iterator, -1);
        items_.insert({key, {value, end_iterator}});
    }

    [[nodiscard]] bool empty() const
    {
        return items_.size() == 0;
    }

    [[nodiscard]] bool full() const
    {
        return items_.size() == capacity_;
    }


private:

    void on_item_accessed(const Key& key)
    {
        const auto& [value, iterator] = items_.at(key);
        keys_.erase(iterator);
        keys_.push_back(key);
        auto end_iterator = keys_.end();
        std::advance(end_iterator, -1);
        items_.insert({key, {value, end_iterator}});
    }

    std::size_t capacity_{ };
    std::list<Key> keys_{ };
    std::unordered_map<Key, std::pair<Value, ListIterator>> items_{ };
};