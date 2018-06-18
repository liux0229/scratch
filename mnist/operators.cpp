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
      // if (dynamic_cast<RegularizerOperator*>(this)) {
      //   if (i == 0) {
      //     cout << "i=" << i << " " << cur << " " << loss1 << " " << loss2
      //          << " diff: " << (loss1 - loss2) << " g=" << g.data()[i] <<
      //          endl;
      //   }
      // }

      w->data()[i] = cur;
    }
    gs.push_back(g);
  }

  return gs;
}

// This requires knowing the dimension of the input before building the graph
// which needs to be changed (because # rows can be changed)
FCLayerOperator::FCLayerOperator(int width, IOperator input)
    : Operator(Dims{width}, {input}),
      w_(Dims{input->dims()[0], width},
         UniformInitScheme{-1.0 / (input->dims()[0] + width),
                           1.0 / (input->dims()[0] + width)}),
      b_(Dims{width}, UniformInitScheme{}) {
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
  get() = Matrix{product} + b;
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
        "{} W gradient ratio: {:.2F}({:.2F}%); "
        "B gradient ratio: {:.2F}({:.2F}%)\n",
        name(),
        w_.l2Norm(),
        g[0].l2Norm() / w_.l2Norm() * 100,
        b_.l2Norm(),
        g[1].l2Norm() / b_.l2Norm() * 100);
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

  cout << "input gradient: " << inputGradient << endl;

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

void FCLayerOperator::attachRegularizer(RegularizerOperator& regularizer) {
  regularizer.addParameter(&w_);
}

void FCLayerOperator::read(std::istream& in) {
  Operator::read(in);

  expectToken(in, "W");
  expectToken(in, "=");
  w_ = Tensor::read(in);

  expectToken(in, "B");
  expectToken(in, "=");
  b_ = Tensor::read(in);
}

void FCLayerOperator::write(std::ostream& out) const {
  Operator::write(out);
  out << "W = ";
  Tensor::write(out, w_);
  out << "B = ";
  Tensor::write(out, b_);
}

GradientPair AdapterOperator::gradientFunc(BackPropOperator* op) {
  auto& parents = op->parents();
  SCHECK(parents.size() == 1);

  auto& parentG = parents[0].op->inputGradient()[parents[0].inputIndex];
  Tensor g{inputs_[0]->dims().addFront(parentG.dims()[0]), parentG};
  return make_pair(Gradient{move(g)}, Gradient{});
}

ConvolutionLayerOperator::ConvolutionLayerOperator(
    int channel,
    int width,
    IOperator input)
    : Operator(computeOutputDims(input->dims(), channel, width), {input}),
      w_(computeWDims(input->dims(), channel, width), UniformInitScheme{}),
      b_(Dims{channel}, UniformInitScheme{}) {
  SCHECK(input->dims()[1] >= w_.dims()[2] && input->dims()[2] >= w_.dims()[3]);
}

Tensor& ConvolutionLayerOperator::compute() {
  get() = convolve(inputs_[0]->get(), w_);

  auto& ret = get();
  Vector bv{b_};
  // TODO: iterator view
  for (int i = 0; i < ret.dims()[0]; ++i) {
    auto example = ret[i];
    for (int j = 0; j < example.dims()[0]; ++j) {
      auto channel = example[j].flatten();
      Vector v{channel};
      // cout << v.n() << " vs " << bv.n() << endl;
      v += bv(j);
    }
  }

  return ret;
}

void ConvolutionLayerOperator::applyGradient(const Gradient& g) {
  SCHECK(g.size() == 2);

  if (diagnostics()) {
    cout << folly::format(
        "{} W gradient ratio: {:.2F}({:.2F}%); "
        "B gradient ratio: {:.2F}({:.2F}%)\n",
        name(),
        w_.l2Norm(),
        g[0].l2Norm() / w_.l2Norm() * 100,
        b_.l2Norm(),
        g[1].l2Norm() / b_.l2Norm() * 100);
    setDiagnostics(false);
  }

  w_ += g[0];
  b_ += g[1];
}

