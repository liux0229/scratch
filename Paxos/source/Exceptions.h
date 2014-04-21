#pragma once

#include <exception>
#include <string>

namespace Paxos {

  class PaxosException : public std::exception {
  public:
    PaxosException(std::string message) : message_(std::move(message)) {
    }
    const char* what() const override {
      return message_.c_str();
    }
  private:
    std::string message_;
  };

}
