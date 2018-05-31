#include "common.h"

#include <folly/futures/Promise.h>
#include <atomic>
#include <random>

struct Example;

class Tensor;
class InitScheme {
 public:
  InitScheme() {}
  virtual void init(Tensor& t) = 0;
  virtual ~InitScheme() {}

 protected:
  std::mt19937& gen() {
    return gen_;
  }

 private:
  std::random_device rd_;
  std::mt19937 gen_{rd_()};
};

class ZeroInitScheme : public InitScheme {
 public:
  void init(Tensor& t) override {}
};

class UniformInitScheme : public InitScheme {
 public:
  UniformInitScheme(Float a = -1, Float b = 1) : a_(a), b_(b) {}
  void init(Tensor& t) override;

 private:
  Float a_, b_;
};

class Tensor {
 public:
  Tensor(const std::vector<std::vector<Float>>& v);
  Tensor(Dims dims, InitScheme&& scheme = ZeroInitScheme{});
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

  Float l2Norm() const;
  Float l2Sum() const;

  bool operator==(const Tensor& other) const {
    return std::tie(dims_, data_) == std::tie(other.dims_, other.data_);
  }

  bool equals(const Tensor& other, double eps) const;

  static Tensor read(std::istream& in);
  static void write(std::ostream& out, const Tensor& tensor);

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

class TransposedMatrix;
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

  Tensor rowSum() const;

  TransposedMatrix transpose();

 private:
  Tensor* tensor_;
};

class TransposedMatrix {
 public:
  TransposedMatrix(Matrix* m) : m_(m) {}
  Dim rows() const {
    return m_->cols();
  }
  Dim cols() const {
    return m_->rows();
  }
  Float operator()(Dim i, Dim j) const {
    return (*m_)(j, i);
  }
  Float& operator()(Dim i, Dim j) {
    return (*m_)(j, i);
  }

 private:
  Matrix* m_;
};

template <typename T1, typename T2>
constexpr bool is_same_v = std::is_same<T1, T2>::value;
#define REQUIRES(T, T1, T2)                                                  \
  typename = typename std::enable_if < is_same_v<T, T1> || is_same_v<T, T2>, \
  void > ::type

template <
    typename MX1,
    typename MX2,
    REQUIRES(MX1, Matrix, TransposedMatrix),
    REQUIRES(MX2, Matrix, TransposedMatrix)>
Tensor operator*(const MX1& a, const MX2& b) {
  SCHECK(a.cols() == b.rows());

  Dims dims{a.rows(), b.cols()};
  Tensor ret{dims};
  Matrix m{ret};

#if 0
  // Distribute the work across K cores

  const int T = TaskRunner::get().nThreads();
  SCHECK(T >= 1);

  std::vector<TaskRunner::Task> tasks;
  tasks.reserve(T);
  for (int t = 0; t < T; t++) {
    int batch = (a.rows() + T - 1) / T;
    int start = t * batch;
    int end = std::min((t + 1) * batch, a.rows());
    auto compute = [start, end, &a, &b, &m]() {
      for (int i = start; i < end; i++) {
        for (int j = 0; j < b.cols(); j++) {
          for (int k = 0; k < a.cols(); k++) {
            m(i, j) += a(i, k) * b(k, j);
          }
        }
      }
    };
    tasks.push_back(compute);
  }

  TaskRunner::get().run(tasks);
#elif 1
  const int T = TaskRunner::get().nThreads();
  SCHECK(T >= 1);

  std::atomic<int> finished{0};
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();

  for (int t = 0; t < T; t++) {
    int batch = (a.rows() + T - 1) / T;
    int start = t * batch;
    int end = std::min((t + 1) * batch, a.rows());
    auto compute =
        [start, end, t, T, &a, &b, &m, &finished, &promise]() -> void {
      for (int i = start; i < end; i++) {
        for (int j = 0; j < b.cols(); j++) {
          for (int k = 0; k < a.cols(); k++) {
            m(i, j) += a(i, k) * b(k, j);
            // auto target = m(i, j) + a(i, k) * b(k, j);
            // target = std::max(std::min(target, 1e15f), -1e15f);
            // m(i, j) = target;
          }
        }
      }
      if (++finished == T) {
        promise.setValue();
      }
    };
    if (t < T - 1) {
      TaskRunner::get().runAsync(compute);
    } else {
      compute();
    }
  }

  future.wait();
#else
  for (int i = 0; i < a.rows(); i++) {
    for (int j = 0; j < b.cols(); j++) {
      for (int k = 0; k < a.cols(); k++) {
        m(i, j) += a(i, k) * b(k, j);
      }
    }
  }
#endif

  return ret;
}

Tensor operator+(const Matrix& a, const Matrix& b);
Tensor operator+(const Matrix& a, const Vector& b);

using Gradient = std::vector<Tensor>;
using GradientList = std::vector<Gradient>;
using GradientPair = std::pair<Gradient, Gradient>;
