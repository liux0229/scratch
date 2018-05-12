#include "operators.h"

#include <folly/Format.h>
#include <cmath>

using namespace std;

IBackPropOperator Operator::getBackPropOperator() {
  if (backPropOp_) {
    return backPropOp_;
  }
  backPropOp_ = make_shared<BackPropOperator>(
      name() + "_grad",
      [this](BackPropOperator* op) { return gradientFunc(op); });
  return backPropOp_;
}

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

void FCLayerOperator::applyGradient(const Gradient& g) {
  SCHECK(g.size() == 2);

  if (diagnostics()) {
    cout << folly::format(
        "W gradient ratio: {}%; B gradient ratio: {}%\n",
        g[0].norm() / w_.norm() * 100,
        g[1].norm() / b_.norm() * 100);
    setDiagnostics(false);
  }

  w_ += g[0];
  b_ += g[1];
}

GradientPair FCLayerOperator::gradientFunc(BackPropOperator* op) {
  // input gradient = parent gradient * W^T
  auto& parents = op->parents();
  // Can make this more generic by iterating over the parents
  SCHECK(parents.size() == 1);
  Matrix parentGradient{parents[0].op->inputGradient()[parents[0].inputIndex]};
  auto inputGradient = parentGradient * Matrix{w_}.transpose();

  // w'(i, j) = x(i) * h'(j) and average over all examples
  // w' = X^T * h'
  Matrix x{inputs_[0]->get()};
  // This is also the reason why we cannot fuse over all the per example
  // gradients for the parent, because we need a per example fuse with the input
  auto wGradient = x.transpose() * parentGradient;

  // b' = h' sum over rows
  auto bGradient = parentGradient.rowSum();

  return make_pair(
      Gradient{move(inputGradient)},
      Gradient{move(wGradient), move(bGradient)});
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

GradientPair ReluOperator::gradientFunc(BackPropOperator* op) {
  auto& parents = op->parents();
  // I can make this more generic (the ReLu output is consumed by multiple
  // operators), but let's simplify for now
  SCHECK(parents.size() == 1);

  auto g = parents[0].op->inputGradient()[parents[0].inputIndex];
  auto& output = get();
  SCHECK(g.dims() == output.dims());
  for (size_t i = 0; i < g.data().size(); ++i) {
    if (output.data()[i] <= 0) {
      g.data()[i] = 0;
    }
  }

  return make_pair(Gradient{move(g)}, Gradient{});
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
      const Float maxi = 1e30f;
      if (-in(i, j) > log(maxi)) {
        e[j] = maxi;
      } else {
        e[j] = exp(-in(i, j));
      }
      // e[j] = min(exp(-in(i, j)), maxi);

      sum += e[j];
    }
    for (size_t j = 0; j < e.size(); j++) {
      m(i, j) = e[j] / sum;
      // if (m(i, j) < 1e-40) {
      //   cout << folly::format(
      //               "i={} j={} in={} e={} sum={}", i, j, in(i, j), e[j], sum)
      //        << endl;
      // }
    }
  }
  return get();
}

GradientPair SoftmaxOperator::gradientFunc(BackPropOperator* op) {
  auto& parents = op->parents();
  SCHECK(parents.size() == 1);
  auto& parentG = parents[0].op->inputGradient()[parents[0].inputIndex];
  Matrix parentM{parentG};

  Tensor g{inputs_[0]->get().dims()};
  Matrix m{g};
  Matrix out{get()};

  SCHECK(make_pair(m.rows(), m.cols()) == make_pair(out.rows(), out.cols()));
  SCHECK(
      make_pair(parentM.rows(), parentM.cols()) ==
      make_pair(m.rows(), m.cols()));

  for (int i = 0; i < m.rows(); ++i) {
    int label = -1;
    for (int j = 0; j < m.cols(); ++j) {
      if (parentM(i, j) != 0) {
        // SCHECK(label == -1);
        label = j;
        break;
      }
    }
    SCHECK(label != -1);

    for (int j = 0; j < m.cols(); ++j) {
      if (j == label) {
        m(i, j) = out(i, j) * out(i, j) - out(i, j);
      } else {
        m(i, j) = out(i, label) * out(i, j);
      }
      // double tmp = m(i, j);
      m(i, j) *= parentM(i, label);
      // if (isnan(m(i, j))) {
      //   cout << folly::format(
      //               "m={} parent={} i={} j={} label={}",
      //               tmp,
      //               parentM(i, label),
      //               i,
      //               j,
      //               label)
      //        << endl;
      //   exit(0);
      // }
    }
  }

  return make_pair(Gradient{move(g)}, Gradient{});
}

