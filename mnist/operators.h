#pragma once

#include "tensor.h"

#include <functional>

class Operator;
using IOperator = std::shared_ptr<Operator>;
using OperatorList = std::vector<IOperator>;

class BackPropOperator;
using IBackPropOperator = std::shared_ptr<BackPropOperator>;

class RegularizerOperator;

class Operator {
 public:
  Operator(Dims dims, OperatorList inputs) : dims_(dims), inputs_(inputs) {}
  virtual ~Operator() {}
  virtual std::string name() const = 0;
  virtual const Dims& dims() const {
    return dims_;
  }
  virtual Tensor& compute() = 0;
  virtual const Tensor& get() const {
    return output_.value();
  }
  // fix the const protection
  virtual Tensor& get() {
    return output_.value();
  }

  const OperatorList& getInputs() const {
    return inputs_;
  }
  OperatorList& getInputs() {
    return inputs_;
  }

  // Use the brute force way to compute gradient for debugging purposes
  Gradient computeGradientDebug(const std::function<double()>& loss);

  virtual void applyGradient(const Gradient& g) {}

  IBackPropOperator getBackPropOperator();

  void setDiagnostics(bool value) {
    diagnostics_ = value;
  }
  bool diagnostics() const {
    return diagnostics_;
  }

  virtual void attachRegularizer(RegularizerOperator& regularizer) {}

  virtual void read(std::istream& in) {
    expectToken(in, name());
  }
  virtual void write(std::ostream& out) const {
    out << name() << std::endl;
  }

  auto getParameterList() {
    std::vector<const Tensor*> ret;
    auto getter = getParameters();
    while (auto w = getter()) {
      ret.push_back(w);
    }
    return ret;
  }

 protected:
  // The first dimension is implicit: it is the # of examples
  // Think of dim size as the size of one single example
  Dims dims_;
  OperatorList inputs_;
  folly::Optional<Tensor> output_;

 private:
  virtual std::function<Tensor*()> getParameters() {
    return []() { return nullptr; };
  }

  // TODO: add back the const modifier
  virtual GradientPair gradientFunc(BackPropOperator*) = 0;

  IBackPropOperator backPropOp_ = nullptr;

  bool diagnostics_ = false;
};

class BackPropOperator {
 public:
  // A parent consumes the output of this operator in the forward pass
  struct Parent {
    IBackPropOperator op;
    int inputIndex; // The index of this operator in the inputs_ of the parent
  };
  using ParentList = std::vector<Parent>;

  // TODO: add back the const modifier (which may not be very important)
  // we may want to fix the const view problem otherwise it permeates everwhere
  // But also note it may also make the Matrix operators hard to write
  // Idea: use T& construct and use perfect forwarding; let compiler deduce
  // const

  using RunBackProp = std::function<GradientPair(BackPropOperator*)>;
  BackPropOperator(std::string name, const RunBackProp& run)
      : name_(name), run_(run) {}

  std::string name() const {
    return name_;
  }
  void addParent(IBackPropOperator op, int inputIndex) {
    parents_.push_back(Parent{op, inputIndex});
  }

  void runBackProp() {
    std::tie(inputGradient_, parameterGradient_) = run_(this);
  }
  // TODO: add back the const modifier
  Gradient& inputGradient() {
    return inputGradient_;
  }
  Gradient& parameterGradient() {
    return parameterGradient_;
  }

  const ParentList& parents() const {
    return parents_;
  }

 private:
  std::string name_;
  RunBackProp run_;
  Gradient parameterGradient_;
  Gradient inputGradient_;
  ParentList parents_;
};
using BackPropOperatorList = std::vector<IBackPropOperator>;

class InputOperator : public Operator {
 public:
  InputOperator(Dims inputDims) : Operator(inputDims, {}) {}
  std::string name() const override {
    return "input";
  }
  void load(const ExampleList& examples, bool label = false) {
    output_ = Tensor{examples, label};
  }
  Tensor& compute() override {
    return get();
  }

 private:
  GradientPair gradientFunc(BackPropOperator*) override {
    return std::make_pair(Gradient{}, Gradient{});
  }
};
using IInputOperator = std::shared_ptr<InputOperator>;

class FCLayerOperator : public Operator {
 public:
  FCLayerOperator(int width, IOperator input);
  std::string name() const override {
    return "fc-layer";
  }
  Tensor& compute() override;

  void applyGradient(const Gradient& g) override;

  void attachRegularizer(RegularizerOperator& regularizer) override;

  void read(std::istream& in) override;
  void write(std::ostream& out) const override;

 private:
  std::function<Tensor*()> getParameters() override;
  GradientPair gradientFunc(BackPropOperator*) override;

  Tensor w_;
  Tensor b_;
};

class ReluOperator : public Operator {
 public:
  ReluOperator(IOperator input);
  std::string name() const override {
    return "relu";
  }
  Tensor& compute() override;

 private:
  GradientPair gradientFunc(BackPropOperator*) override;
};

class SoftmaxOperator : public Operator {
 public:
  SoftmaxOperator(IOperator input);
  std::string name() const override {
    return "softmax";
  }
  Tensor& compute() override;

 private:
  GradientPair gradientFunc(BackPropOperator*) override;
};
using ISoftmaxOperator = std::shared_ptr<SoftmaxOperator>;

class LossOperator : public Operator {
 public:
  LossOperator(IOperator input, IOperator label);
  std::string name() const override {
    return "loss";
  }
  Tensor& compute() override;

 private:
  GradientPair gradientFunc(BackPropOperator*) override;
};
using ILossOperator = std::shared_ptr<LossOperator>;

// Fuse softmax + loss to avoid numeric instability because of the division
class SoftmaxLossOperator : public Operator {
 public:
  SoftmaxLossOperator(ISoftmaxOperator softmaxOp, ILossOperator lossOp);
  std::string name() const override {
    return "softmax_loss";
  }
  Tensor& compute() override;
  const Tensor& get() const override {
    return lossOp_->get();
  }
  Tensor& get() override {
    return lossOp_->get();
  }

 private:
  GradientPair gradientFunc(BackPropOperator*) override;

  ISoftmaxOperator softmaxOp_;
  ILossOperator lossOp_;
};

class RegularizerOperator : public Operator {
 public:
  RegularizerOperator(Float lambda) : Operator({}, {}), lambda_(lambda){};
  void addParameter(Tensor* w) {
    parameters_.push_back(w);
  }

 protected:
  std::vector<Tensor*> parameters_;
  Float lambda_;
};

using IRegularizerOperator = std::shared_ptr<RegularizerOperator>;

class L2RegularizerOperator : public RegularizerOperator {
 public:
  using RegularizerOperator::RegularizerOperator;
  std::string name() const override {
    return "l2_regularizer";
  }
  Tensor& compute() override;
  void applyGradient(const Gradient& g) override;

 private:
  std::function<Tensor*()> getParameters() override;
  GradientPair gradientFunc(BackPropOperator*) override;
};
