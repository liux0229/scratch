#pragma once

#include "PreliminarySynodLegislator.h"
#include "Chamber.h"
#include "Random.h"

namespace Paxos {
  /// Implements the Preliminary Synod protocol.
  /// This protocol guarantees consistency but no progress.
  /// There is no president in this protocol. We'll simulate
  /// balloting by randomly ask a legislator to initiate 
  /// balloting. What we should expect from this protocol, is
  /// that the synod should not pass two different decrees
  /// (the consistency guarantee).

  // TODO: we could probably also simulate legislators leave
  // (and then re-enter the chamber, but that's probably
  // the same as adding a message loss)
  class PreliminarySynod {
    
  public:
    // Simulate a preliminary synod with n legislators.
    PreliminarySynod(size_t n);
    
    // Start simulating.
    void start();
  private:
    Chamber chamber_;
    Random random_;
  };
}