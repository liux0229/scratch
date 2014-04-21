#pragma once

#include "PaxosCommon.h"

namespace Paxos {
  struct NextBallotMessage : Message {
    NextBallotMessage(LegislatorId sender, BallotNumber ballot)
    : Message(sender),
      ballotNumber(ballot) {
    }
    void deliver(Legislator& legislator) const override;

    BallotNumber ballotNumber;
  };

  struct LastVoteMessage : Message {
    LastVoteMessage(LegislatorId sender, BallotNumber b, const Vote& lv)
      : Message(sender),
        ballot(b),
        lastVote(lv) {
    }
    void deliver(Legislator& legislator) const override;

    BallotNumber ballot;
    Vote lastVote;
  };

  struct BeginBallotMessage : Message {
    BeginBallotMessage(LegislatorId sender, BallotNumber b, Decree d)
    : Message(sender),
      ballot(b),
      decree(d) {
    }
    void deliver(Legislator& legislator) const override;
    
    BallotNumber ballot;
    Decree decree;
  };

  struct VotedMessage : Message {
    VotedMessage(LegislatorId sender, BallotNumber b, Decree d)
    : Message(sender),
      ballot(b),
      decree(d) {
    }
    void deliver(Legislator& legislator) const override;

    BallotNumber ballot;
    Decree decree;
  };

  struct SuccessMessage : Message {
    SuccessMessage(LegislatorId sender, Decree d) 
    : Message(sender),
      decree(d) {
    }
    void deliver(Legislator& legislator) const override;

    Decree decree;
  };
}