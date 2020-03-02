#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "utils.hpp"

using std::vector;
using std::unique_lock;
using std::mutex;
using std::condition_variable;


class Rover {

	int solvedProblemCount = 0;
	int totalProblemCount;

	int status = ok;
	bool stuck = false;

	bool exploring = false;
	bool finished = false;

	vector<std::thread> wheelSystems;

	mutex mtx;
	condition_variable condVar;

	void lock() {
		while (!finished) {
			unique_lock<mutex> lck(mtx);
			// Wait until one of the wheels starts freewheeling.
			while (!finished && status != freewheeling)
			  condVar.wait(lck);
			if (finished) break;

			printf("Locking wheel...\n");
			updateStatus(lck, ok);
		}
	}

	void raise() {
		while (!finished) {
			unique_lock<mutex> lck(mtx);
			// Wait until one of the wheels is blocked.
			while (!finished && status != blocked)
				condVar.wait(lck);
			if (finished) break;

			printf("Raising wheel...\n");
			updateStatus(lck, ok);
		}
	}

	void lower() {
		while (!finished) {
			unique_lock<mutex> lck(mtx);
			// Wait until one of the wheels is sinking.
			while (!finished && status != sinking)
				condVar.wait(lck);
			if (finished) break;

			printf("Lowering wheel...\n");
			updateStatus(lck, ok);
		}
	}

	void earth() {
		while (!finished) {
			unique_lock<mutex> lck(mtx);
			// Wait until the rover encounters an unknown problem.
			while (!finished && status != unknown)
			  condVar.wait(lck);
			if (finished) break;

			printf("Passing control to Earth\n");
			updateStatus(lck, ok);
		}
	}

	// Random problem generator
	void generateProblem(int wheel) {
		while (solvedProblemCount < totalProblemCount) {
			unique_lock<mutex> lck(mtx);
			// Wait until the rover is actually moving.
			while (!exploring || stuck) condVar.wait(lck);
			if (finished) break;

			int nextProblem = getRandomInt(1, 4);
			updateStatus(lck, nextProblem);
		}
	}

	void updateStatus(unique_lock<mutex> &lck, int newStatus) {
		// lck.unlock();
		status = newStatus;

		// lck.lock();
		stuck = newStatus != ok;
		if (status == ok) {
			++solvedProblemCount;
			finished = solvedProblemCount == totalProblemCount;
		}

		sleep( getRandomInt(600, 999) );
		lck.unlock();

		condVar.notify_all();
	}

 public:

	// 6 wheels by default, all OK
	Rover(int wheels = 6) : wheelSystems(wheels) {
		for (int i = 0; i < wheels; ++i) {
			// The state of each wheel has to be monitored by a sensor.
			wheelSystems[i] = wheelSensor(i);
		}

		// Enable remote-control via communication with Earth.
		wheelSystems.push_back( earthComms() );

		// Add a separate controller for each possible state
		wheelSystems.push_back( freewheelingController() );
		wheelSystems.push_back( blockedWheelController() );
		wheelSystems.push_back( sinkingWheelController() );
	}

	~Rover() {
		// Forcefully terminate threads that are not finished.
		for (auto &system : wheelSystems) {
			system.~thread();
		}
	}

	void explore() {
		// Set a random amount of problems to be encountered.
		totalProblemCount = getRandomInt(5, 8);

		// Start moving around.
		exploring = true;

		// Wait for the exploration to finish by syncrhonizing the threads.
		for (auto &system : wheelSystems) {
			system.join();
		}
		printf("Finished\n" );
	}

	// A sensor consists of a separate thread running a random problem generator.
	auto wheelSensor(int wheel) -> std::thread {
		return std::thread(&Rover::generateProblem, this, wheel);
	}
	auto earthComms() -> std::thread {
		return std::thread(&Rover::earth, this);
	}
	auto freewheelingController() -> std::thread {
		return std::thread(&Rover::lock, this);
	}
	auto blockedWheelController() -> std::thread {
		return std::thread(&Rover::raise, this);
	}
	auto sinkingWheelController() -> std::thread {
		return std::thread(&Rover::lower, this);
	}

};
