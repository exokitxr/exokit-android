#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <mutex>
#include <condition_variable>

namespace std {
class semaphore {
  private:
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_ = 0; // Initialized as locked.

  public:
    void notify() {
      std::unique_lock<decltype(mutex_)> lock(mutex_);
      ++count_;
      condition_.notify_one();
    }

    void wait() {
      std::unique_lock<decltype(mutex_)> lock(mutex_);
      while (!count_) { // Handle spurious wake-ups.
        condition_.wait(lock);
      }
      --count_;
    }

    void block() {
      std::unique_lock<decltype(mutex_)> lock(mutex_);
      while (!count_) { // Handle spurious wake-ups.
        condition_.wait(lock);
      }
    }

    void eat() {
      std::unique_lock<decltype(mutex_)> lock(mutex_);
      --count_;
    }
};

class condition {
  private:
    std::mutex mutex_;
    std::condition_variable condition_;
    bool open_ = false;

  public:
    void open() {
      std::unique_lock<decltype(mutex_)> lock(mutex_);
      open_ = true;
      condition_.notify_all();
    }

    void close() {
      std::unique_lock<decltype(mutex_)> lock(mutex_);
      open_ = false;
    }

    void block() {
      std::unique_lock<decltype(mutex_)> lock(mutex_);
      while (!open_) {
        condition_.wait(lock);
      }
    }
};
}

#endif
