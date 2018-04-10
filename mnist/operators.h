#pragma once

#include "tensor.h"

class Operator;
using IOperator = std::shared_ptr<Operator>;
using OperatorList = std::vector<IOperator>;

class Operator {
 public:
  Operator(Dims dims, OperatorList inputs) : dims_(dims), inputs_(inputs) {}
  virtual ~Operator() {}
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

 protected:
  // The first dimension is implicit: it is the # of examples
  // Think of dim size as the size of one single example
  Dims dims_;
  OperatorList inputs_;
  folly::Optional<Tensor> output_;
};

class InputOperator : public Operator {
 public:
  InputOperator(Dim inputDim) : Operator({inputDim}, {}) {}
  void load(const ExampleList& examples) {
    output_ = Tensor{examples};
  }
  Tensor& compute() override {
    return get();
  }
};
using IInputOperator = std::shared_ptr<InputOperator>;

class FCLayerOperator : public Operator {
 public:
  FCLayerOperator(int width, IOperator input);
  Tensor& compute() override;

 private:
  Tensor w_;
  Tensor b_;
};

class ReluOperator : public Operator {
 public:
  ReluOperator(IOperator input);
  Tensor& compute() override;
};

class SoftmaxOperator : public Operator {
 public:
  SoftmaxOperator(IOperator input);
  Tensor& compute() override;
};

class LossOperator : public Operator {
 public:
  LossOperator(IInputOperator input, IOperator label);
  Tensor& compute() override;
};
