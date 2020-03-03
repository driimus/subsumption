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

	int encounteredProblemCount = 0;
	int solvedProblemCount = 0;
	int totalProblemCount;

	int status = ok;
	bool moving = false;
	bool finished = false;


	vector<std::thread> wheelSystems;

	mutex mtx;
	condition_variable condVar;

	void act(int targetIssue) {

		while (!finished) {
			unique_lock<mutex> lck(mtx);
			// Wait until the rover encounters a given issue.
			while (!finished && status != targetIssue)
				condVar.wait(lck);
			if (finished) break;

			printf("%s\n", actionMessage[targetIssue].c_str());
			updateStatus(lck, ok);
		}
	}

	// Random problem generator
	void generateProblem(int wheel) {
		while (solvedProblemCount < totalProblemCount) {
			unique_lock<mutex> lck(mtx);
			// Wait until the rover is actually moving (not stuck or idle).
			while (!moving) condVar.wait(lck);
			if (finished) break;

			int nextProblem = getRandomInt(0, 3);
			updateStatus(lck, nextProblem);
		}
	}

	void updateStatus(unique_lock<mutex> &lck, int newStatus) {
		status = newStatus;
		moving = status == ok;

		// Check if we've fixed all the problems.
		if (status == ok) {
			++solvedProblemCount;
			finished = solvedProblemCount == totalProblemCount;
		}

		// Random delay of ~1 second.
		sleep( getRandomFloat(0.7, 1.5) );
		lck.unlock();

		condVar.notify_all();
	}

 public:

	// 6 wheels by default
	Rover(int wheels = 6) : wheelSystems(wheels) {
		// Attach a dedicated sensor for each wheel.
		for (int i = 0; i < wheels; ++i) {
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
		for (auto &system : wheelSystems) { system.~thread(); }
	}

	void explore() {
		unique_lock<mutex> lck(mtx);
		// Set a random amount of problems to be encountered.
		totalProblemCount = getRandomInt(5, 8);

		// Start exploring and wait until the rover finishes.
		moving = true;
		condVar.notify_all();
		lck.unlock();
		for (auto &system : wheelSystems) { system.join(); }

		printf("Finished\n" );
	}

	// A sensor consists of a separate thread running a random problem generator.
	auto wheelSensor(int wheel) -> std::thread {
		return std::thread(&Rover::generateProblem, this, wheel);
	}
	// Separate thread for asking Earth to help when a problem can't be solved autonomously.
	auto earthComms() -> std::thread {
		return std::thread(&Rover::act, this, askEarthForAssitance);
	}
	// Separate thread for solving to free-wheeling issues by locking the wheel.
	auto freewheelingController() -> std::thread {
		return std::thread(&Rover::act, this, lockWheel);
	}
	// Separate thread for solving blocked wheel issues by raising the wheel.
	auto blockedWheelController() -> std::thread {
		return std::thread(&Rover::act, this, raiseWheel);
	}
	// Separate thread for solving sinking wheel issues by lowering the wheel.
	auto sinkingWheelController() -> std::thread {
		return std::thread(&Rover::act, this, lowerWheel);
	}

};
