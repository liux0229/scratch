// FLAGS: -O3 -g

#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <cstring>

volatile int64_t *pa;
volatile int64_t observed;
using namespace std;

void handler(int signal) {
  int64_t x;
  int64_t* px = &x;
  // cout << px << endl;

  // This shows the handler is executed below the stack framek of f() even if it
  // makes a system call.
  cout << "diff = " << pa - px << endl;

  cout << (void *)*(pa + 4) << endl;

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
  volatile int64_t i = 0;

  pa = &a;

  cout << "pa=" << (int64_t*)pa << endl;

  installSignal();

#if 0
  {
    int64_t x;
    asm ("call L2; L2: addq %0, %0" 
         : "=r"(x));
    cout << "n=" << (void *)x << endl;
    // This is &a + 6 (if the call above does pop). 
    // Who pushed the address there?
    cout << "a=" << (void *)a << endl;
  }
#endif

  int64_t n;
  asm ("call L1; L1: pop %0"
       : "=r" (n));
  cout << "&f=" << (void *)f << endl;
  cout << "n=" << (void *)n << endl;
  char buf[1024];
  auto ret = read(0, buf, 1024);
  cout << "read returned: " << ret << endl;
  if (ret < 0) {
    cout << "error: " << strerror(errno) << endl;
  }

  size_t total = 0;
  size_t equal = 0;

  while (true) {
    ++i;
    if (a) {
      cout << "a=" << (void *)a << endl;
      if (a == 1) {
        cout << "After signal: i=" << i << endl;
        ++total;
        equal += i == observed;
      } else if (a == 2) {
        break;
      }
      a = 0;
    }
  }

  cout << "total: " << total << " equal: " << 100.0 * equal / total << "%\n";
}

int main() {
  f();
}
