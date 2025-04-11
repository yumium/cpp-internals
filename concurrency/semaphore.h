#include <atomic>


class Semaphore
{
public:

    Semaphore(int count = 1)
    : count_{ count }
    {
    }

    void acquire() 
    { 
        int count = 0;
        while ((count = count_.load(std::memory_order_relaxed)) <= 0)
        {
            if (count_.compare_exchange_strong(count, count - 1, std::memory_order_acq_rel))
                break;
        }
    }
    void release() 
    { 
        count_.fetch_add(1, std::memory_order_acq_rel);
    }

private:
    std::atomic<int> count_{ 1 };
};