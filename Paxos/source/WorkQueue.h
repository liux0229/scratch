#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>

namespace Paxos {

  template<typename T>
  class WorkQueue {
  public:
    WorkQueue(size_t maxSize)
    : maxSize_(maxSize) {
    }

    // block if the queue is full
    template<typename P>
    void enque(P&& item);

    // block if the queue is empty
    T deque();
  private:
    using Lock = std::unique_lock<std::mutex>;
    const size_t maxSize_;

    std::mutex m_;
    std::condition_variable cvFull_;
    std::condition_variable cvEmpty_;
    std::deque<T> queue_;
  };

  template<typename T>
  template<typename P>
  void WorkQueue<T>::enque(P&& item) {
    Lock lock(m_);
    while (queue_.size() >= maxSize_) {
      cvFull_.wait(lock);
    }
    queue_.push_back(std::forward<P>(item));
    cvEmpty_.notify_one();
  }

  template<typename T>
  T WorkQueue<T>::deque() {
    Lock lock(m_);
    while (queue_.empty()) {
      cvEmpty_.wait(lock);
    }
    T ret{ std::move(queue_.front()) };
    queue_.pop_front();
    cvFull_.notify_one();
    return ret;
  }

}