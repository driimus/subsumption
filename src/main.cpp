#include "rover.hpp"
#include <thread>

auto main() -> int {
	Logger log;
	Rover rover { log };
	std::vector<std::thread> problems;

	log << "Starting rover...";

	// Log final rover status
	log << rover.isStuck() ? "Reached destination." : "Could not reach destination";

	return 0;
}
