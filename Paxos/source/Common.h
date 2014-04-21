#pragma once

#include "Exceptions.h"
#include "format.h"
#include <cassert>
#include <utility>
#include <memory>
#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <vector>
#include <utility>
#include <functional>

#define CHECK(f) assert(f)
#define MCHECK(f, msg) do {\
if (!(f)) { \
  std::cerr << (msg) << std::endl; \
  assert(f); \
} \
} while (false)

#define CALL_MEM_FUNC(obj, ptr) ((obj).*ptr)

#define MakeUnique(type) using U ## type = std::unique_ptr<type>
#define MakeShared(type) using S ## type = std::shared_ptr<type>

namespace Paxos {

  namespace {

#if WIN32
    using std::make_unique;
#else
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args)
    {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
#endif

    template<typename... Args>
    void Throw(const char* fmt, Args&&... args)
    {
      throw PaxosException(format(fmt, std::forward<Args>(args)...));
    }
  }
}