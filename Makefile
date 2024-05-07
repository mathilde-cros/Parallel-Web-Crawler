CXX = g++ 
CXXFLAGS = -g3 -Wall -pthread -Xpreprocessor -fopenmp -Ofast

all: webcrawler

clean:
		rm -f webcrawler webcrawler.o

webcrawler: webcrawler.o
		$(CXX) $(CXXFLAGS) -o $@ $^

webcrawler.o: webcrawler.cpp
		$(CXX) $(CXXFLAGS) -c -o $@ $<
