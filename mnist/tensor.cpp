#include "tensor.h"

#include <cmath>
#include <cstring>
#include "common.h"

using namespace std;

atomic<long> nTensors{0};
atomic<long> tensorBytesCreated{0};
atomic<long> tensorBytesTotal{0};

void printTensorStats() {
  cout << "nTensors = " << nTensors << endl;
  cout << "avg Tensor size = " << tensorBytesCreated / nTensors << endl;
  cout << "total Tensor size = " << tensorBytesTotal / 1024.0 / 1024.0 << " M"
       << endl;
}

void UniformInitScheme::init(Tensor& t) {
  uniform_real_distribution<> dist(a_, b_);
  for (auto& x : t.data()) {
    x = dist(gen());
  }
}

void Tensor::createStorage() {
  int n = dimSize(dims_);

// Zero-init is required
#if 1
  data_ = shared_ptr<Float>{new Float[n](), std::default_delete<Float[]>{}};
#else
  ++nTensors;
  auto* p = new Float[n]();
  tensorBytesCreated += n * sizeof(Float);
  tensorBytesTotal += n * sizeof(Float);
  data_ = shared_ptr<Float>{p, [n](Float* x) {
                              --nTensors;
                              tensorBytesTotal -= n * sizeof(Float);
                              delete[] x;
                            }};
#endif
  // data_ = vector<Float>(n);
  // cout << "createStorage: " << dims_ << " " << n << " " << data().size() << endl;
}

Tensor::Tensor(const Tensor& t) : dims_(t.dims_) {
  // A copy is requested. Let's copy content of the Tensor.
  createStorage();
  copy(t.data().begin(), t.data().end(), data().begin());
}

Tensor::Tensor(Tensor&& t) : dims_(t.dims_), data_(std::move(t.data_)) {
  // So we can catch unintended move
  t.data_ = nullptr;
}

Tensor& Tensor::operator=(const Tensor& t) {
  dims_ = t.dims_;
  createStorage();
  copy(t.data().begin(), t.data().end(), data().begin());
  return *this;
}

Tensor& Tensor::operator=(Tensor&& t) {
  dims_ = t.dims_;
  data_ = move(t.data_);
  t.data_ = nullptr;
  return *this;
}

Tensor::Tensor(Dims dims, InitScheme&& scheme) : dims_(dims) {
  createStorage();
  scheme.init(*this);
}

Tensor Tensor::from(const vector<Float>& v) {
  int n = v.size();
  Tensor ret{Dims{n}};
  copy(v.begin(), v.end(), ret.data().begin());
  return ret;
}

Tensor Tensor::from(const vector<Tensor>& v) {
  SCHECK(v.size() > 0);
  for (auto& x : v) {
    SCHECK(x.dims() == v[0].dims());
  }

  vector<Dim> dims = v[0].dims();
  dims.insert(dims.begin(), v.size());

  Tensor ret{Dims{dims.begin(), dims.end()}};
  int i = 0;
  for (auto& x : v) {
    std::copy(
        x.data().begin(),
        x.data().end(),
        ret.data().begin() + i * v[0].dims().dimSize);
    ++i;
  }

  return ret;
}

// Produce a two dimensional tensor for now
Tensor::Tensor(const ExampleRange& es, bool label) {
  SCHECK(es.size() > 0);

  if (label) {
    loadLabel(es);
    return;
  }

  for (auto& e : es) {
    SCHECK(e.rows == es[0].rows);
    SCHECK(e.cols == es[0].cols);
  }

  auto n = es[0].rows * es[0].cols;
  dims_ = Dims{static_cast<Dim>(es.size()), n};
  createStorage();

  int i = 0;
  for (auto& e : es) {
    for (auto& row : e.image) {
      for (auto c : row) {
        data()[i++] = c;
      }
    }
  }
}

void Tensor::loadLabel(const ExampleRange& es) {
  dims_ = Dims{static_cast<Dim>(es.size())};
  createStorage();

  int i = 0;
  for (auto& e : es) {
    data()[i++] = static_cast<double>(e.label);
  }
}

/*
Tensor::Tensor(const vector<Tensor> tensors) {
  for (auto& t : tensors) {
    for (size_t i = 0; i < t.dims_.size(); i++) {
      SCHECK(t.dims_[i] == tensors[0].dims_[i]);
    }
  }

  dims_.push_back(tensors.size());
  for (auto dim : tensors[0].dims_) {
    dims_.push_back(dim);
  }
  data_.reserve(tensors.size() * tensors[0].data_.size());
  size_t i = 0;
  for (auto& t : tensors) {
    copy(t.data_.begin(), t.data_.end(), data_.begin() + i);
  }
}
*/

Tensor Tensor::operator[](Dim x) const {
  SCHECK(dims_.size() > 1);
  SCHECK(x < dims_[0]);

  // Avoid the excessive vector creation?
  Dims dims{dims_.begin() + 1, dims_.end()};

  return Tensor{dims,
                shared_ptr<Float>{data_, data().begin() + x * dimSize(dims)}};

  // Tensor ret{Dims{dims_.begin() + 1, dims_.end()}};
  // copy(
  //     data().begin() + x * ret.total(),
  //     data().begin() + (x + 1) * ret.total(),
  //     ret.data().begin());
  // return ret;
}

Tensor Tensor::flatten() const {
  return Tensor{Dims{dimSize(dims())}, *this};
  // Tensor ret{Dims{dimSize(dims())}};
  // copy(data().begin(), data().end(), ret.data().begin());
  // return ret;
}

