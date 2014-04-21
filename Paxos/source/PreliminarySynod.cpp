#include "PreliminarySynod.h"

#include <chrono>

namespace Paxos {
  using namespace std;
  using namespace std::chrono;
  namespace SP = Paxos::Synod::Preliminary;

  namespace {
    const size_t SimulateIterations = 50;
  }

  PreliminarySynod::PreliminarySynod(size_t n) {
    for (size_t i = 0; i < n; ++i) {
      chamber_.add(make_unique<SP::Legislator>(
                     LegislatorId{static_cast<int>(i)}, 
                     chamber_));
    }
  }

  void PreliminarySynod::start() {
    chamber_.start();

    for (size_t i = 0; i < SimulateIterations; ++i) {
      auto x = random_.getInt<int32_t>(0, static_cast<int32_t>(chamber_.totalLegislators() - 1));
      LegislatorId id{ x };
      auto& legislator = dynamic_cast<SP::Legislator&>(chamber_.getLegislator(id));
      
      cout << format("{} initiates balloting", id) << endl;
      legislator.initiateBalloting([i]() { return i; });

      this_thread::sleep_for(milliseconds(500));
    }

    this_thread::sleep_for(seconds(10));
  }
}