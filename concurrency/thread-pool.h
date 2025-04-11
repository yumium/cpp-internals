#include <thread>
#include <vector>
#include <optional>
#include <chrono>
#include <atomic>
#include <mutex>
#include <queue>
#include <functional>

class ThreadPool
{
public:

    ThreadPool(std::size_t count = std::thread::hardware_concurrency())
    {
        try
        {
            for (unsigned int i = 0; i < count; i++)
                threads_.emplace_back([this] { Work(); });
        }
        catch (...)
        {
            done_.store(true, std::memory_order_relaxed);
            throw;
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool()
    {
        done_.store(true, std::memory_order_relaxed);

        for (auto& thread : threads_)
            thread.join();
    }

    template <typename Work>
    void submit(Work work)
    {
        std::scoped_lock lock{ mutex_ };
        queue_.push([=] { work(); });
    }

private:

    void Work()
    {
        while (!done_.load(std::memory_order_relaxed))
        {
            std::optional<std::function<void()>> task{ std::nullopt };
            {
                std::scoped_lock lock{  mutex_ };
                if (!queue_.empty())
                {
                    task = queue_.front();
                    queue_.pop();
                }
            }

            if (!task.has_value())
            {
                std::this_thread::yield();
                //std::this_thread::sleep_for(std::chrono::milliseconds{500})
            }
            else
            {
                try
                {
                    task.value()();
                }
                catch (...)
                {
                    // Log
                }
            }
        }
    }

    std::atomic<bool> done_{ false };
    mutable std::mutex mutex_{ };
    std::queue<std::function<void()>> queue_;
    std::vector<std::thread> threads_{ };

};