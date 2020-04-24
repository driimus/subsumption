CXX = g++
CXXFLAGS = -std=c++11 -pthread -Wall -O3

#----------

EXES = main

all: $(EXES)

#----------

%: %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

#----------

clean:
	rm -f $(EXES)
