#pragma once

struct Decree {
  size_t value;
};

class Legislator;
struct Message {
  virtual ~Message();
  virtual void deliver(Legislator& legislator) = 0;
};

struct NextBallotMessage : Message {
  virtual void deliver(Legislator& legislator);
};

struct LastVoteMessage : Message {
  virtual void deliver(Legislator& legislator);
};

struct BeginBallotMessage : Message {
  virtual void deliver(Legislator& legislator);
};

struct VotedMessage : Message {
  virtual void deliver(Legislator& legislator);
};

struct SuccessMessage : Message {
  virtual void deliver(Legislator& legislator);
};

class Chamber;

class Legislator {
public:
  Legislator(Chamber& chamber) : chamber_(chamber) {
  }

  void process(const NextBallotMessage& nextBallot);
  void process(const LastVoteMessage& lastVote);
  void process(const BeginBallotMessage& beginBallot);
  void process(const VotedMessage& voted);
  void process(const SuccessMessage& success);

private:
  Chamber& chamber_;
};
