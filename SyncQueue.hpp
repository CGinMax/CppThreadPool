#ifndef _SYNCQUEUE
#define _SYNCQUEUE

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <iterator>

template<class T>
class SyncQueue {
public:
	SyncQueue(std::size_t maxSize) : 
		m_max_size(maxSize),
		m_stop(false)
	{
	}

	void Put(T& task) 
	{
		Add(task);
	}

	void Put(T&& task)
	{
		Add(std::forward<T>(task));
	}

	std::list<T> AllTask()
	{
		std::list<T> tasks;
		std::unique_lock<std::mutex> locker(m_mutex);
		m_empty_cond.wait(locker, [this](){ return m_stop || !IsEmpty(); });
		if (m_stop) {
			return tasks;
		}
		tasks = std::move(m_queue);

		m_full_cond.notify_one();
		return tasks;

	}

	T Take()
	{
		T t;
		std::unique_lock<std::mutex> locker(m_mutex);
		m_empty_cond.wait(locker, [this](){ return m_stop || !IsEmpty(); });
		if (m_stop) {
			 return nullptr;
		}

		t = m_queue.front();
		m_queue.pop_front();

		m_full_cond.notify_one();
		return t;
	}

	bool Empty() {
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_queue.empty();
	}

	bool Full() {
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_queue.size() == m_max_size;
	}

	size_t Size() const 
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_queue.size();
	}

	size_t MaxSize() const 
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_max_size;
	}

	void Stop()
	{
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			m_stop = true;
		}

		m_empty_cond.notify_all();
		m_full_cond.notify_all();
	}

private:
	bool IsEmpty() {
		bool isEmpty = m_queue.empty();
		if (isEmpty) {
			std::cout << "线程:" << std::this_thread::get_id() << ",队列已空\n";
		}

		return isEmpty;
	}

	bool IsFull() 
	{
		bool isFull = m_queue.size() >= m_max_size;
		if (isFull) {
			std::cout << "队列已满\n";
		}

		return isFull;
	}
	template<typename F>
	void Add(F&& task)
	{
		std::unique_lock<std::mutex> locker(m_mutex);
		m_full_cond.wait(locker, [this](){ return m_stop || !IsFull(); });
		if (m_stop) {
			return ;
		}
		m_queue.emplace_back(std::forward<F>(task));

		m_empty_cond.notify_one();
	}

private:
	std::size_t m_max_size;
	bool m_stop;
	std::list<T> m_queue; // 任务队列
	std::mutex m_mutex;
	std::condition_variable m_empty_cond; // 是否为空的条件变量
	std::condition_variable m_full_cond; // 是否满的条件变量
};

#endif // _SYNCQUEUE
