#pragma once

#include "tensor.h"

#include <functional>

class Operator;
using IOperator = std::shared_ptr<Operator>;
using OperatorList = std::vector<IOperator>;

class BackPropOperator;
using IBackPropOperator = std::shared_ptr<BackPropOperator>;

class Operator {
 public:
  Operator(Dims dims, OperatorList inputs) : dims_(dims), inputs_(inputs) {}
  virtual ~Operator() {}
  virtual std::string name() const = 0;
  virtual const Dims& dims() const {
    return dims_;
  }
  virtual Tensor& compute() = 0;
  const Tensor& get() const {
    return output_.value();
  }
  // fix the const protection
  Tensor& get() {
    return output_.value();
  }

  const OperatorList& getInputs() const {
    return inputs_;
  }

  // Use the brute force way to compute gradient for debugging purposes
  Gradient computeGradientDebug(const std::function<double()>& loss);

  virtual void applyGradient(const Gradient& g) {}

  IBackPropOperator getBackPropOperator();

 protected:
  // The first dimension is implicit: it is the # of examples
  // Think of dim size as the size of one single example
  Dims dims_;
  OperatorList inputs_;
  folly::Optional<Tensor> output_;

  // TODO
  // 1. getBackPropOperator() stores a cache
  // 2. create back-prop operators when creating the backward graph
  // 3. requery the backprop operator and create input edges (add edges later)
  // 4. Each input has its own gradient
  // 5. Back Prop operator that is independent. parent->get() should take an
  // index
 private:
  virtual std::function<Tensor*()> getParameters() {
    return []() { return nullptr; };
  }

  // TODO: add back the const modifier
  virtual GradientPair gradientFunc(BackPropOperator*) = 0;

  IBackPropOperator backPropOp_ = nullptr;
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

  void applyGradient(const Gradient& g) {
    SCHECK(g.size() == 2);
    w_ += g[0];
    b_ += g[1];
  }

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
