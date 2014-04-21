#include "Messages.h"
#include "Legislator.h"

namespace Paxos {
  using namespace std;
 
  void NextBallotMessage::deliver(Legislator& legislator) const {
    legislator.process(*this);
  }

  void NextBallotMessage::output(ostream& out) const {
    out << format("[{} => NextBallot: {}]", sender, ballotNumber);
  }

  void LastVoteMessage::deliver(Legislator& legislator) const {
    legislator.process(*this);
  }

  void LastVoteMessage::output(ostream& out) const {
    out << format("[{} => LastVote: {} {}]", sender, ballot, lastVote);
  }

  void BeginBallotMessage::deliver(Legislator& legislator) const {
    legislator.process(*this);
  }

  void BeginBallotMessage::output(ostream& out) const {
    out << format("[{} => BeginBallot: {} {}]", sender, ballot, decree);
  }

  void VotedMessage::deliver(Legislator& legislator) const {
    legislator.process(*this);
  }

  void VotedMessage::output(ostream& out) const {
    out << format("[{} => Voted: {} {}]", sender, ballot, decree);
  }

  void SuccessMessage::deliver(Legislator& legislator) const {
    legislator.process(*this);
  }

  void SuccessMessage::output(ostream& out) const {
    out << format("[{} => Success: {}]", sender, decree);
  }
}