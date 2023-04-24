//
// Created by co2ma on 2023/1/24.
//

#ifndef CODE_BLOCKINGQUEUE_H
#define CODE_BLOCKINGQUEUE_H

#include <mutex>
#include <condition_variable>
#include <deque>

template<typename T>
class BlockingQueue
{
public:
    explicit BlockingQueue(size_t capacity = 1024) : capacity_(capacity) {}

    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue(BlockingQueue&&) = delete;
    BlockingQueue& operator = (const BlockingQueue&) = delete;
    BlockingQueue& operator = (BlockingQueue&&) = delete;

    void push(T x)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            notFull_.wait(lock, [&]{return queue_.size() < capacity_;});
            queue_.push_back(std::move(x));
        }
        // 这里需要提前释放锁
        // if not, 被通知的线程会立即再次阻塞，等待通知线程释放锁
        notEmpty_.notify_one();
    }

    bool try_push(T x)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (queue_.size() == capacity_) {
                return false;
            }
            queue_.push_back(std::move(x));
        }
        notEmpty_.notify_one();
        return true;
    }

    // pop() and try_pop() accept a reference to the item rather than returning the item.
    // This provides the strong exception guarantee.
    // If moving the element from the queue to item throws, the queue remains unchanged.
    void pop(T &x)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            notEmpty_.wait(lock, [&]{return !queue_.empty();});
            x = std::move(queue_.front());
            queue_.pop_front();
        }
        notFull_.notify_one();
    }

    bool try_pop(T &x)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                return false;
            }
            x = std::move(queue_.front());
            queue_.pop_front();
        }
        notFull_.notify_one();
        return true;
    }

    size_t size()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    std::deque<T> queue_;
    size_t capacity_;
    std::mutex mutex_;
    std::condition_variable notEmpty_, notFull_;
};

#endif //CODE_BLOCKINGQUEUE_H
