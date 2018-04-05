#pragma once

#include "common.h"

enum class Algorithm {
  CONST,
  MLP,
};

class Trainer {
public:
  static IModel train(ExampleList examples, Algorithm algorithm);
};
