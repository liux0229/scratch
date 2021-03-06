#pragma once

#include "Common.h"

namespace Paxos {
  struct LegislatorId {
    explicit LegislatorId(int i) : id(i) {}
    int id;
  };
  inline bool operator<(LegislatorId a, LegislatorId b) {
    return a.id < b.id;
  }
  std::ostream& operator<<(std::ostream& out, LegislatorId id);

  struct BallotNumber {
    int64_t number;
    LegislatorId legislatorId;
  };
  inline bool operator<(const BallotNumber& a, const BallotNumber& b) {
    // TODO: what happens if we check legislator id first?
    return a.number < b.number ||
      a.number == b.number && a.legislatorId < b.legislatorId;
  }
  std::ostream& operator<<(std::ostream& out, BallotNumber ballot);

  struct Decree {
    Decree() = default;
    Decree(int64_t v) : value(v) {}
    bool isBlank() const { return value == -1;  }

    int64_t value{ -1 };
  };
  inline bool operator==(const Decree& a, const Decree& b) {
    return a.value == b.value;
  }
  std::ostream& operator<<(std::ostream& out, Decree decree);

  struct Vote {
    static Vote getNullVote(LegislatorId legislator);
    bool isNull() const;

    LegislatorId legislator;
    BallotNumber ballot;
    Decree decree;
  };
  inline bool operator<(const Vote& a, const Vote& b) {
    // votes with the same ballot number will compare equally, which is fine
    return a.ballot < b.ballot;
  }
  std::ostream& operator<<(std::ostream& out, const Vote& vote);

  class Legislator;
  struct Message {
    Message(LegislatorId s) : sender(s) { }
    virtual ~Message() {}

    virtual void deliver(Legislator& legislator) const = 0;
    virtual void output(std::ostream& out) const = 0;

    LegislatorId sender;
  };
  using SMessage = std::shared_ptr<Message>;

  inline std::ostream& operator<<(std::ostream& out, const Message& msg) {
    msg.output(out);
    return out;
  }
}