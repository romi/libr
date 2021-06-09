#ifndef ROMI_ROVER_BUILD_AND_TEST_THREADSAFEQUEUE_H
#define ROMI_ROVER_BUILD_AND_TEST_THREADSAFEQUEUE_H


#include "log.h"
#include "IThreadsafeQueue.h"


template<typename T>
class ThreadsafeQueue : IThreadsafeQueue< T >{
    std::queue<T> queue_;
    mutable std::mutex mutex_{};

public:
    ThreadsafeQueue();
    ThreadsafeQueue(const ThreadsafeQueue<T> &) = delete ;
    ThreadsafeQueue& operator=(const ThreadsafeQueue<T> &) = delete ;

    ThreadsafeQueue(ThreadsafeQueue<T>&& other) noexcept(false){
        std::lock_guard<std::mutex> lock(mutex_);
        if (!queue_.empty()) {
            throw std::runtime_error("Move ctr queue already has entries.");
        }
        queue_ = std::move(other.queue_);
    }

    virtual ~ThreadsafeQueue() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!queue_.empty()) {
            r_err("destroying ThreadsafeQueue with %d elements.", queue_.size());
        }
    }

    [[nodiscard]] unsigned long size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return {};
        }
        T tmp = queue_.front();
        queue_.pop();
        return tmp;
    }

    void push(const T &item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
    }
};

template<typename T>
ThreadsafeQueue<T>::ThreadsafeQueue() : queue_(){

}


#endif //ROMI_ROVER_BUILD_AND_TEST_THREADSAFEQUEUE_H
