#include "Legislator.h"

namespace Paxos {

  Vote Legislator::getNullVote() const {
    return Vote::getNullVote(id_);
  }

}