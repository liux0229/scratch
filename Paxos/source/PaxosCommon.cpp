#include "PaxosCommon.h"

namespace Paxos {
  using namespace std;

  ostream& operator<<(ostream& out, LegislatorId id) {
    out << format("Legislator {}", id.id);
    return out;
  }

  ostream& operator<<(ostream& out, BallotNumber ballot) {
    out << format("[Ballot {}:{}]", ballot.number, ballot.legislatorId);
    return out;
  }

  ostream& operator<<(ostream& out, Decree decree) {
    out << format("Decree {}", decree.value);
    return out;
  }
  
  // static
  Vote Vote::getNullVote(LegislatorId legislator) {
    return Vote{ legislator, BallotNumber{ -1, legislator }, Decree{} };
  }

  bool Vote::isNull() const {
    return ballot.number == -1;
  }

  ostream& operator<<(ostream& out, const Vote& vote) {
    out << format("[Vote {} {} {}]", vote.legislator, vote.ballot, vote.decree);
    return out;
  }
}