#ifndef CPP11THREADPOOL_H
#define CPP11THREADPOOL_H
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <future>
#include <queue>
#include <functional>
#include <vector>

class Cpp11ThreadPool{

public:
    using task_t = std::function<void()>;
    explicit Cpp11ThreadPool(int threadNum = std::thread::hardware_concurrency());
    ~Cpp11ThreadPool();

    template<typename Function, typename... Args>
    std::future<typename std::result_of<Function(Args...)>::type> add(Function&& f, Args&&... args);

private:
    Cpp11ThreadPool(Cpp11ThreadPool& other) = delete;
    Cpp11ThreadPool& operator=(Cpp11ThreadPool& other) = delete;
    Cpp11ThreadPool(const Cpp11ThreadPool& other) = delete;
    Cpp11ThreadPool* operator=(const Cpp11ThreadPool& other) = delete;

    std::vector<std::thread> m_threads;
    std::queue<task_t> m_tasks;
    std::mutex m_mtx;
    std::condition_variable m_cond;
    std::atomic_bool m_isStop;
};

Cpp11ThreadPool::Cpp11ThreadPool(int threadNum)
    : m_isStop{false}
{
    for (int i = 0; i < threadNum; i++) {
        m_threads.push_back(std::thread([this]{
            while (!m_isStop.load(std::memory_order_acquire)) {
                task_t task;
                {
                    std::unique_lock<std::mutex> locker(m_mtx);
                    m_cond.wait(locker, [this]{ return m_isStop.load(std::memory_order_acquire) || !m_tasks.empty();});
                    if (m_isStop.load(std::memory_order_acquire)) {
                        return ;
                    }
                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }
                task();
            }
        }));
    }
}

Cpp11ThreadPool::~Cpp11ThreadPool()
{
    m_isStop.store(true, std::memory_order_relaxed);
    m_cond.notify_all();
    for (auto& thd : m_threads) {
        if (thd.joinable()) {
            thd.join();
        }
    }
}

template<typename Function, typename... Args>
std::future<typename std::result_of<Function(Args...)>::type> Cpp11ThreadPool::add(Function&& f, Args&&... args)
{
    using return_type = typename std::result_of<Function(Args...)>::type;
    using task = std::packaged_task<return_type()>;

    // std::packaged_task don't support copy ctor
    auto t = std::make_shared<task>(std::bind(std::forward<Function>(f), std::forward<Args>(args)...));
    auto ret = t->get_future();

    {
        std::lock_guard<std::mutex> locker(m_mtx);
        if (m_isStop.load(std::memory_order_acquire)) {
            throw std::runtime_error("The thread pool is stop!!!");
        }
        m_tasks.emplace([t]{(*t)();});
    }
    m_cond.notify_one();
    return ret;
}

#endif // CPP11THREADPOOL_H
