#pragma once

#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

enum status: int {
	// idle = -2,
	ok = -1,
	blocked,		// raise
	freewheeling,	// lock
	sinking,		// lower
	unknown			// earth
};

std::vector<std::string> statusMessage = {
	"is blocked.",
	"is freewheeling.",
	"is sinking.",
	"encountered an unknown problem."
};

enum action: int {
	raiseWheel = 0,
	lockWheel,
	lowerWheel,
	askEarthForAssitance
};

std::vector<std::string> actionMessage = {
	"Raising wheel...",
	"Locking wheel...",
	"Lowering wheel...",
	"Asking Earth for assistance..."
};

auto getRandomInt(int from, int to) -> int {
	std::uniform_int_distribution<int> range{from, to};
	std::random_device rnd;
	return range(rnd);
}

auto getRandomFloat(float from, float to) -> float {
	std::uniform_real_distribution<float> range{from, to};
	std::random_device rnd;
	return range(rnd);
}

void sleep(float sec) {
	int ms = sec * 1000;
	std::this_thread::sleep_for( std::chrono::milliseconds(ms) );
}
