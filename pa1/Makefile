CXX = mpic++
CXXFLAGS = -std=c++11
SRC = main.cpp
EXEC = pi_calc

all: $(EXEC)

$(EXEC): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(EXEC)

.PHONY: all clean