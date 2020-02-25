enum state: int {
	ok = 0,
	blocked,		// raise
	freewheeling,	// lock
	sinking,		// lower
};

class Rover {

	// 6 wheels, all OK by default
	int wheelStatus[6] = {ok};
	std::vector<std::thread> wheels;

	bool finished = false;
	bool stuck = false;

	// Random problem generator
	void genProblem(int affectedWheel) {
		std::unique_lock<std::mutex> lck(mtx);

		// Wait until the rover is not stuck (problems must not occur in parallel)
		while (stuck) condVar.wait(lck);

		int problem = ;			// random in range [1, 4)
		wheelStatus[affectedWheel] = problem;

		// LOG < problems
		// solveProblems
		// LOG < problem < action
	}

public:

	Rover() {
		// LOG Start

		// Initialize a new thread for each wheel
		for (int i=0; i<wheelStatus.size(); ++i) {
			wheels.push_back(std::thread(genProblem, i));
		}
		for (auto wheel: wheels) {
			wheel.join();
		}
	}

}


void explore() {
	// LOG End
}
