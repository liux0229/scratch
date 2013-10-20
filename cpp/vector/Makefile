CXX=ccache g++
CPPFLAGS=-Wall -g -std=gnu++11

VectorTestObjs=VectorTest.o

all: clean vectortest

vectortest: $(VectorTestObjs)
	$(CXX) $(CPPFLAGS) -o vector_test $(VectorTestObjs)
	./vector_test

clean:
	rm -f *.o vector_test
