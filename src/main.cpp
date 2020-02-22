#include "rover.hpp"
#include <thread>

auto main() -> int {
	Logger log;
	Rover rover { log };
	std::vector<std::thread> problems;

	log << "Starting rover...";
	// start a new thread for exploring with the rover
	std::thread explorer(&Rover::explore, &rover);

	// start between 5-10 threads, each generating a problem

	int PROBLEM_COUNT = 5;		// in range [5-10)
	for(int i; i<PROBLEM_COUNT; ++i) {
		std::thread problem(&Rover::genDelayedProblem, &rover);
		problems.push_back(problem);
	}

	for (auto prob: problems) prob.join();
	explorer.join();

	// Log final rover status
	log << rover.isStuck() ? "Reached destination." : "Could not reach destination";

	return 0;
}
