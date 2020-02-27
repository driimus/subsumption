#pragma once

#include <random>
#include <chrono>
#include <thread>

enum status: int {
	ok = 0,
	blocked,		// raise
	freewheeling,	// lock
	sinking,		// lower
	unknown			// earth
};

auto getRandomInt(int from, int to) -> int {
	std::uniform_int_distribution<int> range{from, to};
	std::random_device rnd;
	return range(rnd);
}

void sleep(int ms) {
	std::this_thread::sleep_for( std::chrono::milliseconds(ms) );
}
