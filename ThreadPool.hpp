#include <functional>
#include <memory>
#include <atomic>

#include "SyncQueue.hpp"

class ThreadPool {
	using Task = std::function<void()>;
public:
	ThreadPool(int runNum = std::thread::hardware_concurrency()) : m_queue(100)
	{
		Start(runNum);
	}

	~ThreadPool()
	{
		Stop();
	}

	void AddTask(Task&& task) 
	{
		m_queue.Put(std::forward<Task>(task));
	}

	void AddTask(Task& task)
	{
		m_queue.Put(task);
	}

	void Start(int thdNums)
	{
		m_running = true;
		for (int i = 0; i < thdNums; ++i) {
			//m_thread_group.emplace_back(std::make_shared<std::thread>(&ThreadPool::RunTask, this));
			m_thread_group.emplace_back(std::thread(&ThreadPool::RunTask, this));

		}

	}
	void Stop()
	{
		std::call_once(m_flag, [this](){this->StopThreadGroup(); });
	}

	private:
	void RunTask()
	{
		while (m_running) {
			Task task = m_queue.Take();
			if (!task) {
				continue;
			}
			task();
		}
	}
	void StopThreadGroup() 
	{
		m_queue.Stop();
		m_running = false;
		for (auto& thd : m_thread_group) {
			if (thd.joinable()) {
				thd.join();
			}
		}
		m_thread_group.clear();
	}

private:
	std::atomic_bool m_running;
	//std::list<std::shared_ptr<std::thread>> m_thread_group;
	std::list<std::thread> m_thread_group;
	SyncQueue<Task> m_queue;
	std::once_flag m_flag;
};
