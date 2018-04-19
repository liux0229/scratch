#include "common.h"

struct Example;

class Tensor {
 public:
  enum class InitScheme {
    Zero,
    UniformRandom,
  };

  Tensor(const std::vector<std::vector<Float>>& v);
  Tensor(Dims dims, InitScheme scheme = InitScheme::Zero);
  Tensor(const ExampleList& es, bool label);
  // Tensor(const std::vector<Tensor> tensors);

  Dim total() const {
    return data_.size();
  }
  Dims dims() const {
    return dims_;
  }

  // Return the raw data
  std::vector<Float>& data() {
    return data_;
  }
  const std::vector<Float>& data() const {
    return data_;
  }

  // Make this more efficient
  Tensor operator[](Dim x) const;

  bool operator==(const Tensor& other) const {
    return std::tie(dims_, data_) == std::tie(other.dims_, other.data_);
  }

  friend void print(std::ostream& out, const Tensor& tensor, std::string tab);

 private:
  void loadLabel(const ExampleList& es);

  friend class Vector;
  friend class Matrix;

  Dims dims_;
  std::vector<Float> data_;
};

std::ostream& operator<<(std::ostream&, const Tensor& tensor);

inline Tensor& operator+=(Tensor& x, const Tensor& y) {
  SCHECK(x.dims() == y.dims());
  for (size_t k = 0; k < x.data().size(); ++k) {
    x.data()[k] += y.data()[k];
  }
  return x;
}

inline Tensor& operator*=(Tensor& x, double y) {
  for (auto& e : x.data()) {
    e *= y;
  }
  return x;
}

// A view on top of the general tensor
class Vector {
 public:
  Vector(Tensor& tensor);
  Dim n() const {
    return tensor_->dims_[0];
  }
  Float operator()(Dim i) const {
    return tensor_->data_[i];
  }
  Float& operator()(Dim i) {
    return tensor_->data_[i];
  }

 private:
  Tensor* tensor_;
};

// A view on top of the general tensor
// Consider disabling copy
class Matrix {
 public:
  Matrix(Tensor& tensor);
  Dim rows() const {
    return tensor_->dims_[0];
  }
  Dim cols() const {
    return tensor_->dims_[1];
  }
  Float operator()(Dim i, Dim j) const {
    return tensor_->data_[i * cols() + j];
  }
  Float& operator()(Dim i, Dim j) {
    return tensor_->data_[i * cols() + j];
  }

 private:
  Tensor* tensor_;
};

Tensor operator*(const Matrix& a, const Matrix& b);
Tensor operator+(const Matrix& a, const Matrix& b);
Tensor operator+(const Matrix& a, const Vector& b);

using Gradient = std::vector<Tensor>;
using GradientList = std::vector<Gradient>;
using GradientPair = std::pair<Gradient, Gradient>;

Gradient operator*(const Gradient& g, double a);
