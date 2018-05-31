#include "tensor.h"

#include <cmath>
#include <cstring>
#include "common.h"

using namespace std;

void UniformInitScheme::init(Tensor& t) {
  uniform_real_distribution<> dist(a_, b_);
  for (auto& x : t.data()) {
    x = dist(gen());
  }
}

Tensor::Tensor(Dims dims, InitScheme&& scheme) : dims_(dims) {
  int n = 1;
  for (auto d : dims_) {
    n *= d;
  }
  data_ = vector<Float>(n);
  scheme.init(*this);
}

Tensor::Tensor(const vector<vector<Float>>& v) {
  dims_ = Dims{static_cast<int>(v.size()), static_cast<int>(v[0].size())};
  data_.reserve(dims_[0] * dims_[1]);
  for (auto& row : v) {
    for (auto c : row) {
      data_.push_back(c);
    }
  }
}

// Produce a two dimensional tensor for now
Tensor::Tensor(const ExampleList& es, bool label) {
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
  data_.reserve(es.size() * n);

  for (auto& e : es) {
    for (auto& row : e.image) {
      for (auto c : row) {
        data_.push_back(c);
      }
    }
  }
}

void Tensor::loadLabel(const ExampleList& es) {
  dims_ = Dims{static_cast<Dim>(es.size())};
  data_.reserve(es.size());

  for (auto& e : es) {
    data_.push_back(static_cast<double>(e.label));
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

  Tensor ret{Dims{dims_.begin() + 1, dims_.end()}};
  copy(
      data_.begin() + x * ret.total(),
      data_.begin() + (x + 1) * ret.total(),
      ret.data_.begin());
  return ret;
}

Float Tensor::l2Norm() const {
  return sqrt(l2Sum());
}

Float Tensor::l2Sum() const {
  Float squaredSum = 0;
  for (auto x : data_) {
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
    for (auto x : tensor.data_) {
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

Tensor Tensor::read(std::istream& in) {
  expectToken(in, "{");

  Dims dims;
  in >> dims;
  Tensor ret{dims};

  std::vector<Float> x;
  in >> x;

  SCHECK(ret.data().size() == x.size());
  ret.data() = x;

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
