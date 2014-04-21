#pragma once

#include "Legislator.h"

#include <map>

namespace Paxos {

  class Chamber {
  public:
    void add(ULegislator legislator);
    void send(LegislatorId receiver, UMessage msg);
    void broadcast(UMessage msg);
    bool isMajority(size_t n) const;
  private:
    std::map<LegislatorId, ULegislator> legislators_;
  };

}
