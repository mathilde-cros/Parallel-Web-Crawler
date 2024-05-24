CXX = g++
CXXFLAGS = -std=c++11 -g3 -Wall -pthread
LDFLAGS = -L/opt/homebrew/opt/curl/lib
CPPFLAGS = -I/opt/homebrew/opt/curl/include
LIBS = -lcurl

all: webcrawler webcrawler_parallel

clean:
	rm -f webcrawler webcrawler.o webcrawler_parallel webcrawler_parallel.o

webcrawler: webcrawler.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

webcrawler.o: webcrawler.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

webcrawler_parallel: webcrawler_parallel.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

webcrawler_parallel.o: webcrawler_parallel.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
