//
// Created by co2ma on 2023/6/13.
//

#ifndef SPINLOCK_H
#define SPINLOCK_H


#include <atomic>
#include <thread>

class Spinlock
{
	enum { FREE = 0, LOCKED = 1 };
public:
	Spinlock() : flag_(FREE) {}

	void lock() {
		for (int i = 0; flag_.load(std::memory_order_relaxed) || flag_.exchange(LOCKED, std::memory_order_acquire); i ++) {
			if (i == 8) {
				i = 0;
				std::this_thread::yield();
			}
		}
	}

	void unlock() {
		flag_.store(FREE, std::memory_order_release);
	}
private:
	std::atomic<uint32_t> flag_;
};


#endif //SPINLOCK_H
