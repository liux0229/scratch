#pragma once

#include "common.h"

enum class Algorithm {
  CONST,
};

class Trainer {
public:
  static IModel train(ExampleList examples, Algorithm algorithm);
};
