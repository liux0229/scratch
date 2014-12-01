#include <cassert>
#include <algorithm>

#include "Allocator.h"

namespace {

void CHECK(bool c) {
  assert(c);
}

constexpr size_t Word = 4;
constexpr size_t DoubleWord = Word * 2;
constexpr size_t Expansion = 1 << 12; // 4K
constexpr size_t MinBlock = DoubleWord * 2;

size_t roundUp(size_t size) {
  // return x which satisfy x >= size and x is a multiple of DoubleWord;
  return (size + DoubleWord - 1) / DoubleWord * DoubleWord;
}

} // unnamed

class AllocatedBlock;
class FreeBlock;

// The key of implementing Block is that we want to use it to raise the level of
// abstraction. Yet the Block data structure is imposed on top of the blocks we
// actually allocating, which means we need to use in place new.
// `this` points to the start of the Block.
// We cannot destruct Block objects for performance reasons, and we rely on
// static typing for better correctness. There are only definite points in the
// program where we perform "explicit casts" (in place new); all other places we
// directly work on higher level objects.
class Block {
  class Marker {
   public:
    // Impose type information on an application returned block.
    Marker() { }

    Marker(size_t size, bool allocated) : value_(size | allocated) {
      CHECK(size % DoubleWord == 0);
    }
    size_t size() const {
      return value_ & ~0x7;
    }
    bool allocated() const {
      return value_ & 0x1;
    }

   private:
    uint32_t value_;
  };
  static_assert(sizeof(Marker) == Word, "");

 public:
  // Consider macro out `this`
  Block& next() {
    return *reinterpret_cast<Block*>(getThis() + size());
  }
  Block& previous() {
    // No check for now, which is safe.
    return *reinterpret_cast<Block*>(
        getThis() - reinterpret_cast<Marker*>(getThis() - Word)->size());
  }

  size_t size() const { return header_.size(); }
  bool allocated() const {
    return header_.allocated();
  }

  // Return null if allocated.
  FreeBlock* getFree();

  void verify() {
    if (size() > 0) {
      CHECK(header().size() == footer().size());
      CHECK(header().allocated() == footer().allocated());
    }
  }

  bool isEpilogue() const {
    return header_.size() == 0;
  }

 protected:
  // Impose type information on an application returned block.
  Block() { }
  // Construct both header and footer.
  // Footer is not explicitly represented as an object but the fact that a Block
  // constructed implies that the footer is also constructed.
  Block(size_t targetSize, bool allocated)
      : header_(roundUp(targetSize), allocated) {
    if (size() > 0) {
      new (&footer()) Marker(header_);
    }
    // else: header and footer are combined.
  }
  Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;

  // We want to perform pointer arithmetic on char*, not on Block*.
  char* getThis() {
    return reinterpret_cast<char*>(this);
  }

 private:
  // don't bother with protected as that may change the layout.
  friend class AllocatedBlock;

  Marker& header() { return header_; }
  Marker& footer() {
    return *reinterpret_cast<Marker*>(getThis() + size() - Word);
  }

  // Note while we can directly place header_, footer_'s address is calculated
  // (and we don't place that address inside the Block; it is inferred)
  Marker header_;

  // Just an easy way to refer to the payload (if any).
  // Note its size is not necessarily 1 byte.
  char payload_;
}; // class Block

class AllocatedBlock : public Block {
 public:
  AllocatedBlock(size_t size) : Block(size, true) {}

  // We use this construct to impose the `type information` back onto a
  // pointer we receive from the application.
  static AllocatedBlock& fromPayload(void* ptr) {
    return *(new (static_cast<char*>(ptr) - Word) AllocatedBlock());
  }

  char* payload() { return &payload_; }

  // We could also consider using r-value references to indicate the object
  // cannot be used any more after this operation.
  // I wonder whether Rust can solve this better.
  FreeBlock* free();

 private:
  AllocatedBlock() {}
};

class FreeBlock : public Block {
 public:
  FreeBlock(size_t size) : Block(size, false) {}

  AllocatedBlock* place(size_t targetSize) {
    CHECK(targetSize <= size());

    if (targetSize + MinBlock > size()) {
      // note: this is arguably more efficient than performing a
      // read-modify-write on header and footer.
      return new (getThis()) AllocatedBlock(size());
    } else {
      auto left = size() - targetSize;
      auto* ret = new (getThis()) AllocatedBlock(targetSize);
      new (&ret->next()) FreeBlock(left);
      return ret;
    }
  }

  FreeBlock* coalesce() {
    bool before = previous().allocated();
    bool after = next().allocated();
    if (before && after) {
      return this;
    } else if (before && !after) {
      return new (getThis()) FreeBlock(size() + next().size());
    } else if (!before && after) {
      return new (&previous()) FreeBlock(size() + previous().size());
    } else {
      return new (&previous())
          FreeBlock(size() + previous().size() + next().size());
    }
  }
};

FreeBlock* Block::getFree() {
  if (allocated()) {
    return nullptr;
  } else {
    return static_cast<FreeBlock*>(this);
  }
}

FreeBlock* AllocatedBlock::free() {
  auto* block = new (getThis()) FreeBlock(size());
  return block->coalesce();
}

class Prologue : public Block {
 public:
  Prologue() : Block(DoubleWord, true) {}
};

class Epilogue : public Block {
 public:
  Epilogue() : Block(0, true) {}

  std::pair<FreeBlock*, Epilogue*> insertBefore(size_t size) {
    // turn the current block into a normal block and append an epilogue
    // afterwards
    //
    // double construction is fine.
    //
    auto* freeBlock = new (getThis()) FreeBlock(size);
    auto* epilogue = new (&freeBlock->next()) Epilogue();
    freeBlock = freeBlock->coalesce();
    return std::make_pair(freeBlock, epilogue);
  }
};

Allocator::Allocator() {
  char* p = expand(2 * DoubleWord);
  auto* prologue = new (p + Word) Prologue();

  epilogue_ = new (&prologue->next()) Epilogue();
  first_ = prologue; 
}

void* Allocator::allocate(size_t size) {
  if (size == 0) {
    return nullptr;
  }

  size = roundUp(size + DoubleWord);
  for (auto* block = first_; !block->isEpilogue(); block = &block->next()) {
    if (auto* freeBlock = block->getFree()) {
      if (freeBlock->size() >= size) {
        auto* allocated = freeBlock->place(size);
        auto* ret = allocated->payload();
        verify();
        return ret;
      }
    }
  }

  auto expandSize = std::max(size, Expansion);
  if (!expand(expandSize)) {
    return nullptr;
  }

  verify();

  FreeBlock* block;
  std::tie(block, epilogue_) = epilogue_->insertBefore(expandSize);

  auto* ret = block->place(size)->payload();
  verify();

  return ret;
}

void Allocator::free(void* ptr) {
  if (!ptr) {
    return;
  }

  auto& block = AllocatedBlock::fromPayload(ptr);
  block.free();

  verify();
}

// Return the address pointed by next_ before the expansion
char* Allocator::expand(size_t size) {
  // size must be multiples of double words
  size = roundUp(size);
  if (next_ + size > MaxHeap) {
    return nullptr;
  }
  auto ret = heap_.get() + next_;
  next_ += size;
  return ret;
}

void Allocator::verify() const {
#ifdef ENABLE_VERIFY
  for (auto* block = first_; !block->isEpilogue(); block = &block->next()) {
    block->verify();
  }
#endif
}
