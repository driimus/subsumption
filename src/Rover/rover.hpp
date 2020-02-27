#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class Rover {

	bool finished = false;
	bool stuck = false;

	std::vector<int> wheelStatus;
	std::vector<std::vector<std::thread>> wheelSystems;

	std::mutex mtx;
	std::condition_variable condVar;

	void lock(const int wheel);
	void raise(const int wheel);
	void lower(const int wheel);
	void earth(const int wheel);

	// Random problem generator
	void generateProblem(const int wheel);

public:

	// 6 wheels by default, all OK
	Rover(int wheels = 6);
	~Rover();

	// A sensor consists of a random problem generator.
	auto wheelSensor(int wheel) -> std::thread;
	auto earthComms(int wheel) -> std::thread;
	auto freewheelingController(int wheel) -> std::thread;
	auto blockedWheelController(int wheel) -> std::thread;
	auto sinkingWheelController(int wheel) -> std::thread;
};
