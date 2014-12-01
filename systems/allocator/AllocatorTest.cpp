#include <gtest/gtest.h>
#include <vector>
#include <cstdlib>
#include <chrono>

#include "Allocator.h"

using namespace std;
using namespace std::chrono;

namespace {

template<typename Allocator>
void runTest() {
  struct Buffer {
    int* ptr_;
    int length_;
    int value_;
  };

  Allocator alloc;
  auto* p = alloc.allocate(500);
  alloc.free(p);

  vector<Buffer> bufs;
  for (int i = 1; i <= 10240; ++i) {
    auto* buf = static_cast<int*>(alloc.allocate(i * sizeof(int)));
    ASSERT_TRUE(buf);
    ASSERT_TRUE(reinterpret_cast<uint64_t>(buf) % 8 == 0);
    for (int j = 0; j < i; ++j) {
      buf[j] = i;
    }
    bufs.push_back({buf, i, i});

    // 20% probability free a previous buffer
    if (rand() % 5 == 0) {
      auto r = rand() % bufs.size();
      alloc.free(bufs[r].ptr_);
      bufs.erase(bufs.begin() + r);
    }
  }

  cout << "Total buffer left: " << bufs.size() << endl;
  for (size_t i = 0; i < bufs.size(); ++i) {
#ifdef ENABLE_VERIFY
    for (size_t j = i; j < bufs.size(); ++j) {
      auto* p = bufs[j].ptr_;
      auto expected = bufs[j].value_;
      auto length = bufs[j].length_;
      for (int k = 0; k < length; ++k) {
        ASSERT_EQ(expected, p[k]) << "i=" << i << " j=" << j << " k=" << k
                                  << " ptr=" << p + k;
      }
    }
#endif
    alloc.free(bufs[i].ptr_);
  }
}

ostream& operator<<(ostream& out, microseconds d) {
  out << 1.0 * d.count() / 1000 << " ms";
  return out;
}

template<typename Allocator>
class AllocatorProfiler {
  using Clock = high_resolution_clock;
  using Duration = Clock::duration;
 public:
  ~AllocatorProfiler() {
    cout << "alloc: " << to(allocTime_) << " free: " << to(freeTime_)
         << " total: " << to(allocTime_ + freeTime_) << endl;
  }
  void* allocate(size_t size) { 
    auto now = Clock::now();
    auto* ret = alloc_.allocate(size); 
    allocTime_ += Clock::now() - now;
    return ret;
  }
  void free(void* p) { 
    auto now = Clock::now();
    alloc_.free(p); 
    freeTime_ += Clock::now() - now;
  }

 private:
  static microseconds to(Duration duration) {
    return duration_cast<microseconds>(duration);
  }

  Allocator alloc_;
  Duration allocTime_{Duration::zero()};
  Duration freeTime_{Duration::zero()};
};
}

TEST(Allocator, basic) {
  runTest<AllocatorProfiler<Allocator>>();
}

TEST(Allocator, benchmark) {
  class MallocAllocator {
   public:
    void* allocate(size_t size) { return malloc(size); }
    void free(void* ptr) { ::free(ptr); }
  };
  runTest<AllocatorProfiler<MallocAllocator>>();
}
