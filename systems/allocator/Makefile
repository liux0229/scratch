CXX = ccache g++
CXXFLAGS = -g -Wall -isystem ~/dev/gtest/include -std=c++11 -pthread

# CXXFLAGS += -DENABLE_VERIFY
CXXFLAGS += -O3

OBJ=Allocator.o AllocatorTest.o

allocator_test: clean $(OBJ)
	$(CXX) $(CXXFLAGS) -o allocator_test $(OBJ) ~/dev/gtest/bin/gtest_main.a

test: allocator_test
	./allocator_test

clean:
	rm -f *.o allocator_test

