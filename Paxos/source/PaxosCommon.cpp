#include "PaxosCommon.h"

namespace Paxos {
  // static
  Vote Vote::getNullVote(LegislatorId legislator) {
    return Vote{ legislator, BallotNumber{ -1, legislator }, Decree{} };
  }

  bool Vote::isNull() const {
    return ballot.number == -1;
  }
}