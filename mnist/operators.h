#pragma once

#include "tensor.h"

class Operator;
using IOperator = std::shared_ptr<Operator>;

class Operator {
 public:
  virtual ~Operator() {}
  virtual const Dims& dims() const {
    return dims_;
  }
  virtual const Tensor& compute() = 0;
  const Tensor& get() const {
    return output_.value();
  }
  // fix the const protection
  Tensor& get() {
    return output_.value();
  }

 protected:
  Dims dims_;
  std::vector<IOperator> inputs_; // maybe this generic interface is not useful
  folly::Optional<Tensor> output_;
};

class InputOperator : public Operator {
 public:
  InputOperator();
  void load(const ExampleList& examples) {
    output_ = Tensor{examples};
  }
  const Tensor& compute() override {
    return get();
  }
};

class FCLayerOperator : public Operator {
 public:
  FCLayerOperator(int width, IOperator input);
  const Tensor& compute() override;

 private:
  Tensor w_;
  Tensor b_;
};

class ReluOperator : public Operator {
 public:
  ReluOperator(IOperator input);
  const Tensor& compute() override;
};

class SoftmaxOperator : public Operator {
 public:
  SoftmaxOperator(IOperator input);
  const Tensor& compute() override;
};

class LossOperator : public Operator {
 public:
  LossOperator(IOperator input, IOperator label);
  const Tensor& compute() override;
};
