// FLAGS: -O3

#include <iostream>
#include <signal.h>

volatile int64_t *pa;
volatile int64_t observed;
using namespace std;

void handler(int signal) {
  // int64_t x;
  // int64_t* px = &x;
  // cout << px << endl;
  // cout << "diff = " << pa - px << endl;

  cout << "i = " << *(pa + 1) << endl;
  observed = *(pa + 1);
  *pa = signal == SIGINT ? 1 : 2;
}

void installSignal() {
  struct sigaction action{};
  action.sa_handler = &handler;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, nullptr);
  sigaction(SIGTERM, &action, nullptr);
}

void f() {
  volatile int64_t a = 0;
  // note: this must be volatile to force the compiler to actually update the
  // memory for i. There is no guarantee that this would happen even if we
  // take the address of i.
  int64_t i = 0;

  pa = &a;

  cout << (int64_t*)pa << endl;

  installSignal();

  size_t total = 0;
  size_t equal = 0;

  while (true) {
    ++i;
    if (a == 1) {
      a = 0;
      cout << "After signal: i=" << i << endl;
      ++total;
      equal += i == observed;
    } else if (a == 2) {
      break;
    }
  }

  cout << "total: " << total << " equal: " << 100.0 * equal / total << "%\n";
}

int main() {
  f();
}
