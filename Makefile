CXX = g++ 
CXXFLAGS = -std=c++11 -g3 -Wall -pthread -o webcrawler -l curl
export LDFLAGS="-L/opt/homebrew/opt/curl/lib"
export CPPFLAGS="-I/opt/homebrew/opt/curl/include"

all: webcrawler

clean:
		rm -f webcrawler webcrawler.o
		rm -f webcrawlerjas webcrawlerjas.o

webcrawler: webcrawler.o
		$(CXX) $(CXXFLAGS) -o $@ $^

webcrawler.o: webcrawler.cpp
		$(CXX) $(CXXFLAGS) -c -o $@ $<

webcrawlerjas: webcrawlerjas.o
		$(CXX) $(CXXFLAGS) -o $@ $^

webcrawlerjas.o: webcrawler2.cpp
		$(CXX) $(CXXFLAGS) -c -o $@ $<
