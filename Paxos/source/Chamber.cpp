#include "Chamber.h"

#include <chrono>

namespace Paxos {
  using namespace std;
  using namespace std::chrono;

  namespace {
    const double MessageLossProbability{ 0.005 };
    const milliseconds MessageDelay{ 500 };
    const size_t MessageQueueMaxSize{ 500 };
  }

  Chamber::Chamber() : messages_{ MessageQueueMaxSize } { }

  Chamber::~Chamber() {
    messages_.enque(MessageInfo{ LegislatorId{-1}, nullptr });
    dispatcher_.join();
  }

  Legislator& Chamber::getLegislator(LegislatorId id) {
    auto it = legislators_.find(id);
    if (it == legislators_.end()) {
      Throw("{} is not present in the chamber", id);
    }
    return *it->second;
  }

  bool Chamber::isMajority(size_t n) const {
    return n > legislators_.size() / 2;
  }

  void Chamber::add(ULegislator legislator) {
    auto id = legislator->id();
    legislators_.insert(make_pair(id, move(legislator)));
  }

  void Chamber::broadcast(SMessage msg) {
    for (auto& kv : legislators_) {
      send(kv.first, msg);
    }
  }

  void Chamber::send(LegislatorId receiver, SMessage msg) {
    if (messageLost()) {
      cout << format("{} lost; was delivered to {}", *msg, receiver) << endl;
      return;
    }

    // add a random message delivery delay; do not block the sender
    auto delay = milliseconds{ random_.getInt(0LL, MessageDelay.count()) };

    MessageInfo message{receiver, msg};
    auto doSend = [this, message, delay]() {
      this_thread::sleep_for(delay);
      // cout << format("enqueue {}\n", *message.payload);
      messages_.enque(message);
    };
    thread{ doSend }.detach();
  }

  void Chamber::start() {
    dispatcher_ = thread{&Chamber::dispatch, this};
  }

  void Chamber::dispatch() {
    while (true) {
      MessageInfo message = messages_.deque();
      if (message.isStopper()) {
        break;
      }
      deliver(message.receiver, message.payload);
    }
  }

  void Chamber::deliver(LegislatorId receiver, SMessage msg) {
    auto& target = getLegislator(receiver);
    cout << format("Delivered to {} {}", receiver, *msg) << endl;
    msg->deliver(target);
  }

  bool Chamber::messageLost() {
    return random_.getReal(0.0, 1.0) < MessageLossProbability;
  }
}