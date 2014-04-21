#pragma once

#include "Legislator.h"

#include <functional>
#include <mutex>

namespace Paxos {
  namespace Synod {
    namespace Preliminary {
      class Legislator : public Paxos::Legislator {
        using Base = Paxos::Legislator;
      public:
        using DecreeGetter = std::function<Decree()>;

        Legislator(LegislatorId id, Chamber& chamber)
          : Base(id, chamber) {
        }

        // getDecree will only be invoked if we are free to ballot for
        // an arbitrary decree
        void initiateBalloting(DecreeGetter getDecree);

        void process(const NextBallotMessage& nextBallot) override;
        void process(const LastVoteMessage& lastVote) override;
        void process(const BeginBallotMessage& beginBallot) override;
        void process(const VotedMessage& voted) override;
        void process(const SuccessMessage& voted) override;

      private:
        struct Ballot {
          Ballot(BallotNumber b, DecreeGetter getter)
          : number(b),
            decreeGetter(getter) {
          }

          BallotNumber number;
          DecreeGetter decreeGetter;
          std::map<LegislatorId, Vote> lastVotes;

          bool ballotingBegun{ false };
          Decree decree;
          std::set<LegislatorId> votes;
        };

        std::mutex m_;

        int64_t nextBallotNumber_{ 0 };

        // keep track of all ballots initiated by this legislator
        std::map<BallotNumber, Ballot> ballots_;

        std::vector<Vote> votes_;
        // forbidden ranges (a, b) to vote
        std::vector<std::pair<BallotNumber, BallotNumber>> noVotes_;

        // the decree that has been successfully voted
        Decree decree_;
      };
    }
  }
}