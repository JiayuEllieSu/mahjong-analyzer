CXX = c++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic

.PHONY: all test clean web

all: mahjong

mahjong: main.cpp hand_analyzer.hpp
	$(CXX) $(CXXFLAGS) main.cpp -o mahjong

tests/test_analyzer: tests/test_analyzer.cpp hand_analyzer.hpp
	$(CXX) $(CXXFLAGS) tests/test_analyzer.cpp -o tests/test_analyzer

test: mahjong tests/test_analyzer
	./tests/test_analyzer
	bash tests/test_cli.sh
	python3 -m unittest discover -s tests -p 'test_web.py'

web: mahjong
	python3 web_server.py

clean:
	rm -f mahjong tests/test_analyzer
