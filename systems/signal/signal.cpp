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

  // cout << "a[1] = " << *(pa + 1) << endl;
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
  volatile int64_t a[2]{};
  pa = a;
  cout << (int64_t*)pa << endl;

  installSignal();

  size_t total = 0;
  size_t equal = 0;

  while (true) {
    ++a[1];
    if (a[0] == 1) {
      a[0] = 0;
      // cout << "After signal: i=" << a[1] << endl;
      ++total;
      equal += a[1] == observed;
    } else if (a[0] == 2) {
      break;
    }
  }

  cout << "total: " << total << " equal: " << 100.0 * equal / total << "%\n";
}

int main() {
  f();
}
