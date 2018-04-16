#pragma once

#include "tensor.h"

#include <functional>

class Operator;
using IOperator = std::shared_ptr<Operator>;
using OperatorList = std::vector<IOperator>;

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
};

class SoftmaxOperator : public Operator {
 public:
  SoftmaxOperator(IOperator input);
  std::string name() const override {
    return "softmax";
  }
  Tensor& compute() override;
};

class LossOperator : public Operator {
 public:
  LossOperator(IOperator input, IOperator label);
  std::string name() const override {
    return "loss";
  }
  Tensor& compute() override;
};
