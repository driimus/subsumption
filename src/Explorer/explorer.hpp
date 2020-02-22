enum state : int{
	ok = 0,
	blocked,		// raise
	freewheeling,	// lock
	sinking,		// lower
};

class Rover {

	// 6 wheels, all OK by default
	int wheels[6] = {ok};
	bool finished = false;

public:
	Rover() {
		// needs a thread for each wheel
	}

	// to be used by separate threads (5 to 10), each calling with a random delay
	void genProblem() {
		int problem = ;			// random in range [1, 4)
		wheels[affectedWheel] = problem;
	}

	void genDelayedProblem() {
		// sleep for random amount of time

		genProblem();
	}
}


void explore() {
	// LOG Start
	for(int section=0; section<5; ++section) {
		// LOG < section
		// getProblems
		// LOG < problems
		// solveProblems
			// LOG < problem < action
	}
	// LOG End
}
