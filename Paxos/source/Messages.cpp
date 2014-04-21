#include "Messages.h"
#include "Legislator.h"

namespace Paxos {
  using namespace std;
  
  ostream& operator<<(ostream& out, LegislatorId id) {
    out << format("Legislator {}", id.id);
    return out;
  }
  
  void NextBallotMessage::deliver(Legislator& legislator) const {
    legislator.process(*this);
  }

  void LastVoteMessage::deliver(Legislator& legislator) const {
    legislator.process(*this);
  }

  void BeginBallotMessage::deliver(Legislator& legislator) const {
    legislator.process(*this);
  }

  void VotedMessage::deliver(Legislator& legislator) const {
    legislator.process(*this);
  }

  void SuccessMessage::deliver(Legislator& legislator) const {
    legislator.process(*this);
  }
}