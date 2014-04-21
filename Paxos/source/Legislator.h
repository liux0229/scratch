#pragma once

#include "Messages.h"

#include <memory>
#include <vector>

namespace Paxos {

  class Chamber;
  class Legislator {
  public:
    ~Legislator() { }
    LegislatorId id() const { return id_; }

    virtual void process(const NextBallotMessage& nextBallot) = 0;
    virtual void process(const LastVoteMessage& lastVote) = 0;
    virtual void process(const BeginBallotMessage& beginBallot) = 0;
    virtual void process(const VotedMessage& voted) = 0;
    virtual void process(const SuccessMessage& success) = 0;
  protected:
    Legislator(LegislatorId id, Chamber& chamber)
      : id_(id),
      chamber_(chamber) {
    } 
    Vote getNullVote() const;
    Chamber& chamber() const { return chamber_; }

  private:
    LegislatorId id_;
    Chamber& chamber_;
  };
  using ULegislator = std::unique_ptr<Legislator>;
}