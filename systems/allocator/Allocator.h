#pragma once

#include <memory>

/// A heap allocator based on implicit free lists which uses boundary tags to
/// implement coalescing.
/// The goal of this exercise is to show this low level programming can be
/// implemented in a higher level language like C++ with better type checking.
class Block;
class Epilogue;

class Allocator {
 public:
  Allocator();
  /// Only support size <= 2^32 due to the size of the block header.
  void *allocate(size_t size);
  void free(void *ptr);

 private:
  // In the current design the heap area must be contiguous.
  // Thus we allocate only once.
  static constexpr size_t MaxHeap = 1 << 28; // 256M

  char* expand(size_t size);

  void verify() const;

  // Just hold the heap area.
  std::unique_ptr<char[]> heap_{new char[MaxHeap]};

  // Start of the unallocated heap area.
  size_t next_{0};

  Block* first_;
  Epilogue* epilogue_;
};
