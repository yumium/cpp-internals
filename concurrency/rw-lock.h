#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class ReaderWriterLock
{
public:

    void ReaderLock()
    {
        std::unique_lock lock{ mutex_ };

        if (writing_)
        {
            readers_queued_++;
            readers_ready_.wait(lock, [this] { return !writing_; });
            readers_queued_--;
        }

        readers_++;
    }
    void ReaderUnlock() 
    { 
        std::unique_lock lock{ mutex_ };

        if (--readers_ == 0)
        {
            if (writers_queued_ > 0)
            {
                lock.unlock();
                writers_ready_.notify_one();
            }
        }
    }

    void WriterLock() 
    { 
        std::unique_lock lock{ mutex_ };

        if (readers_ > 0)
        {
            writers_queued_++;
            writers_ready_.wait(lock, [this] { return readers_ == 0 && !writing_; });
            writers_queued_--;
        }

        writing_ = true;
    }

    void WriterUnlock()
    { 
        std::unique_lock lock{ mutex_ };
        
        writing_ = false;

        if (writers_queued_ > 0)
        {
            lock.unlock();
            writers_ready_.notify_one();
        }
        else
        {
            if (readers_queued_ > 0)
            {
                lock.unlock();
                readers_ready_.notify_all();
            }
        }
    }

private:

    std::mutex mutex_{ };
    std::condition_variable writers_ready_{ };
    std::condition_variable readers_ready_{ };


    unsigned int readers_{ 0 };
    bool writing_{ false };
    unsigned int writers_queued_{ 0 };
    unsigned int readers_queued_{ 0 };
};