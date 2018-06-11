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
  explicit Dims() {}

  explicit Dims(std::initializer_list<Dim> dims) : std::vector<Dim>(dims) {
    computeDimSize();
  }

  template <class InputIt>
  Dims(InputIt first, InputIt last) : std::vector<Dim>(first, last) {
    computeDimSize();
  }

  void computeDimSize() {
    dimSize = 1;
    for (auto x : *this) {
      dimSize *= x;
    }
  }

  Dim dimSize = 0;
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
  virtual void read(std::istream& in) {}
  virtual void write(std::ostream& out) const {}
};
using IModel = std::shared_ptr<Model>;

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

  int nThreads() const {
    return nThreads_;
  }

 private:
  TaskRunner();

  const int nThreads_ = 32;
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

template <typename T, std::size_t N>
std::ostream& operator<<(std::ostream& out, const std::array<T, N>& v) {
  out << "[ ";
  for (auto& x : v) {
    out << x << " ";
  }
  out << "]";

  return out;
}
