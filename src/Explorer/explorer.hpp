
#include <vector>

#include <thread>
#include <mutex>
#include <condition_variable>

#include "utils.hpp"

class Rover {

	// 6 wheels, all OK by default
	static const int WHEELS = 6;
	int wheelStatus[WHEELS] = {ok};
	bool finished = false;
	bool stuck = false;

	std::vector<std::vector<std::thread>> wheelSystems;

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

	// A sensor consists of a random problem generator.
	auto wheelSensor(const int wheel) -> std::thread {
		return std::thread(&Rover::generateProblem, this, wheel);
	}
	auto freewheelingController(const int wheel) -> std::thread {
		return std::thread(&Rover::lock, this, wheel);
	}
	auto blockedWheelController(const int wheel) -> std::thread {
		return std::thread(&Rover::raise, this, wheel);
	}
	auto sinkingWheelController(const int wheel) -> std::thread {
		return std::thread(&Rover::lower, this, wheel);
	}
	auto passControlToEarth(const int wheel) -> std::thread {
		return std::thread(&Rover::earth, this, wheel);
	}

	Rover() {
		// LOG Start

		// Initialize each wheel on a separate thread.
		for (int i=0; i<WHEELS; ++i) {
			wheelSystems.push_back({});
			// The state of each wheel has to be monitored by a sensor.
			wheelSystems[i].push_back(wheelSensor(i));

			// Add a separate controller for each possible state
			wheelSystems[i].push_back(freewheelingController(i));
			wheelSystems[i].push_back(blockedWheelController(i));
			wheelSystems[i].push_back(sinkingWheelController(i));

			wheelSystems[i].push_back(passControlToEarth(i));
		}
	}

	~Rover() {
		for (auto &wheel: wheelSystems) {
			for (auto &controller: wheel)
				controller.join();
		}
	}

};
