#include "Chamber.h"

namespace Paxos {
  using namespace std;

  bool Chamber::isMajority(size_t n) const {
    return n > legislators_.size() / 2;
  }

  void Chamber::add(ULegislator legislator) {
    auto id = legislator->id();
    legislators_.insert(make_pair(id, move(legislator)));
  }

  void Chamber::broadcast(UMessage msg) {
    for (auto& kv : legislators_) {
      msg->deliver(*kv.second);
    }
  }

  void Chamber::send(LegislatorId id, UMessage msg) {
    auto it = legislators_.find(id);
    if (it == legislators_.end()) {
      Throw("{} is not present in the chamber", id);
    }
    msg->deliver(*it->second);
  }
}