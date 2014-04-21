#include "PreliminarySynod.h"

int main() {
  Paxos::PreliminarySynod synod(50);
  synod.start();
}