std::function<Tensor*()> ConvolutionLayerOperator::getParameters() {
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

void ConvolutionLayerOperator::attachRegularizer(
    RegularizerOperator& regularizer) {
  regularizer.addParameter(&w_);
}

GradientPair ConvolutionLayerOperator::gradientFunc(BackPropOperator* op) {
  auto& parents = op->parents();
  SCHECK(parents.size() == 1);

  // w'(0, 0) = g . x(-offset, -offset): for g(i, j), what was the x(,) that's
  // used for the w(,)

  Tensor wg{w_.dims()};

  auto& g = parents[0].op->inputGradient()[parents[0].inputIndex];
  auto& x = inputs_[0]->get();
  SCHECK(g.dims()[0] == x.dims()[0]);

  for (int e = 0; e < g.dims()[0]; ++e) {
    auto ge = g[e];
    auto xe = x[e];

    for (int o = 0; o < ge.dims()[0]; ++o) {
      auto go = ge[o];
      auto wo = wg[o];
      Matrix gm{go};

      for (int i = 0; i < xe.dims()[0]; ++i) {
        auto xi = xe[i];
        auto woi = wo[i];
        Matrix wm{woi};
        Matrix xm{xi};
        SCHECK(xm.rows() == gm.rows() && xm.cols() == gm.cols());

        cout << "gm: " << go << endl;
        cout << "xm: " << xi << endl;

        for (int r = 0; r < wm.rows(); ++r) {
          for (int c = 0; c < wm.cols(); ++c) {
            wm(r, c) += dot(
                MatrixPatch(gm, 0, 0, gm.rows(), gm.cols()),
                MatrixPatch(
                    xm, -wm.rows() / 2, -wm.cols() / 2, xm.rows(), xm.cols()));
          }
        }
      }
    }
  }

  // b
  Tensor bg{b_.dims()};
  Vector bm{bg};
  for (int e = 0; e < g.dims()[0]; ++e) {
    auto ge = g[e];
    for (int o = 0; o < bm.n(); ++o) {
      auto go = ge[o];
      bm(o) += Matrix{go}.sum();
    }
  }

  // x
  Tensor xg{x.dims()};
  for (int e = 0; e < g.dims()[0]; ++e) {
    auto ge = g[e];
    auto xe = xg[e];

    for (int o = 0; o < ge.dims()[0]; ++o) {
      auto go = ge[o];
      auto wo = w_[o];
      Matrix gm{go};

      for (int i = 0; i < xe.dims()[0]; ++i) {
        auto xi = xe[i];
        auto woi = wo[i];
        Matrix xm{xi};
        Matrix wm{woi};
        int R = wm.rows(), C = wm.cols();

        for (int r = 0; r < xm.rows(); ++r) {
          for (int c = 0; c < xm.cols(); ++c) {
            // Determine the w(,) that is applied to x(r,c) to produce g(0,0)
            int r1 = R / 2 + r;
            int c1 = C / 2 + c;

            // Determine the w(,) that is applied to x(r,c) to produce g(n - 1,
            // m - 1)
            // int r2 = R / 2 - (gm.rows() - 1) + r;
            // int c2 = C / 2 - (gm.cols() - 1) + c;

            xm(r, c) +=
                dot(MatrixPatch(gm, 0, 0, gm.rows(), gm.cols()),
                    MatrixPatch(wm, r1, c1, gm.rows(), gm.cols(), true));
          }
        }
      }
    }
  }

  return GradientPair{Gradient{move(xg)}, Gradient{move(wg), move(bg)}};
}

PoolingOperator::PoolingOperator(int width, int stride, IOperator input)
    : Operator(computeOutputDims(input->dims(), width, stride), {input}),
      width_(width),
      stride_(stride) {}

Tensor& PoolingOperator::compute() {
  auto& x = inputs_[0]->get();
  auto d = dims().addFront(x.dims()[0]);
  get() = Tensor{d};
  auto& ret = get();

  SCHECK(ret.dims()[0] == x.dims()[0] && ret.dims()[1] == x.dims()[1]);

  for (int e = 0; e < x.dims()[0]; e++) {
    auto xe = x[e];
    auto re = ret[e];
    for (int channel = 0; channel < x.dims()[1]; ++channel) {
      auto xc = xe[channel];
      auto rc = re[channel];
      Matrix xm{xc};
      Matrix rm{rc};

      // TODO: make it symmetric. But finish the gradient for the current form
      // first
      for (int r = 0, i = 0; r < xm.rows(); r += stride_, ++i) {
        for (int c = 0, j = 0; c < xm.cols(); c += stride_, ++j) {
          rm(i, j) = std::get<0>(MatrixPatch{xm, r, c, width_, width_}.max());
        }
      }
    }
  }

  return ret;
}

GradientPair PoolingOperator::gradientFunc(BackPropOperator* op) {
  auto& parents = op->parents();
  SCHECK(parents.size() == 1);

  auto& parentG = parents[0].op->inputGradient()[parents[0].inputIndex];
  auto& x = inputs_[0]->get();
  Tensor g{x.dims()};

  for (int e = 0; e < x.dims()[0]; e++) {
    auto xe = x[e];
    auto ge = g[e];
    auto pge = parentG[e];
    for (int channel = 0; channel < x.dims()[1]; ++channel) {
      auto xc = xe[channel];
      auto gc = ge[channel];
      auto pgc = pge[channel];
      Matrix xm{xc};
      Matrix gm{gc};
      Matrix pgm{pgc};

      for (int r = 0, i = 0; r < xm.rows(); r += stride_, ++i) {
        for (int c = 0, j = 0; c < xm.cols(); c += stride_, ++j) {
          Float max;
          int R, C;
          tie(max, R, C) = MatrixPatch{xm, r, c, width_, width_}.max();
          if (R >= 0 && R < gm.rows() && C >= 0 && C < gm.cols()) {
            gm(R, C) += pgm(i, j);
          }
        }
      }
    }
  }

  return make_pair(Gradient{move(g)}, Gradient{});
}

ReluOperator::ReluOperator(IOperator input) : Operator(input->dims(), {input}) {
  SCHECK(input->dims().size() >= 1);
}

Tensor& ReluOperator::compute() {
  get() = inputs_[0]->get();
  auto& ret = get();
  for (auto& x : ret.data()) {
    if (x < 0) {
      x = 0;
    }
  }
  return ret;
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
  get() = Tensor{inputs_[0]->get().dims()};
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
    : Operator(Dims{}, {input, label}) {
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

    s += -log(y) * weight();
  }
  // cout << "weight=" << weight() << endl;

  Tensor ret{Dims{1}};
  Vector{ret}(0) = s;
  get() = ret;

  return get();
}

GradientPair LossOperator::gradientFunc(BackPropOperator* op) {
  Tensor g{inputs_[0]->get().dims()};
  Matrix m{g};
  Matrix in{inputs_[0]->get()};
  Vector label{inputs_[1]->get()};
  for (int i = 0; i < m.rows(); ++i) {
    SCHECK(label(i) < m.cols());
    // Note here we are already applying the 1/m scaling operation needed to
    // compute the averaged loss (same pattern as the forward pass)

    m(i, label(i)) = -1 / in(i, label(i)) * weight();
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
    : LossOperator(
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
        m(i, j) = (1.0 - softmax(i, j)) * weight();
      } else {
        m(i, j) = -softmax(i, j) * weight();
      }
    }
  }

  return make_pair(Gradient{move(g)}, Gradient{});
}

