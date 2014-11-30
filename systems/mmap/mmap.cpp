#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>

using namespace std;

void doRead(string f) {
  int fd = open(f.c_str(), 0);

  struct stat s;
  fstat(fd, &s);
  auto size = s.st_size;

  auto* buf = (char*)mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
  for (int i = 0; i < size; ++i) {
    cout << buf[i];
  }
  munmap(buf, size);
  close(fd);
}

int main(int argc, const char** argv) {
  string file = argv[1]; 
  doRead(file);
}
