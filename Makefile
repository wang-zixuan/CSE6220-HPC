CXX = mpic++
CXXFLAGS = -std=c++11 -O3
SRC = main.cpp
EXEC = spmat

all: $(EXEC)

$(EXEC): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(EXEC)

.PHONY: all clean