Float Tensor::l2Norm() const {
  return sqrt(l2Sum());
}

Float Tensor::l2Sum() const {
  Float squaredSum = 0;
  for (auto x : data()) {
    squaredSum += x * x;
  }
  return squaredSum;
}

bool Tensor::equals(const Tensor& other, double eps) const {
  if (dims() != other.dims()) {
    return false;
  }
  for (int i = 0; i < static_cast<int>(data().size()); ++i) {
    if (abs(data()[i] - other.data()[i]) > eps) {
      return false;
    }
  }
  return true;
}

void print(ostream& out, const Tensor& tensor, string tab) {
  out << tab << "{" << endl;

  if (tensor.dims_.size() == 1) {
    out << tab << "\t";
    for (auto x : tensor.data()) {
      out << x << ", ";
    }
    out << endl;
  } else {
    for (int i = 0; i < tensor.dims_[0]; ++i) {
      print(out, tensor[i], tab + "\t");
    }
  }

  out << tab << "}" << endl;
}

ostream& operator<<(ostream& out, const Tensor& tensor) {
  print(out, tensor, "");
  return out;
}

ostream& operator<<(ostream& out, Tensor::Array v) {
  out << "[ ";
  for (auto x : v) {
    out << x << " ";
  }
  out << "]";

  return out;
}

Tensor Tensor::read(std::istream& in) {
  expectToken(in, "{");

  Dims dims;
  in >> dims;
  Tensor ret{dims};

  std::vector<Float> x;
  in >> x;

  SCHECK(ret.data().size() == x.size());
  copy(x.begin(), x.end(), ret.data().begin());

  expectToken(in, "}");

  return ret;
}

void Tensor::write(std::ostream& out, const Tensor& tensor) {
  out << "{" << endl;

  out << tensor.dims() << endl;
  out << tensor.data() << endl;

  out << "}" << endl;
}

Vector::Vector(Tensor& tensor) : tensor_(&tensor) {
  // cout << tensor.dims_ << endl;
  SCHECK(tensor.dims_.size() == 1);
}

Matrix::Matrix(Tensor& tensor) : tensor_(&tensor) {
  SCHECK(tensor_->dims_.size() == 2);
}

TransposedMatrix Matrix::transpose() {
  return TransposedMatrix{this};
}

Tensor operator+(const Matrix& a, const Matrix& b) {
  SCHECK(a.rows() == b.rows());
  SCHECK(a.cols() == b.cols());

  Dims dims{a.rows(), a.cols()};
  Tensor ret{dims};
  Matrix m{ret};

  for (int i = 0; i < a.rows(); i++) {
    for (int j = 0; j < a.cols(); j++) {
      m(i, j) = a(i, j) + b(i, j);
    }
  }

  return ret;
}

Tensor operator+(const Matrix& a, const Vector& b) {
  SCHECK(a.cols() == b.n());

  Dims dims{a.rows(), a.cols()};
  Tensor ret{dims};
  Matrix m{ret};

  for (int i = 0; i < a.rows(); i++) {
    for (int j = 0; j < a.cols(); j++) {
      m(i, j) = a(i, j) + b(j);
    }
  }

  return ret;
}

Tensor operator+(const Vector& a, const Vector& b) {
  Tensor ret{Dims{a.n()}};
  Vector r{ret};

  SCHECK(a.n() == b.n());
  for (int i = 0; i < a.n(); ++i) {
    r(i) = a(i) + b(i);
  }
  return ret;
}

Vector& operator+=(Vector& a, const Vector& b) {
  SCHECK(a.n() == b.n());
  for (int i = 0; i < a.n(); ++i) {
    a(i) += b(i);
  }
  return a;
}

Vector& operator+=(Vector& a, Float x) {
  for (int i = 0; i < a.n(); ++i) {
    a(i) += x;
  }
  return a;
}

Tensor Matrix::rowSum() const {
  Tensor ret{Dims{cols()}};
  Vector v{ret};
  for (int j = 0; j < cols(); ++j) {
    for (int i = 0; i < rows(); ++i) {
      v(j) += (*this)(i, j);
    }
  }
  return ret;
}

Tensor convolve(const Tensor& x, const Tensor& w) {
  // x: {batch, input channel, row, column}
  // w: {output channel, input channel, row, column}

  SCHECK(x.dims().size() == 4);
  SCHECK(w.dims().size() == 4);

  Tensor ret{Dims{x.dims()[0], w.dims()[0], x.dims()[2], x.dims()[3]}};
  const Dim R = w.dims()[2];
  const Dim C = w.dims()[3];

  for (Dim e = 0; e < ret.dims()[0]; ++e) {
    auto example = ret[e];
    auto xe = x[e];

    for (Dim c = 0; c < ret.dims()[1]; ++c) {
      auto channel = example[c];
      Matrix out{channel};
      auto wc = w[c];

      for (Dim k = 0; k < x.dims()[1]; ++k) {
        // cout << "input dim: " << k << endl;
        auto xk = xe[k];
        Matrix xm{xk};

        auto wck = wc[k];
        Matrix wm{wck};

        for (Dim i = 0; i < out.rows(); ++i) {
          for (Dim j = 0; j < out.cols(); ++j) {
            out(i, j) +=
                dot(MatrixPatch(xm, i - R / 2, j - C / 2, R, C),
                    MatrixPatch(wm, 0, 0, R, C));
          }
        }
      }
    }
  }

  return ret;
}
