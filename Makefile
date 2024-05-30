CXX = g++
CXXFLAGS = -std=c++11 -g3 -Wall -pthread
LIBS = -lcurl

all: webcrawler webcrawler_parallel

clean:
	rm -f webcrawler webcrawler.o 
	rm -f webcrawler_parallel webcrawler_parallel.o

webcrawler: webcrawler.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

webcrawler.o: webcrawler.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

webcrawler_parallel: webcrawler_parallel.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

webcrawler_parallel.o: webcrawler_parallel.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<