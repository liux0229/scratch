#include "operators.h"

#include <cmath>

using namespace std;

FCLayerOperator::FCLayerOperator(int width, IOperator input)
    : Operator({input->dims()[0], width}, {input}),
      w_(Dims{input->dims()[1], width}),
      b_(Dims{input->dims()[0], width}) {
  assert(input->dims().size() == 2);
}

Tensor& FCLayerOperator::compute() {
  Matrix w{w_};
  Matrix b{b_};
  Matrix x{inputs_[0]->get()};

  // Allow rvalue conversion to Matrix
  auto product = x * w;
  output_ = Matrix{product} + b;
  return get();
}

ReluOperator::ReluOperator(IOperator input) : Operator(input->dims(), {input}) {
  assert(input->dims().size() == 2);
}

Tensor& ReluOperator::compute() {
  output_ = inputs_[0]->get();
  Vector v{get()};
  for (int i = 0; i < v.n(); i++) {
    v(i) = v(i) > 0 ? v(i) : 0;
  }
  return get();
}

SoftmaxOperator::SoftmaxOperator(IOperator input)
    : Operator(input->dims(), {input}) {
  assert(input->dims().size() == 2);
}

Tensor& SoftmaxOperator::compute() {
  output_ = Tensor{dims_};
  Matrix m{get()};

  Matrix in{inputs_[0]->get()};

  for (int i = 0; i < m.rows(); i++) {
    // It would be good if I have a row view
    vector<Float> e(in.cols());
    Float sum = 0;
    for (size_t j = 0; j < e.size(); j++) {
      e[j] = exp(-in(i, j));
      sum += e[j];
    }
    for (size_t j = 0; j < e.size(); j++) {
      m(i, j) = e[j] / sum;
    }
  }
  return get();
}

LossOperator::LossOperator(IInputOperator input, IOperator label)
    : Operator({input->dims()[0]}, {input, label}) {
  assert(input->dims().size() == 2);
  assert(label->dims().size() == 1);
  assert(input->dims()[0] == label->dims()[0]);
}

Tensor& LossOperator::compute() {
  output_ = Tensor{dims_};
  Vector v{get()};

  Matrix in{inputs_[0]->get()};
  Vector label{inputs_[1]->get()};

  for (int i = 0; i < v.n(); i++) {
    auto x = label(i);
    assert(x < in.cols());
    v(i) = -log(in(i, x));
  }
  return get();
}
