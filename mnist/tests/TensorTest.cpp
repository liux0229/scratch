#include <gtest/gtest.h>

#include "experimental/rockyliu/mnist/tensor.h"

using namespace std;

TEST(TensorTest, matrix) {
  Tensor a{{{1, 2, 3}, {4, 5, 6}}};
  Tensor b{{{1, 2}, {3, 4}, {5, 6}}};
  Tensor c{{{22, 28}, {49, 64}}};
  Tensor d{{{-1, -2, -3}, {-4, -5, -6}}};

  ASSERT_EQ(c, Matrix{a} * Matrix{b});
  ASSERT_EQ((Tensor{Dims{2, 3}}), Matrix{a} + Matrix{d});
}
