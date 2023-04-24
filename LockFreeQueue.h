#ifndef CODE_LOCKFREEQUEUE_H
#define CODE_LOCKFREEQUEUE_H

#include <atomic>
#include <vector>

template <typename T, size_t N = 1024>
class LockFreeQueue
{
    struct Element
    {
        std::atomic<bool> full_;
        T data_;
    };

public:
    explicit LockFreeQueue(): data_(N), read_index_(0), write_index_(0) {}

    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue(LockFreeQueue&&) = delete;
    LockFreeQueue& operator = (const LockFreeQueue&) = delete;
    LockFreeQueue& operator = (LockFreeQueue&&) = delete;

    // queue full : write_index_ == read_index_ + data_.size(), return false
    bool try_push(T value)
    {
        size_t write_index;
        Element* e;
        do {
            write_index = write_index_.load(std::memory_order_relaxed);
            if (write_index >= read_index_.load(std::memory_order_relaxed) + data_.size())
            {
                return false;
            }
            size_t index = write_index % N;
            e = &data_[index];
        // 重试：该槽位不可写，或CAS失败
        } while (e->full_.load(std::memory_order_relaxed) ||
                !write_index_.compare_exchange_weak(write_index, write_index + 1, std::memory_order_release, std::memory_order_relaxed));
        // write_index_++ 不能保证数据已经写入，只是表示占据了这个槽位
        // 对于消费者来说，如果没有多余的标志位full_，不能保证读到数据（数据读写是非原子的）
        // 因此，full_作为数据发布标志，是必要的
        e->data_ = std::move(value);
        e->full_.store(true, std::memory_order_release);
        return true;
    }
    // queue empty : write_index_ == read_index_
    bool try_pop(T& value)
    {
        size_t read_index;
        Element* e;
        do {
            read_index = read_index_.load(std::memory_order_relaxed);
            if (read_index >= write_index_.load(std::memory_order_relaxed)) {
                return false;
            }
            size_t index = read_index % N;
            e = &data_[index];
        // 重试：该槽位不可读，或CAS失败
        } while (!e->full_.load(std::memory_order_relaxed) ||
                 !read_index_.compare_exchange_weak(read_index, read_index + 1, std::memory_order_release, std::memory_order_relaxed));
        value = std::move(e->data_);
        e->full_.store(false, std::memory_order_release);
        return true;
    }

private:
    std::vector<Element> data_;
    std::atomic<size_t> read_index_;
    std::atomic<size_t> write_index_;
};

#endif
