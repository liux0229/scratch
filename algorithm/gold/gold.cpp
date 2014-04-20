#include <iostream>
#include <array>
#include <algorithm>
using namespace std;

class Gold {
 public:
  Gold() : value_{} { }
  Gold(int v) : value_(v) { }
  bool operator==(const Gold& other) const { 
    return value_ == other.value_;
  }
  bool operator<(const Gold& other) const { 
    return value_ < other.value_;
  }
  bool operator>(const Gold& other) const { 
    return value_ > other.value_;
  }
  Gold& operator++() {
    ++value_;
    return *this;
  }
  Gold& operator--() {
    --value_;
    return *this;
  }

 private:
  int value_;
};

using GoldSet = array<Gold, 12>;

GoldSet getGold() {
  GoldSet ret;
  for (auto& v : ret) {
    v = 10;
  }
  ++ret[0];
  random_shuffle(ret.begin(), ret.end());
  return ret;
}

class GoldSorter {
 public:
  void sort(const GoldSet& goldSet) {
  }
};

int main()
{
  auto goldSet = getGold();
  GoldSorter().sort(goldSet);
}
