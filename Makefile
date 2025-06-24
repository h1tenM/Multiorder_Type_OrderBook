CXX := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic

Orderbook: Orderbook.cpp
	$(CXX) $(CXXFLAGS) Orderbook.cpp -o Orderbook

clean:
	rm -f Orderbook