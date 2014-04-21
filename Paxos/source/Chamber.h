#pragma once

#include "Legislator.h"
#include "WorkQueue.h"
#include "Random.h"

#include <thread>

namespace Paxos {

  class Chamber {
  public:
    Chamber();
    ~Chamber();
    void add(ULegislator legislator);
    void start();

    size_t totalLegislators() const {
      return legislators_.size();
    }
    Legislator& getLegislator(LegislatorId id);

    void send(LegislatorId receiver, SMessage msg);
    void broadcast(SMessage msg);
    bool isMajority(size_t n) const;
  private:
    struct MessageInfo {
      bool isStopper() {
        return payload == nullptr;
      }
      LegislatorId receiver;
      SMessage payload;
    };

    void dispatch();
    void deliver(LegislatorId receiver, SMessage msg);
    bool messageLost();

    std::map<LegislatorId, ULegislator> legislators_;
    WorkQueue<MessageInfo> messages_;
    Random random_;
    std::thread dispatcher_;
  };

}
