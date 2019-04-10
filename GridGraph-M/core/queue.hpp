
#ifndef QUEUE_H
#define QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class Queue {
	const size_t capacity;
	std::queue<T> queue;
	std::mutex mutex;
	std::condition_variable cond_full;
	std::condition_variable cond_empty;
public:
	Queue(const size_t capacity) : capacity(capacity) { }
	void push(const T & item) {
		std::unique_lock<std::mutex> lock(mutex);
		cond_full.wait(lock, [&]{ return !is_full(); });
		queue.push(item);
		lock.unlock();
		cond_empty.notify_one();
	}
	T pop() {
		std::unique_lock<std::mutex> lock(mutex);
		cond_empty.wait(lock, [&]{ return !is_empty(); });
		auto item = queue.front();
		queue.pop();
		lock.unlock();
		cond_full.notify_one();
		return item;
	}
	bool is_full() {
		return queue.size()==capacity;
	}
	bool is_empty() {
		return queue.empty();
	}
};

#endif
