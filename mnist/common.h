#pragma once

#include <folly/Optional.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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

// class Exception : public std::exception {
// public:
//   std::string what() const override {
//
//   }
// };

// _tmp ? _tmp : throw std::exception(__FILE__, __LINE__, __FUNCTION__, #e);

#define ENFORCE(e, ...)                   \
  ({                                      \
    auto const& _tmp = (e);               \
    _tmp ? _tmp : throw std::exception(); \
  })
