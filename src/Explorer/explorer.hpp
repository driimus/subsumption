#include <random>
#include <chrono>

#include <vector>

#include <thread>
#include <mutex>
#include <condition_variable>


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
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

class Rover {

	static const int WHEELS = 6;
	// 6 wheels, all OK by default
	int wheelStatus[WHEELS] = {ok};
	std::vector<std::vector<std::thread>> wheelControllers;

	bool finished = false;
	bool stuck = false;

	std::mutex mtx;
	std::condition_variable condVar;

	void lock(const int wheel) {
		while(!finished) {
			std::unique_lock<std::mutex> lck(mtx);
			while (wheelStatus[wheel] != freewheeling) condVar.wait(lck);
			lck.unlock();

			wheelStatus[wheel] = ok;
			sleep(500);

			lck.lock();
			stuck = false;
			lck.unlock();

			condVar.notify_all();

			printf("Locking wheel %d...\n", wheel+1);
		}
	}

	void raise(const int wheel) {
		while(!finished) {
			std::unique_lock<std::mutex> lck(mtx);
			while (wheelStatus[wheel] != blocked) condVar.wait(lck);
			lck.unlock();

			sleep(300);
			wheelStatus[wheel] = ok;

			lck.lock();
			stuck = false;
			lck.unlock();

			condVar.notify_all();

			printf("Raising wheel %d...\n", wheel+1);
		}
	}

	void lower(const int wheel) {
		while(!finished) {
			std::unique_lock<std::mutex> lck(mtx);
			while (wheelStatus[wheel] != sinking) condVar.wait(lck);
			lck.unlock();

			sleep(700);
			wheelStatus[wheel] = ok;

			lck.lock();
			stuck = false;
			lck.unlock();

			condVar.notify_all();

			printf("Lowering wheel %d...\n", wheel+1);
		}
	}

	void earth(const int wheel) {
		while(!finished) {
			std::unique_lock<std::mutex> lck(mtx);
			while (wheelStatus[wheel] != unknown) condVar.wait(lck);
			lck.unlock();

			wheelStatus[wheel] = ok;

			lck.lock();
			stuck = false;
			lck.unlock();

			condVar.notify_all();

			printf("Passing control to Earth\n");
		}
	}

	// Random problem generator
	void generateProblem(const int wheel) {
		while (!finished) {
			std::unique_lock<std::mutex> lck(mtx);

			// Wait until the rover solves the current problem if already stuck
			while (stuck) condVar.wait(lck);
			lck.unlock();

			int problem = getRandomInt(1, 3);
			wheelStatus[wheel] = problem;

			lck.lock();
			stuck = true;
			lck.unlock();

			condVar.notify_all();
		}
	}

public:

	auto monitorWheel(const int wheel) -> std::thread {
		return std::thread(&Rover::generateProblem, this, wheel);
	}
	auto lockWheel(const int wheel) -> std::thread {
		return std::thread(&Rover::lock, this, wheel);
	}
	auto raiseWheel(const int wheel) -> std::thread {
		return std::thread(&Rover::raise, this, wheel);
	}
	auto lowerWheel(const int wheel) -> std::thread {
		return std::thread(&Rover::lower, this, wheel);
	}
	auto passControlToEarth() -> std::thread {
		return std::thread(&Rover::earth, this, wheel);
	}

	Rover() {
		// LOG Start

		// Initialize each wheel on a separate thread.
		for (int i=0; i<WHEELS; ++i) {
			wheelControllers.push_back({});
			// A well constitues of a random problem generator.
			wheelControllers[i].push_back(monitorWheel(i));

			// Add a separate controller for each possible state
			wheelControllers[i].push_back(lockWheel(i));
			wheelControllers[i].push_back(raiseWheel(i));
			wheelControllers[i].push_back(lowerWheel(i));
			wheelControllers[i].push_back(passControlToEarth(i));
		}
		for (auto &wheel: wheelControllers) {
			for (auto &controller: wheel)
				controller.join();
		}
	}

};