LossOperator::LossOperator(IOperator input, IOperator label)
    : Operator({}, {input, label}) {
  SCHECK(input->dims().size() == 1);
  SCHECK(label->dims().size() == 0);
}

Tensor& LossOperator::compute() {
  SCHECK(inputs_[0]->get().dims()[0] == inputs_[1]->get().dims()[0]);

  Matrix in{inputs_[0]->get()};
  Vector label{inputs_[1]->get()};

  SCHECK(in.rows() == label.n());

  Float s = 0;
  for (int i = 0; i < label.n(); i++) {
    auto x = label(i);
    SCHECK(x < in.cols());

    Float y = max(in(i, x), static_cast<Float>(1e-30f));

    s += -log(y) / label.n();
  }

  Tensor ret{Dims{1}};
  Vector{ret}(0) = s;
  output_ = ret;

  return get();
}

GradientPair LossOperator::gradientFunc(BackPropOperator* op) {
  Tensor g{inputs_[0]->get().dims(), Tensor::InitScheme::Zero};
  Matrix m{g};
  Matrix in{inputs_[0]->get()};
  Vector label{inputs_[1]->get()};
  for (int i = 0; i < m.rows(); ++i) {
    SCHECK(label(i) < m.cols());
    // Note here we are already applying the 1/m scaling operation needed to
    // compute the averaged loss (same pattern as the forward pass)

    m(i, label(i)) = -1 / in(i, label(i)) / m.rows();
    // if (isinf(m(i, label(i)))) {
    //   cout << folly::format("in={};i={};label={}", in(i, label(i)), i,
    //   label(i))
    //        << endl;
    // }
  }

  return make_pair(Gradient{move(g)}, Gradient{});
}

SoftmaxLossOperator::SoftmaxLossOperator(
    ISoftmaxOperator softmaxOp,
    ILossOperator lossOp)
    : Operator(
          lossOp->dims(),
          softmaxOp->getInputs() + lossOp->getInputs() -
              OperatorList{softmaxOp}),
      softmaxOp_(softmaxOp),
      lossOp_(lossOp) {}

Tensor& SoftmaxLossOperator::compute() {
  softmaxOp_->compute();
  lossOp_->compute();
  return get();
}

GradientPair SoftmaxLossOperator::gradientFunc(BackPropOperator* op) {
  SCHECK(op->parents().empty());

  Tensor g{softmaxOp_->getInputs()[0]->get().dims()};
  Matrix m{g};
  Matrix softmax{softmaxOp_->get()};
  Vector label{lossOp_->getInputs()[1]->get()};

  SCHECK(
      make_pair(m.rows(), m.cols()) ==
      make_pair(softmax.rows(), softmax.cols()));

  for (int i = 0; i < m.rows(); ++i) {
    SCHECK(label(i) < m.cols());

    for (int j = 0; j < m.cols(); ++j) {
      if (j == label(i)) {
        m(i, j) = (1.0 - softmax(i, j)) / m.rows();
      } else {
        m(i, j) = -softmax(i, j) / m.rows();
      }
    }
  }

  return make_pair(Gradient{move(g)}, Gradient{});
}
