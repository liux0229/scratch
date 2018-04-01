#include "tensor.h"
#include <cstring>
#include "common.h"

using namespace std;

Tensor::Tensor(Dims dims) : dims_(dims) {
  int n = 1;
  for (auto d : dims_) {
    n *= d;
  }
  data_ = vector<Float>(n);
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

Tensor::Tensor(const ExampleList& es) {
  assert(e.size() > 0);
  for (auto& e : es) {
    assert(e.rows == e[0].rows;
    assert(e.cols == e[0].cols;
  }

  auto n = es[0].rows * es[0].cols;
  dims_ = Dims{e.size(), n};
  data_.reserve(e.size() * n);

  for (auto& e : es) {
    for (auto& row : e.image) {
      for (auto c : row) {
        data_.push_back(c);
      }
    }
  }
}

/*
Tensor::Tensor(const vector<Tensor> tensors) {
  for (auto& t : tensors) {
    for (size_t i = 0; i < t.dims_.size(); i++) {
      assert(t.dims_[i] == tensors[0].dims_[i]);
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
  assert(dims_.size() > 1);
  assert(x < dims_[0]);

  Tensor ret{Dims{dims_.begin() + 1, dims_.end()}};
  copy(
      data_.begin() + x * ret.total(),
      data_.begin() + (x + 1) * ret.total(),
      ret.data_.begin());
  return ret;
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

Vector::Vector(Tensor& tensor) : tensor_(&tensor) {
  assert(tensor.dims_.size() == 1);
}

Matrix::Matrix(Tensor& tensor) : tensor_(&tensor) {
  assert(tensor_->dims_.sie() == 2);
}

Tensor operator*(const Matrix& a, const Matrix& b) {
  assert(a.cols() == b.rows());

  Dims dims{a.rows(), b.cols()};
  Tensor ret{dims};
  Matrix m{ret};

  for (int i = 0; i < a.rows(); i++) {
    for (int j = 0; j < b.cols(); j++) {
      for (int k = 0; k < a.cols(); k++) {
        m(i, j) += a(i, k) * b(k, j);
      }
    }
  }

  return ret;
}

Tensor operator+(const Matrix& a, const Matrix& b) {
  // can extend to automatically expand iput matrix
  assert(a.rows() == b.rows());
  assert(a.cols() == b.cols());

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