Tensor& L2RegularizerOperator::compute() {
  Float s = 0;
  for (auto* w : parameters_) {
    s += lambda_ * w->l2Sum();
  }

  Tensor ret{Dims{1}};
  Vector{ret}(0) = s;
  get() = ret;

  return get();
}

void L2RegularizerOperator::applyGradient(const Gradient& g) {
  SCHECK_MSG(
      parameters_.size() == g.size(),
      folly::format("{}  vs {}", parameters_.size(), g.size()).str());

  if (diagnostics()) {
    cout << name() << " gradient ratio:";
    for (size_t i = 0; i < g.size(); ++i) {
      auto& w = *parameters_[i];
      cout << folly::format(
          " {:.2F}({:.2F}%)", w.l2Norm(), g[i].l2Norm() / w.l2Norm() * 100);
    }
    cout << endl;
    setDiagnostics(false);
  }

  for (size_t i = 0; i < g.size(); ++i) {
    (*parameters_[i]) += g[i];
  }
}

GradientPair L2RegularizerOperator::gradientFunc(BackPropOperator*) {
  // cout << "L2RegularizerOperator::gradientFunc" << endl;

  Gradient g;
  g.reserve(parameters_.size());
  for (const auto* w : parameters_) {
    Tensor t{w->dims()};
    for (size_t i = 0; i < t.data().size(); ++i) {
      t.data()[i] = lambda_ * w->data()[i] * 2;
      // if (i == 0) {
      //   cout << w->data()[i] << " " << t.data()[i] << endl;
      // }
    }

    g.push_back(move(t));
  }
  return make_pair(Gradient{}, Gradient{move(g)});
}

std::function<Tensor*()> L2RegularizerOperator::getParameters() {
  size_t state = 0;
  return [this, state]() mutable -> Tensor* {
    if (state >= parameters_.size()) {
      return nullptr;
    } else {
      return parameters_[state++];
    }
  };
}
