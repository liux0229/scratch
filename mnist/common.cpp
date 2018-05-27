#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

#include <iostream>

#include <folly/futures/Future.h>

#include "common.h"

using namespace std;
using namespace folly;

namespace {
int readInt(istream& in) {
  char x[4];
  in.read(x, 4);
  return static_cast<unsigned char>(x[0]) << 24 |
      static_cast<unsigned char>(x[1]) << 16 |
      static_cast<unsigned char>(x[2]) << 8 | static_cast<unsigned char>(x[3]);
}
} // namespace

// Into [-1, 1]
void Example::normalize() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      image[i][j] /= 255.0 / 2;
      image[i][j] -= 1.0;
    }
  }
}

ExampleReader::ExampleReader(string image, string label)
    : image_(image, std::ios::binary), label_(label, std::ios::binary) {
  int magic = readInt(image_);
  assert(magic == 0x00000803);
  magic = readInt(label_);
  assert(magic == 0x00000801);

  numExample_ = readInt(image_);
  int n = readInt(label_);
  assert(numExample_ == n);
}

Optional<Example> ExampleReader::read() {
  if (numExample_ == cur_) {
    return folly::none;
  }

  Example e;
  for (int i = 0; i < e.rows; i++) {
    for (int j = 0; j < e.cols; j++) {
      e.image[i][j] = image_.get();
    }
  }
  e.normalize();
  e.label = label_.get();

  ++cur_;

  return e;
}

ExampleList ExampleReader::readAll() {
  ExampleList ret;
  while (auto e = read()) {
    ret.push_back(e.value());
  }
  return ret;
}

void printStackTrace() {
  void* array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  backtrace_symbols_fd(array, size, STDERR_FILENO);
}

TaskRunner& TaskRunner::get() {
  static TaskRunner runner;
  return runner;
}

TaskRunner::TaskRunner() {
  auto queue = std::make_unique<folly::LifoSemMPMCQueue<
      folly::CPUThreadPoolExecutor::CPUTask,
      folly::QueueBehaviorIfFull::BLOCK>>(nThreads() * 10);
  executor_ =
      std::make_unique<folly::CPUThreadPoolExecutor>(nThreads(), move(queue));
}

void TaskRunner::runAsync(const TaskRunner::Task& task) {
  folly::via(executor_.get(), task);
}

void TaskRunner::run(const vector<TaskRunner::Task>& tasks) {
  vector<folly::Future<folly::Unit>> futures;
  futures.reserve(tasks.size());

  for (auto& t : tasks) {
    futures.push_back(folly::via(executor_.get(), t));
  }

  collectAll(futures).wait();
}
