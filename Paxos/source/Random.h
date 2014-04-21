#pragma once

#include <random>
#include <cstdint>

namespace Paxos {

  class Random {
  public:
    Random()
      : engine_(std::random_device{}()) {
    }

    template<typename T>
    T getInt(T min, T max) {
      std::uniform_int_distribution<T> dist{ min, max };
      return dist(engine_);
    }

    template<typename T>
    T getReal(T min, T max) {
      std::uniform_real_distribution<T> dist{ min, max };
      return dist(engine_);
    }
  private:
    std::mt19937_64 engine_;
  };

}