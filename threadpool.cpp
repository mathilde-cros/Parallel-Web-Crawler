#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>

class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;

    void process_task_from_queue();

public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();
    void add_task_to_queue(const std::function<void()>& task);
};

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back(&ThreadPool::process_task_from_queue, this);
    }
}

ThreadPool::~ThreadPool() {
    for (std::thread& worker : workers) {
        worker.join();
    }
}

void ThreadPool::add_task_to_queue(const std::function<void()>& task) {
    std::unique_lock<std::mutex> lock(queueMutex);
    tasks.push(task);
    condition.notify_one();
}

void ThreadPool::process_task_from_queue() {
    int i = 0;
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (i == 0){ // If at beginning, wait for tasks to arrive
                condition.wait(lock, [this]() { return !tasks.empty(); });
            } 
            if (tasks.empty()) {
                return;
            } 
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
        i++;
    }
}