CXX = mpic++
CXXFLAGS = -std=c++11 -O3
SRC = bonus.cpp
EXEC = bonus

all: $(EXEC)

$(EXEC): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(EXEC)

.PHONY: all clean