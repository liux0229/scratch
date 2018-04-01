#include "operators.h"

#include <cmath>

FCLayerOperator::FCLayerOperator(int width, IOperator input)
    : dims_{input->dims()[0], width},
      inputs_{input},
      w_(Dims{width, input->dims()[0]}),
      b_(input->dims()[0], Dims{width}) {
  assert(input->dims().size() == 2);
}

const Tensor& FCLayerOperator::compute() {
  Matrix w{w_};
  Matrix b{b_};
  Matrix x{inputs_[0].get()};
  output_ = x * w + b;
  return get();
}

ReluOperator::ReluOperator(IOperator input)
    : dims_(input->dims()), inputs_{input} {
  assert(input->dims().size() == 2);
}

const Tensor& ReluOperator::compute() {
  output_ = inputs_[0].get();
  Vector v{output_};
  for (int i = 0; i < v.n(); i++) {
    v(i) = v(i) > 0 : v(i) : 0;
  }
  return get();
}

SoftmaxOperator::SoftmaxOperator(IOperator input)
    : dims_(input->dims()), inputs_{input} {
  assert(input->dims().size() == 2);
}

const Tensor& SoftmaxOperator::compute() {
  output_ = Tensor{dims_};
  Matrix m{get()};

  Matrix in{inputs_[0]->get()};

  for (int i = 0; i < m.rows(); i++) {
    // It would be good if I have a row view
    vector<Float> e(in.dims()[1]);
    Float sum = 0;
    for (int j = 0; j < e.size(); j++) {
      e[j] = exp(-in(i, j));
      sum += e[j];
    }
    for (int j = 0; j < e.size(); j++) {
      m(i, j) = e[j] / sum;
    }
  }
  return get();
}

LossOperator::LossOperator(IOperator input, IOperator label)
    : dims_(input->dims()[0]), inputs_{input, label} {
  assert(input->dims().size() == 2);
  assert(label->dims().size() == 1);
  assert(input->dims()[0] == label->dims()[0]);
}

const Tensor& LossOperator::compute() {
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
