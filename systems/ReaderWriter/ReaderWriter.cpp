// FLAGS: -pthread -g -O3

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

class ReaderWriterLockReaderPreferredWeak {
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


class ReaderWriterLockReaderPreferredWeakMutex {
public:
  void lockReader() {
    lock_guard<mutex> lock(cm_);
    if (readers_++ == 0) {
      // Need to exclude writers
      rm_.lock();
    }
  }
  void unlockReader() {
    lock_guard<mutex> lock(cm_);
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

class ReaderWriterLockReaderPreferredStrongMutex {
public:
  void lockReader() {
    lock_guard<mutex> lock(cm_);
    if (readers_++ == 0) {
      // Need to exclude writers
      rm_.lock();
    }
  }
  void unlockReader() {
    lock_guard<mutex> lock(cm_);
    if (--readers_ == 0) {
      // open up for writers
      rm_.unlock();
    }
  }
  void lockWriter() {
    wm_.lock();
    rm_.lock();
  }
  void unlockWriter() {
    rm_.unlock();
    wm_.unlock();
  }
private:
  mutex rm_;
  mutex wm_;
  mutex cm_;
  int readers_{0};
};

class ReaderWriterLockWriterPreferredSlow {
 public:
  void lockReader() {
    unique_lock<mutex> lock(m_);
    // nWWaiter_ > 0 implies nWriter_ > 0
    while (nWWaiter_ > 0) {
      cv_.wait(lock);
    }
    ++nReader_;
  }
  void unlockReader() {
    unique_lock<mutex> lock(m_);
    assert(nWriter_ == 0);
    assert(nReader_ > 0);
    if (--nReader_ == 0) {
      // must notify all readers and writers
      // as a notified reader may not be able to make progress

      // notify threads which are waiting for the above condition
      cv_.notify_all();
    }
  }
  void lockWriter() {
    unique_lock<mutex> lock(m_);
    ++nWWaiter_;
    while (nWriter_ > 0 || nReader_ > 0) {
      cv_.wait(lock);
    }
    ++nWriter_;

    // We could have decremented nWWaiter_ at this point; but doing so would not
    // readers making progress, as they would still be blocked by this writer.
    // So let's delay its execution.
  }
  void unlockWriter() {
    unique_lock<mutex> lock(m_);
    assert(nWriter_ == 1);
    --nWriter_;
    --nWWaiter_;
    // notify all readers and writers, as a notified reader may not be able to
    // make progress
    cv_.notify_all();
  }

 private:
  mutex m_;

  condition_variable cv_;

  int nReader_{0};
  int nWriter_{0};
  int nWWaiter_{0};
};

class ReaderWriterLockWriterPreferred {
 public:
  void lockReader() {
    unique_lock<mutex> lock(m_);
    // nWWaiter_ > 0 implies nWriter_ > 0
    while (nWWaiter_ > 0) {
      cvReader_.wait(lock);
    }
    ++nReader_;
  }
  void unlockReader() {
    unique_lock<mutex> lock(m_);
    assert(nWriter_ == 0);
    assert(nReader_ > 0);
    if (--nReader_ == 0) {
      // writers are waiting for this condition
      cvWriter_.notify_one();
    }
  }
  void lockWriter() {
    unique_lock<mutex> lock(m_);
    ++nWWaiter_;
    while (nWriter_ > 0 || nReader_ > 0) {
      cvWriter_.wait(lock);
    }
    ++nWriter_;

    // We could have decremented nWWaiter_ at this point; but doing so would not
    // readers making progress, as they would still be blocked by this writer.
    // So let's delay its execution.
  }
  void unlockWriter() {
    unique_lock<mutex> lock(m_);
    assert(nWriter_ == 1);
    --nWriter_;
    --nWWaiter_;
    if (nWWaiter_ > 0) {
      // notify a reader would not help
      // but there is a writer we could notify
      cvWriter_.notify_one();
    } else {
      // no writer waiting currently, so notify all readers (if any).
      // note after this point it's still possible for a writer to claim
      // `m_` and preempt the notified readers, before the readers can claim the
      // lock, but that is fine.
      // All note that we must notify all the waiting readers in the queue
      // as they can all make progress and if we don't notify all of them,
      // they may not be notified in the future at all.
      cvReader_.notify_all();
    }
  }

 private:
  mutex m_;

  // think of them as queues of waiters waiting for a condition to happen
  condition_variable cvReader_;
  condition_variable cvWriter_;

  int nReader_{0};
  int nWriter_{0};
  int nWWaiter_{0};
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
    // delay 0.1 - 100 us
    this_thread::sleep_for(nanoseconds(rand() % 1000 * 100));

    auto now = Clock::now();
    lock_.lockReader(); 
    readerDuration_ += Clock::now() - now;

    assert(nWriter_ == 0);
    ++nReader_;
    work();
    --nReader_;
    lock_.unlockReader();
  }

  void writer() {
    // delay 0.1 - 100 us
    this_thread::sleep_for(nanoseconds(rand() % 1000 * 100));

    auto now = Clock::now();
    lock_.lockWriter();
    writerDuration_ += Clock::now() - now;

    assert(nWriter_ == 0);
    assert(nReader_ == 0);
    ++nWriter_;
    work();
    --nWriter_;
    lock_.unlockWriter();
  }

  void work() {
    int t = rand() % 1000;
    this_thread::sleep_for(nanoseconds(t));
  }

  ReaderWriterLockReaderPreferredStrongMutex lock_;
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
