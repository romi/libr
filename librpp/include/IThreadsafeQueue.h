#ifndef ROMI_ROVER_BUILD_AND_TEST_ITHREADSAFEQUEUE_H
#define ROMI_ROVER_BUILD_AND_TEST_ITHREADSAFEQUEUE_H

#include <mutex>
#include <optional>
#include <queue>

template<typename T>
class IThreadsafeQueue {

public:
    IThreadsafeQueue() = default;
    virtual ~IThreadsafeQueue() = default;

    [[nodiscard]] virtual unsigned long size() const = 0;
    virtual std::optional<T> pop() = 0;
    virtual void push(const T &item) = 0;
};


#endif //ROMI_ROVER_BUILD_AND_TEST_ITHREADSAFEQUEUE_H
