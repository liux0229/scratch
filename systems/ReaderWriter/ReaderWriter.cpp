// FLAGS: -pthread -O3

#include <condition_variable>
#include <mutex>
#include <cassert>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <thread>
#include <iostream>

using namespace std;
using namespace std::chrono;

class ReaderWriterLock {
 public:
  void lockReader() {
    unique_lock<mutex> lock(m_);
    while (nWriter_ > 0) {
      cv_.wait(lock);
    }
    ++nReader_;

    // read exclusivity obtained (I incremented nReader_)
    // cout << "accquiring reader lock; nReader_ = " << nReader_ << endl;
  }
  void unlockReader() {
    unique_lock<mutex> lock(m_);
    assert(nWriter_ == 0);
    assert(nReader_ > 0);
    if (--nReader_ == 0) {
      cv_.notify_one();
    }
    // cout << "release reader lock: " << nReader_ << endl;
  }
  void lockWriter() {
    unique_lock<mutex> lock(m_);
    while (nWriter_ > 0 || nReader_ > 0) {
      cv_.wait(lock);
    }
    ++nWriter_;
    // write exclusivity obtained (I changed nWriter_ from 0 to 1)
    // cout << "accquiring writer lock; nWriter = " << nWriter_ << endl;
  }
  void unlockWriter() {
    unique_lock<mutex> lock(m_);
    assert(nWriter_ == 1);
    --nWriter_;
    cv_.notify_one();
    // cout << "release writer lock: " << nWriter_ << endl;
  }

 private:
  mutex m_;
  condition_variable cv_;
  int nReader_{0};
  int nWriter_{0};
};


class ReaderWriterLock2 {
public:
  void lockReader() {
    lock_guard lock(cm_);
    if (readers_++ == 0) {
      // Need to exclude writers
      rm_.lock();
    }
  }
  void unlockReader() {
    lock_guard lock(cm_);
    if (--readers_ == 0) {
      // open up for writers
      rm_.unlock();
    }
  }
  void lockWriter() {
    rm_.lock();
  }
  void unlockWriter() {
    rm_.unlock();
  }
private:
  mutex rm_;
  mutex cm_;
  int readers_{0};
};

class Runner {
 public:
  using Clock = high_resolution_clock;
  using Duration = Clock::duration;

  void run(int nReader, int nWriter) {
    vector<thread> v;
    for (int i = 0; i < nReader; ++i) {
      v.emplace_back([this]() {
        for (int i = 0; i < 100; i++) {
          reader();
        }
      });
    }
    for (int i = 0; i < nWriter; ++i) {
      v.emplace_back([this]() {
        for (int i = 0; i < 100; i++) {
          writer();
        }
      });
    }

    for (auto& t : v) {
      t.join();
    }

    assert(nReader_ == 0);
    assert(nWriter_ == 0);
#if 0
    cout << "reader: " << duration_cast<milliseconds>(readerDuration_).count()
         << " ms; writer: "
         << duration_cast<milliseconds>(writerDuration_).count() << " ms"
         << endl;
#endif
  }

  Duration readerDuration() const {
    return readerDuration_;
  }

  Duration writerDuration() const {
    return writerDuration_;
  }

 private:
  void reader() { 
    lock_.lockReader(); 
    assert(nWriter_ == 0);
    ++nReader_;
    readerDuration_ += work();
    --nReader_;
    lock_.unlockReader();
  }

  void writer() {
    lock_.lockWriter();
    assert(nWriter_ == 0);
    assert(nReader_ == 0);
    ++nWriter_;
    writerDuration_ += work();
    --nWriter_;
    lock_.unlockWriter();
  }

  Duration work() {
    int us = rand() % 1000;
    auto now = Clock::now();
    this_thread::sleep_for(microseconds(us));
    return Clock::now() - now;
  }

  ReaderWriterLock lock_;
  Duration readerDuration_{};
  Duration writerDuration_{};
  atomic<int> nReader_{0};
  atomic<int> nWriter_{0};
};

int main() {
  int nReader, nWriter;
  cin >> nReader >> nWriter;

  double ratio = 0;
  for (int i = 0; i < 3; ++i) {
    Runner runner;
    runner.run(nReader, nWriter);
    ratio += 1.0 * runner.readerDuration() / runner.writerDuration();
  }

  cout << ratio / 3 << endl;
}
