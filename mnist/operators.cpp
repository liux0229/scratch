#include "operators.h"

#include <cmath>

using namespace std;

Gradient Operator::computeGradientDebug(const std::function<double()>& loss) {
  const double eps = 1e-3;
  Gradient gs;

  auto f = getParameters();
  while (auto* w = f()) {
    Tensor g{w->dims()};
    for (size_t i = 0; i < w->data().size(); ++i) {
      auto cur = w->data()[i];

      w->data()[i] = cur + eps;
      // cout << "cur=" << w->data()[i] << endl;
      auto loss1 = loss();

      w->data()[i] = cur - eps;
      // cout << "cur=" << w->data()[i] << endl;
      auto loss2 = loss();

      g.data()[i] = (loss1 - loss2) / (2 * eps);
      // cout << "i=" << i << " " << cur << " " << loss1 << " " << loss2
      //      << " diff: " << (loss1 - loss2) << " g=" << g.data()[i] << endl;
      w->data()[i] = cur;
    }
    gs.push_back(g);
  }

  return gs;
}

// This requires knowing the dimension of the input before building the graph
// which needs to be changed (because # rows can be changed)
FCLayerOperator::FCLayerOperator(int width, IOperator input)
    : Operator({width}, {input}),
      w_(Dims{input->dims()[0], width}, Tensor::InitScheme::UniformRandom),
      b_(Dims{width}, Tensor::InitScheme::UniformRandom) {
  SCHECK(input->dims().size() == 1);
}

Tensor& FCLayerOperator::compute() {
  Matrix w{w_};
  Vector b{b_};
  Matrix x{inputs_[0]->get()};
  // cout << "x: " << inputs_[0]->get() << endl;
  // cout << "FC " << w_.dims() << " " << inputs_[0]->get().dims() << endl;
  // cout << "FC w(3, 0): " << w(3, 0) << " " << "x(0, 3): " << x(0, 3) << endl;

  // Allow rvalue conversion to Matrix
  auto product = x * w;
  output_ = Matrix{product} + b;
  // cout << "FC output: " << get() << endl;
  return get();
}

std::function<Tensor*()> FCLayerOperator::getParameters() {
  int state = 0;
  return [this, state]() mutable -> Tensor* {
    auto s = state++;
    switch (s) {
      case 0:
        return &w_;
      case 1:
        return &b_;
      default:
        return nullptr;
    }
  };
}

ReluOperator::ReluOperator(IOperator input) : Operator(input->dims(), {input}) {
  SCHECK(input->dims().size() == 1);
}

Tensor& ReluOperator::compute() {
  output_ = inputs_[0]->get();
  Matrix m{get()};
  for (int i = 0; i < m.rows(); ++i) {
    for (int j = 0; j < m.cols(); ++j) {
      m(i, j) = m(i, j) > 0 ? m(i, j) : 0;
    }
  }
  return get();
}

SoftmaxOperator::SoftmaxOperator(IOperator input)
    : Operator(input->dims(), {input}) {
  SCHECK(input->dims().size() == 1);
}

Tensor& SoftmaxOperator::compute() {
  output_ = Tensor{inputs_[0]->get().dims()};
  // cout << get().dims() << endl;
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

LossOperator::LossOperator(IOperator input, IOperator label)
    : Operator({}, {input, label}) {
  SCHECK(input->dims().size() == 1);
  SCHECK(label->dims().size() == 0);
}

Tensor& LossOperator::compute() {
  SCHECK(inputs_[0]->get().dims()[0] == inputs_[1]->get().dims()[0]);

  Tensor loss{Dims{inputs_[0]->get().dims()[0]}};
  Vector v{loss};

  Matrix in{inputs_[0]->get()};
  Vector label{inputs_[1]->get()};

  SCHECK(in.rows() == label.n());

  for (int i = 0; i < v.n(); i++) {
    auto x = label(i);
    SCHECK(x < in.cols());
    v(i) = -log(in(i, x));
  }

  double s = 0;
  for (int i = 0; i < v.n(); i++) {
    s += v(i);
  }
  s /= v.n();
  output_ = Tensor{Dims{1}};
  Vector{get()}(0) = s;

  return get();
}
