#include <gtest/gtest.h>

#include "experimental/rockyliu/mnist/tensor.h"

using namespace std;

using V = vector<Float>;
using VV = vector<V>;
using VVV = vector<VV>;
using VVVV = vector<VVV>;

TEST(TensorTest, matrix) {
  auto a = Tensor::from(VV{{1, 2, 3}, {4, 5, 6}});
  auto b = Tensor::from(VV{{1, 2}, {3, 4}, {5, 6}});
  auto c = Tensor::from(VV{{22, 28}, {49, 64}});
  auto d = Tensor::from(VV{{-1, -2, -3}, {-4, -5, -6}});

  ASSERT_EQ(c, Matrix{a} * Matrix{b});
  ASSERT_EQ((Tensor{Dims{2, 3}}), Matrix{a} + Matrix{d});
}

TEST(TensorTest, vector) {
  auto a = Tensor::from({1, 2, 1});
  auto b = Tensor::from({0, -2, 0});
  auto c = Tensor::from({1, 0, 1});

  ASSERT_EQ(c, Vector{a} + Vector{b});
}

TEST(TensorTest, flatten) {
  auto a = Tensor::from(VV{{1, 2, 3}, {4, 5, 6}});
  auto b = Tensor::from({1, 2, 3, 4, 5, 6});

  ASSERT_EQ(b, a.flatten());

  Tensor c = a.flatten();
  Vector{c}(0) = 100;
  // Verify c is sharing data with a
  auto d = Tensor::from(VV{{100, 2, 3}, {4, 5, 6}});
  ASSERT_EQ(d, a);
}

TEST(TensorTest, view) {
  auto a = Tensor::from(VV{{1, 2, 3}, {4, 5, 6}});
  auto b = Tensor::from({4, 5, 6});

  ASSERT_EQ(b, a[1]);

  Tensor c = a[1];
  Vector{c}(0) = 100;
  // // Verify c is sharing data with a
  auto d = Tensor::from(VV{{1, 2, 3}, {100, 5, 6}});
  ASSERT_EQ(d, a);
}

namespace {

Tensor operator*(const Tensor& x, Float y) {
  Tensor ret = x;
  for (auto& e : ret.data()) {
    e *= y;
  }
  return ret;
}

Tensor getMask(int x) {
  Tensor ret{Dims{3, 3}};
  Matrix m{ret};

  Float target = 10000;
  if (x % 3 == 0) {
    target = 1;
  } else if (x % 3 == 1) {
    target = -1;
  } else {
    target = 0.5;
  }

  m(x / 3, x % 3) = target;
  return ret;
}

} // namespace

// TODO: Contain different sizes for R and C
TEST(TensorTest, convolve) {
  Tensor x{Dims{4, 4}};
  {
    int i = 1;
    for (auto& e : x.data()) {
      e = i++;
    }
  }
  x = Tensor::from({x, x * -1, x * 2});
  x = Tensor::from({x, x * 10, x * 100});

  vector<Tensor> v;
  for (int i = 0; i < 3; ++i) {
    vector<Tensor> vv;
    for (int j = 0; j < 3; ++j) {
      vv.push_back(getMask(i * 3 + j));
    }
    v.push_back(Tensor::from(vv));
  }
  auto w = Tensor::from(v);

  auto r = Tensor::from(VVV{
      // O1
      {
          // R1
          {0, 0, 0, 0},
          // R2
          {4, 6, 9, 7},
          // R3
          {11, 18, 21, 15},
          // R4
          {19, 30, 33, 23},
      },
      // O2
      {
          // R1
          {4, 6, 9, 7},
          // R2
          {11, 18, 21, 15},
          // R3
          {19, 30, 33, 23},
          // R4
          {27, 42, 45, 31},
      },
      // O3
      {
          // R1
          {11, 18, 21, 15},
          // R2
          {19, 30, 33, 23},
          // R3
          {27, 42, 45, 31},
          // R4
          {0, 0, 0, 0},
      },
  });
  r = Tensor::from({r, r * 10, r * 100});

  ASSERT_EQ(r, convolve(x, w));
}
