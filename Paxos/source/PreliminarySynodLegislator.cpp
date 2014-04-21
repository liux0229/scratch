#include "PreliminarySynodLegislator.h"
#include "Chamber.h"

// Note the implementation below assumes that message passing is asynchronous
// which does not block; we could have improved this by not including 
// message passing as part of the lock.
namespace Paxos {
  namespace Synod {
    namespace Preliminary {
      using namespace std;
      using Lock = std::unique_lock<std::mutex>;

      void Legislator::initiateBalloting(DecreeGetter decreeGetter) {
        Lock lock(m_);
        
        BallotNumber b{ nextBallotNumber_++, id() };
        ballots_.insert(make_pair(b, Ballot{b, decreeGetter}));
        chamber().broadcast(make_shared<NextBallotMessage>(id(), b));
      }

      void Legislator::process(const NextBallotMessage& nextBallot) {
        Lock lock(m_);
        
        Vote lastVote = getNullVote();
        for (auto& v : votes_) {
          if (v.ballot < nextBallot.ballotNumber) {
            lastVote = max(lastVote, v);
          }
        }

        // we are going to tell the sender of the nextBallot message that
        // the lastVote answer is 'lastVote'. We can make sure that answer
        // does not change by promising to not vote in 
        // (lastVote.ballot ... nextBallot.ballotNumber)
        noVotes_.push_back(make_pair(lastVote.ballot, nextBallot.ballotNumber));

        chamber().send(nextBallot.sender, 
                       make_shared<LastVoteMessage>(id(), nextBallot.ballotNumber, lastVote));
      }

      void Legislator::process(const LastVoteMessage& lastVote) {
        Lock lock(m_);
        
        auto& ballot = ballots_.at(lastVote.ballot);
        if (ballot.ballotingBegun) {
          return;
        }

        ballot.lastVotes.insert(make_pair(lastVote.sender, lastVote.lastVote));
        if (!chamber().isMajority(ballot.lastVotes.size())) {
          return;
        }

        // we have a majority of legislators responded with lastVote;
        // select that set as the quorum of this ballot and begin balloting
        Vote maxi = getNullVote();
        for (auto& kv : ballot.lastVotes) {
          maxi = max(maxi, kv.second);
        }
        if (maxi.isNull()) {
          // the decree could be arbitrary
          ballot.decree = ballot.decreeGetter();
        } else {
          ballot.decree = maxi.decree;
        }

        ballot.ballotingBegun = true;
        // go through the quorum and send beginBallot
        for (auto& kv : ballot.lastVotes) {
          chamber().send(kv.second.legislator, 
                         make_shared<BeginBallotMessage>(id(), ballot.number, ballot.decree));
        }
      }

      void Legislator::process(const BeginBallotMessage& beginBallot) {
        Lock lock(m_);
        
        // determine if this legislator could vote on the ballot
        for (auto& r : noVotes_) {
          if (r.first < beginBallot.ballot && beginBallot.ballot < r.second) {
            cout << format("{} declines to vote on {} due to restriction ({},{})\n",
                           id(), 
                           beginBallot,
                           r.first,
                           r.second);
            return;
          }
        }

        // otherwise, always vote
        Vote vote{id(), beginBallot.ballot, beginBallot.decree};
        // remember the voting decision
        votes_.push_back(vote);

        chamber().send(beginBallot.sender, 
                       make_shared<VotedMessage>(id(), vote.ballot, vote.decree));
      }

      void Legislator::process(const VotedMessage& voted) {
        Lock lock(m_);
        
        auto& ballot = ballots_.at(voted.ballot);
        if (!ballot.ballotingBegun) {
          // TODO: more comprehensive error reporting
          Throw("[{}] voted message received for ballot that has not begun", id());
        }

        ballot.votes.insert(voted.sender);
        if (ballot.votes.size() != ballot.lastVotes.size()) {
          return;
        }

        // the quorum has now voted; broadcast the success
        chamber().broadcast(make_shared<SuccessMessage>(id(), ballot.decree));
      }

      void Legislator::process(const SuccessMessage& success) {
        Lock lock(m_);
        
        CHECK(decree_.isBlank() || decree_ == success.decree);
        decree_ = success.decree;
      }
    }
  }
}