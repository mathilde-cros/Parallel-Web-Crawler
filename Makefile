CXX = g++ 
CXXFLAGS = -g3 -Wall -pthread -L/opt/homebrew/opt/libomp/lib -I/opt/homebrew/opt/libomp/include -Xpreprocessor -fopenmp -lomp -Ofast

all: webcrawler

clean:
		rm -f webcrawler webcrawler.o

webcrawler: webcrawler.o
		$(CXX) $(CXXFLAGS) -o $@ $^

webcrawler.o: webcrawler.cpp
		$(CXX) $(CXXFLAGS) -c -o $@ $<
