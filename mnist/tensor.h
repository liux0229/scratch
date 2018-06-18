#include "common.h"

#include <folly/futures/Promise.h>
#include <atomic>
#include <limits>
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
  // A view object
  struct Array {
    // Array(Float* d, int n, std::vector<Float>* v) : d_(d), n_(n), v_(v) {}
    Array(Float* d, int n) : d_(d), n_(n) {}

    Float& operator[](size_t x) {
      return d_[x];
      // return (*v_)[x];
    }
    // Float operator[](size_t x) const {
    //   return d_[x];
    // }
    Float* begin() {
      return d_;
      // return &(*v_)[0];
    }
    Float* end() {
      // return &(*v_)[0] + v_->size();
      return d_ + n_;
    }
    // const Float* begin() const {
    //   return d_;
    // }
    // const Float* end() const {
    //   return d_ + n_;
    // }
    size_t size() {
      // return v_->size();
      return n_;
    }

   private:
    Float* d_;
    size_t n_;
    // std::vector<Float>* v_;
  };

  static Tensor from(const std::vector<Float>& v);

  template <typename T>
  static Tensor from(const std::vector<T>& v) {
    std::vector<Tensor> vt;
    for (auto& x : v) {
      vt.push_back(Tensor::from(x));
    }
    return from(vt);
  }

  static Tensor from(const std::vector<Tensor>& v);

  Tensor() : dims_({}), data_() {}
  Tensor(Dims dims, InitScheme&& scheme = ZeroInitScheme{});
  Tensor(const ExampleRange& es, bool label);

  // Adapts the Tensor to a different shape
  Tensor(Dims dims, const Tensor& tensor) : dims_(dims), data_(tensor.data_) {
    // std::cout << dims << " " << tensor.data().size() << std::endl;
    SCHECK(dimSize(dims) == tensor.data().size());
  }

  Tensor(const Tensor&);
  Tensor(Tensor&&);
  Tensor& operator=(const Tensor&);
  Tensor& operator=(Tensor&&);

  Dim total() const {
    return data().size();
  }
  Dims dims() const {
    return dims_;
  }

  // TODO: actually respects the constness
  // TODO: dimSize() seems slow; need to change Dims to a real object so we can
  // cache the answer But we should profile it
  Array data() const {
    return Array(data_.get(), dimSize(dims_));
    // return Array(
    //     const_cast<Float*>(&data_[0]),
    //     dimSize(dims_));
    // SCHECK(dimSize(dims_) == data_.size());
    // return Array(
    //     const_cast<Float*>(&data_[0]),
    //     data_.size());
  }

  // const std::vector<Float>& data() const { return data_; }
  // std::vector<Float>& data() { return data_; }

  Tensor operator[](Dim x) const;
  Tensor flatten() const;

  Float l2Norm() const;
  Float l2Sum() const;

  bool operator==(const Tensor& other) const {
    if (dims_ != other.dims_) {
      return false;
    }
    if (!std::equal(data().begin(), data().end(), other.data().begin())) {
      return false;
    }
    return true;
  }

  bool equals(const Tensor& other, double eps) const;

  static Tensor read(std::istream& in);
  static void write(std::ostream& out, const Tensor& tensor);

  friend void print(std::ostream& out, const Tensor& tensor, std::string tab);

 private:
  Tensor(Dims dims, std::shared_ptr<Float> data) : dims_(dims), data_(data) {}

  void createStorage();
  void loadLabel(const ExampleRange& es);

  friend class Vector;
  friend class Matrix;

  Dims dims_;
  std::shared_ptr<Float> data_;
  // std::vector<Float> data_;
};

std::ostream& operator<<(std::ostream&, const Tensor& tensor);

std::ostream& operator<<(std::ostream&, Tensor::Array x);

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
    return tensor_->data()[i];
  }
  Float& operator()(Dim i) {
    return tensor_->data()[i];
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
    return tensor_->data()[i * cols() + j];
  }
  Float& operator()(Dim i, Dim j) {
    return tensor_->data()[i * cols() + j];
  }

  Tensor rowSum() const;
  Float sum() const {
    Float ret = 0;
    for (auto x : tensor_->data()) {
      ret += x;
    }
    return ret;
  }

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

class MatrixPatch {
 public:
  MatrixPatch(
      Matrix& m,
      Dim ro,
      Dim co,
      Dim rows,
      Dim cols,
      bool reverse = false)
      : m_(&m),
        ro_(ro),
        co_(co),
        rows_(rows),
        cols_(cols),
        direction_(reverse ? -1 : 1) {}
  Dim rows() const {
    return rows_;
  }
  Dim cols() const {
    return cols_;
  }

  // TODO: make the range checks optional
  Float operator()(Dim i, Dim j) const {
    SCHECK(i >= 0 && i < rows_ && j >= 0 && j < cols_);

    auto r = targetR(i);
    auto c = targetC(j);
    if (r < 0 || r >= m_->rows() || c < 0 || c >= m_->cols()) {
      // TODO: consider returning the minimum of the whole image
      return 0.0;
    }
    return (*m_)(r, c);
  }

  std::tuple<Float, int, int> max() const {
    auto ret = std::numeric_limits<Float>::min();
    int maxI = -1, maxJ = -1;
    for (int i = 0; i < rows(); ++i) {
      for (int j = 0; j < cols(); ++j) {
        auto cur = (*this)(i, j);
        if (cur > ret) {
          ret = cur;
          maxI = i;
          maxJ = j;
        }
      }
    }
    return std::make_tuple(ret, targetR(maxI), targetC(maxJ));
  }

 private:
  int targetR(Dim r) const {
    return ro_ + direction_ * r;
  }

  int targetC(Dim c) const {
    return co_ + direction_ * c;
  }

  Matrix* m_;
  Dim ro_;
  Dim co_;
  Dim rows_;
  Dim cols_;
  int direction_;
};

inline std::ostream& operator<<(std::ostream& out, const MatrixPatch& m) {
  out << "[";
  for (int r = 0; r < m.rows(); ++r) {
    for (int c = 0; c < m.cols(); ++c) {
      out << " " << m(r, c);
    }
    out << ";";
  }
  out << "]";

  return out;
}

// TODO: skip lopping through the areas that have value 0.
inline Float dot(const MatrixPatch& a, const MatrixPatch& b) {
  SCHECK(a.rows() == b.rows() && a.cols() == b.cols());
  // std::cout << "dot " << a << " . " << b << ": ";
  Float s = 0;
  for (Dim i = 0; i < a.rows(); ++i) {
    for (Dim j = 0; j < a.cols(); ++j) {
      if (a(i, j) * b(i, j) != 0) {
        // std::cout << " + " << a(i, j) * b(i, j);
      }
      s += a(i, j) * b(i, j);
    }
  }
  // std::cout << std::endl;
  return s;
}

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
#elif 0
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
      // std::cout << i << "," << j << "=" << m(i, j) << std::endl;
    }
  }
#endif

  // std::cout << ret << std::endl;
  return ret;
}

Tensor operator+(const Matrix& a, const Matrix& b);
// Row wise addition
Tensor operator+(const Matrix& a, const Vector& b);
Tensor operator+(const Vector& a, const Vector& b);
Vector& operator+=(Vector& a, const Vector& b);
Vector& operator+=(Vector& a, Float x);

Tensor convolve(const Tensor& x, const Tensor& w);

using Gradient = std::vector<Tensor>;
using GradientList = std::vector<Gradient>;
using GradientPair = std::pair<Gradient, Gradient>;
