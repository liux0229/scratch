#pragma once

#include <folly/Format.h>
#include <folly/Optional.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <folly/executors/CPUThreadPoolExecutor.h>

const int N_IMAGE = 28;
const int N_CLASS = 10;
using Float = double;

using Dim = int;

struct Dims : std::vector<Dim> {
  using Base = std::vector<Dim>;

  explicit Dims() {}

  explicit Dims(std::initializer_list<Dim> dims) : Base(dims) {
    computeDimSize();
  }

  template <class InputIt>
  Dims(InputIt first, InputIt last) : Base(first, last) {
    computeDimSize();
  }

  Dims(std::vector<Dim>&& d) : Base(std::move(d)) {
    computeDimSize();
  }

  void computeDimSize() {
    dimSize = 1;
    for (auto x : *this) {
      dimSize *= x;
    }
  }

  Dims addFront(Dim x) const {
    Base dims{x};
    dims.insert(dims.end(), begin(), end());
    return Dims{std::move(dims)};
  }

  Dim dimSize = 0;

 private:
  using Base::insert;
};

inline int dimSize(const Dims& dims) {
  return dims.dimSize;
}

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

void printStackTrace();

#define SCHECK(f, ...)                                              \
  if (!(f)) {                                                       \
    printStackTrace();                                              \
    std::cout << #f << " failed at " << __FILE__ << ":" << __LINE__ \
              << std::endl;                                         \
    std::abort();                                                   \
  }

#define SCHECK_MSG(f, msg)                                          \
  if (!(f)) {                                                       \
    printStackTrace();                                              \
    std::cout << msg << std::endl;                                  \
    std::cout << #f << " failed at " << __FILE__ << ":" << __LINE__ \
              << std::endl;                                         \
    std::abort();                                                   \
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

template <typename T, typename C>
std::vector<T> operator*(const std::vector<T>& a, C b) {
  auto ret = a;

  for (auto& x : ret) {
    x *= b;
  }

  return ret;
}

template <typename T>
std::vector<T> operator/(const std::vector<T>& a, const std::vector<T>& b) {
  SCHECK(a.size() == b.size());

  std::vector<T> ret;
  for (size_t i = 0; i < a.size(); ++i) {
    ret.push_back(a[i] / b[i]);
  }

  return ret;
}

class TaskRunner {
 public:
  using Task = std::function<void()>;

  static TaskRunner& get();
  void run(const std::vector<Task>& tasks);
  void runAsync(const Task& task);

  static void setThreads(int n) {
    nThreads_ = n;
  }
  static int nThreads() {
    return nThreads_;
  }

 private:
  TaskRunner();

  static int nThreads_;
  std::unique_ptr<folly::Executor> executor_;
};

namespace {
using std::istream;
using std::string;

void expectToken(istream& in, string expected) {
  string token;
  in >> token;
  SCHECK_MSG(
      token == expected,
      folly::format("Expect {}; received {}", expected, token));
}

void expectLine(istream& in, string expected) {
  while (!in.eof() && std::isspace(in.peek())) {
    auto ch = in.get();
    if (ch == '\n') {
      break;
    }
  }

  string token;
  getline(in, token);
  SCHECK_MSG(
      token == expected,
      folly::format("Expect {}; received {}", expected, token));
}

template <typename T>
T expect(istream& in) {
  T x;
  in >> x;
  SCHECK(!in.fail());
  return x;
}

string readString(istream& in) {
  string token;
  in >> token;
  SCHECK(
      token.size() >= 2 && token[0] == '"' && token[token.size() - 1] == '"');
  return token.substr(1, token.size() - 2);
}

template <typename T>
T parse(string s) {
  std::istringstream in(s);
  T ret;
  in >> ret;
  return ret;
}
} // namespace

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v) {
  out << "[ ";
  for (auto& x : v) {
    out << x << " ";
  }
  out << "]";

  return out;
}

template <typename T>
std::istream& operator>>(std::istream& in, std::vector<T>& v) {
  expectToken(in, "[");

  while (true) {
    std::string s;
    in >> s;
    if (s == "]") {
      break;
    }
    auto x = parse<T>(s);
    v.push_back(x);
  }

  return in;
}

inline std::istream& operator>>(std::istream& in, Dims& v) {
  in >> static_cast<Dims::Base&>(v);
  v.computeDimSize();
  return in;
}

template <typename T, std::size_t N>
std::ostream& operator<<(std::ostream& out, const std::array<T, N>& v) {
  out << "[ ";
  for (auto& x : v) {
    out << x << " ";
  }
  out << "]";

  return out;
}

class ExampleRange {
 public:
  ExampleRange(const ExampleList& examples, int start = 0, int size = 0)
      : v_(&examples),
        start_(start),
        size_(size == 0 ? examples.size() : size) {
    SCHECK(start < examples.size());
  }

  class Iterator {
   public:
    Iterator(const ExampleRange& base, int i) : base_(&base), i_(i) {}
    const Example& operator*() const {
      return (*base_)[i_];
    }

    Iterator& operator++() {
      ++i_;
      return *this;
    }

    bool operator==(Iterator b) const {
      return base_ == b.base_ && i_ == b.i_;
    }

    bool operator!=(Iterator b) const {
      return !(*this == b);
    }

   private:
    const ExampleRange* base_;
    int i_;
  };

  Iterator begin() const {
    return Iterator{*this, 0};
  }

  Iterator end() const {
    return Iterator{*this, size_};
  }

  int size() const {
    return size_;
  }

  const Example& operator[](int i) const {
    auto x = (start_ + i) % v_->size();
    return (*v_)[x];
  }

  std::vector<ExampleRange> splitToNBatches(int n) const {
    SCHECK(n >= 1);
    n = std::min(n, size());
    auto b = size() / n;

    std::vector<ExampleRange> ret;
    ret.reserve(n);

    for (int i = 0; i < n - 1; ++i) {
      ret.push_back(ExampleRange(*v_, (start_ + i * b) % v_->size(), b));
    }
    ret.push_back(ExampleRange(
        *v_, (start_ + (n - 1) * b) % v_->size(), size() - (n - 1) * b));
    return ret;
  }

  std::vector<ExampleRange> splitByBatchSize(int batchSize) const {
    SCHECK(batchSize >= 1 && batchSize <= size());
    SCHECK(start_ + size() <= v_->size()); // ensure does not wrap around

    std::vector<ExampleRange> ret;

    int start = 0;
    while (start < size()) {
      auto end = std::min(size(), start + batchSize);
      ret.push_back(ExampleRange(*v_, start_ + start, end - start));
      start = end;
    }

    return ret;
  }

 private:
  const ExampleList* v_;
  int start_;
  int size_;
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
  virtual std::vector<Prediction> predict(const ExampleList& example) const = 0;
  virtual void read(std::istream& in) {}
  virtual void write(std::ostream& out) const {}
};
using IModel = std::shared_ptr<Model>;
