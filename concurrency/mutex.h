#include <thread>


class Mutex
{
public:

    void lock()
    {
        for (unsigned int i{ 0 }; flag_.load(std::memory_order_relaxed) || flag_.exchange(1, std::memory_order_acquire); i++)
        {
            if (i % 8 == 0 && i != 0)
                std::this_thread::yield();
        }
    }

    void unlock()
    {
        flag_.store(0, std::memory_order_release);
    }

private:

    std::atomic<unsigned int> flag_{ 0 };
};