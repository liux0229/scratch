#pragma once

#include <functional>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include <folly/Optional.h>

namespace V1 {
/// Answer top k query with each arrival from a stream of additions and
/// deletions in O(logN).
template <typename Item>
class TopK {
 public:
  using Compare = std::function<bool(Item, Item)>;
  using Callback = std::function<void(Item)>;
  TopK(size_t k, Compare less, Callback in, Callback out)
      : k_(k), less_(less), in_(in), out_(out) {
    CHECK(k_ > 0);
  }

  void add(Item item) {
    if (deleted_.count(item)) {
      deleted_.erase(item);
      if (isChosen(item)) {
        markIn(item);
        if (nChosen_ > k_) {
          moveMinTopToMax();
        }
      }
      return;
    }

    if (nChosen_ < k_) {
      enterMinHeap(item);
      markIn(item);
    } else {
      auto top = getTop(&minHeap_);
      CHECK(top);
      if (less_(*top, item)) {
        moveMinTopToMax();

        enterMinHeap(item);
        markIn(item);
      } else {
        maxHeap_.push(item);
      }
    }
  }

  void remove(Item item) {
    bool wasChosen = isChosen(item);
    deleted_.insert(item);

    if (wasChosen) {
      markOut(item);
      moveMaxTopToMin();
    }
  }

 private:
  class CompareWrapper {
   public:
    CompareWrapper(Compare less, bool greater)
        : less_(less), greater_(greater) {}
    bool operator()(Item a, Item b) const {
      return greater_ ? less_(b, a) : less_(a, b);
    }

   private:
    Compare less_;
    bool greater_;
  };
  using Heap = std::priority_queue<Item, std::vector<Item>, CompareWrapper>;

  bool isChosen(Item item) const {
    return inMinHeap_.count(item) && !deleted_.count(item);
  }

  void moveMinTopToMax() {
    auto top = getTop(&minHeap_);
    CHECK(top);
    markOut(*top);

    inMinHeap_.erase(*top);
    minHeap_.pop();
    maxHeap_.push(*top);
  }

  void moveMaxTopToMin() {
    if (auto top = getTop(&maxHeap_)) {
      maxHeap_.pop();
      enterMinHeap(*top);
      markIn(*top);
    }
  }

  void enterMinHeap(Item item) {
    minHeap_.push(item);
    inMinHeap_.insert(item);
  }

  void markIn(Item item) {
    ++nChosen_;
    in_(item);
  }

  void markOut(Item item) {
    --nChosen_;
    out_(item);
  }

  // Get heap top with consideration of deleted elements.
  folly::Optional<Item> getTop(Heap* heap) {
    while (!heap->empty() && deleted_.count(heap->top())) {
      deleted_.erase(heap->top());
      if (heap == &minHeap_) {
        inMinHeap_.erase(heap->top());
      }
      heap->pop();
    }
    if (heap->empty()) {
      return folly::none;
    } else {
      return heap->top();
    }
  }

  size_t k_;
  Compare less_;
  Callback in_;
  Callback out_;
  Heap minHeap_{CompareWrapper{less_, /* greater: */ true}};
  Heap maxHeap_{CompareWrapper{less_, false}};
  std::unordered_set<Item> inMinHeap_;
  std::unordered_set<Item> deleted_;
  size_t nChosen_{0};
};

} // V1

namespace V2 {

/// O(K) per operation
template <typename Item>
class TopK {
 public:
  using Compare = std::function<bool(Item, Item)>;
  using Callback = std::function<void(Item)>;
  TopK(size_t k, Compare less, Callback in, Callback out)
      : k_(k), less_(less), in_(in), out_(out) {
    CHECK(k_ > 0);
  }

  void add(Item item) {
    order_.insert(item);
    size_t n = 0;
    auto it = order_.begin();
    while (n < k_ && it != order_.end()) {
      auto x = *it++;
      if (!chosen_.count(x)) {
        chosen_.insert(x);
        in_(x);
      }
      ++n;
    }
    if (chosen_.size() > k_) {
      CHECK(it != order_.end());
      CHECK(chosen_.count(*it));
      chosen_.erase(*it);
      out_(*it);
    }
  }

  void remove(Item item) {
    order_.erase(item);
    if (chosen_.count(item)) {
      chosen_.erase(item);
      out_(item);

      if (order_.size() > chosen_.size()) {
        auto it = std::next(order_.begin(), k_ - 1);
        CHECK(!chosen_.count(*it));
        chosen_.insert(*it);
        in_(*it);
      }
    }
  }

 private:
  class CompareWrapper {
   public:
    explicit CompareWrapper(Compare less) : less_(less) {}
    bool operator()(Item a, Item b) const { return compare(b, a); }

   private:
    bool compare(Item a, Item b) const {
      if (less_(a, b)) {
        return true;
      } else if (!less_(b, a)) {
        // equivalent, use item identity to disambiguate.
        return a < b;
      }
      return false;
    }

    Compare less_;
  };

  size_t k_;
  Compare less_;
  Callback in_;
  Callback out_;
  std::set<Item, CompareWrapper> order_{CompareWrapper{less_}};
  std::set<Item> chosen_;
};

} // V2

template <typename T>
using TopK = V1::TopK<T>;
