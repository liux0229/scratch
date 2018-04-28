#pragma once

#include <folly/Optional.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <folly/executors/CPUThreadPoolExecutor.h>

const int N_IMAGE = 28;
const int N_CLASS = 10;
using Float = float;

using Dim = int;
using Dims = std::vector<Dim>;

struct Example {
  void normalize();

  const int rows = N_IMAGE;
  const int cols = N_IMAGE;
  std::array<std::array<Float, N_IMAGE>, N_IMAGE> image;
  int label;
};

using ExampleList = std::vector<Example>;

class ExampleReader {
 public:
  ExampleReader(std::string image, std::string label);
  folly::Optional<Example> read();
  ExampleList readAll();

 private:
  std::ifstream image_;
  std::ifstream label_;
  int numExample_;
  int cur_{0};
};

struct Prediction {
  std::array<float, N_CLASS> prob;
  int getClass() const {
    return std::max_element(prob.begin(), prob.end()) - prob.begin();
  }
};

class Model {
 public:
  virtual ~Model() {}
  virtual Prediction predict(const Example& e) const = 0;
};
using IModel = std::shared_ptr<Model>;

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v) {
  out << "[";
  for (auto& x : v) {
    out << x << " ";
  }
  out << "]";

  return out;
}

void printStackTrace();

#define SCHECK(f)      \
  if (!(f)) {          \
    printStackTrace(); \
    assert(f);         \
  }

// class Exception : public std::exception {
// public:
//   std::string what() const override {
//
//   }
// };

// _tmp ? _tmp : throw std::exception(__FILE__, __LINE__, __FUNCTION__, #e);

// #define ENFORCE(e, ...)                   \
//   ({                                      \
//     auto const& _tmp = (e);               \
//     _tmp ? _tmp : throw std::exception(); \
//   })

template <typename T>
struct reversion_wrapper {
  T& iterable;
};

template <typename T>
auto begin(reversion_wrapper<T> w) {
  return std::rbegin(w.iterable);
}

template <typename T>
auto end(reversion_wrapper<T> w) {
  return std::rend(w.iterable);
}

template <typename T>
reversion_wrapper<T> reverse(T&& iterable) {
  return {iterable};
}

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b) {
  std::vector<T> ret;
  ret.reserve(a.size() + b.size());
  for (auto& x : a) {
    ret.push_back(x);
  }
  for (auto& x : b) {
    ret.push_back(x);
  }
  return ret;
}

template <typename T>
std::vector<T> operator-(const std::vector<T>& a, const std::vector<T>& b) {
  std::vector<T> ret;
  for (auto& x : a) {
    if (find(b.begin(), b.end(), x) == b.end()) {
      ret.push_back(x);
    }
  }
  return ret;
}

class TaskRunner {
 public:
  using Task = std::function<void()>;

  static TaskRunner& get();
  void run(const std::vector<Task>& tasks);

 private:
  TaskRunner();

  std::unique_ptr<folly::Executor> executor_;
};
