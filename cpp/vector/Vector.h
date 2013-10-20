#include <memory>
#include <utility>
#include <iostream>

namespace library {

template<typename T, typename A = std::allocator<T>>
struct VectorBase;
template<typename T, typename A>
void swap(VectorBase<T, A>& a, VectorBase<T, A>& b);

template<typename T, typename A>
struct VectorBase {
  using SizeType = typename A::size_type;
  VectorBase(A a, SizeType n, SizeType m = 0)
    : alloc(a),
      elem(a.allocate(n + m)),
      space(elem + n),
      last(elem + n + m) { }

  ~VectorBase() {
    alloc.deallocate(elem, last - elem);
  }

  VectorBase(VectorBase&& rhs)
    : alloc(rhs.alloc),
      elem(rhs.elem),
      space(rhs.space),
      last(rhs.last) {
    rhs.elem = rhs.space = rhs.last = nullptr; 
  }

  VectorBase& operator=(VectorBase&& rhs) {
    swap(*this, rhs);
    return *this;
  }

  // default generated copy operations would not work
  VectorBase(const VectorBase&) = delete;
  VectorBase& operator=(const VectorBase&) = delete;

  A alloc;
  T* elem;
  T* space;
  T* last;
};

template<typename T, typename A>
void swap(VectorBase<T, A>& a, VectorBase<T, A>& b) {
  using std::swap;
  swap(a.alloc, b.alloc);
  swap(a.elem, b.elem);
  swap(a.space, b.space);
  swap(a.last, b.last);
}

template<typename T, typename A = std::allocator<T>>
class Vector;
template<typename T, typename A>
void swap(Vector<T, A>& a, Vector<T, A>& b);

template<typename T, typename A>
class Vector {
  using SizeType = typename A::size_type;
 public:
  using iterator = T*;
  using const_iterator = const T*;

  iterator begin() const { return base_.elem; }
  iterator end() const { return base_.space; }
  const_iterator cbegin() const { return base_.elem; }
  const_iterator cend() const { return base_.space; }
  SizeType size() const { return base_.space - base_.elem; }
  SizeType capacity() const { return base_.last - base_.elem; }

  explicit Vector(SizeType n = 0, const T& v = T{}, A alloc = A{})
    : base_(alloc, n) {
      std::uninitialized_fill(base_.elem, base_.space, v);
  }

  Vector(Vector&& rhs)
    : base_(move(rhs.base_)) { 
  }

  Vector& operator=(Vector&& rhs) {
    swap(*this, rhs);
    return *this;
  }

  Vector(const Vector& rhs)
    : base_(rhs.base_.alloc, rhs.size()) {
    std::uninitialized_copy(rhs.base_.elem, rhs.base_.space, base_.elem);
  }

  Vector& operator=(const Vector& rhs) {
    Vector t{rhs};
    swap(*this, t);
    return *this;
  }
 private:
  friend void swap(Vector& a, Vector& b) {
    swap(a.base_, b.base_);
  }
  VectorBase<T, A> base_;
};

}
