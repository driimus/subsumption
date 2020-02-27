#include "utils.hpp"
#include "rover.hpp"

void Rover::lock(const int wheel) {
	while (!finished) {
		std::unique_lock<std::mutex> lck(mtx);
		while (wheelStatus[wheel] != freewheeling) condVar.wait(lck);
		lck.unlock();

		wheelStatus[wheel] = ok;
		sleep(500);

		lck.lock();
		stuck = false;
		lck.unlock();

		condVar.notify_all();

		printf("Locking wheel %d...\n", wheel + 1);
	}
}

void Rover::raise(const int wheel) {
	while (!finished) {
		std::unique_lock<std::mutex> lck(mtx);
		while (wheelStatus[wheel] != blocked) condVar.wait(lck);
		lck.unlock();

		sleep(300);
		wheelStatus[wheel] = ok;

		lck.lock();
		stuck = false;
		lck.unlock();

		condVar.notify_all();

		printf("Raising wheel %d...\n", wheel + 1);
	}
}

void Rover::lower(const int wheel) {
	while (!finished) {
		std::unique_lock<std::mutex> lck(mtx);
		while (wheelStatus[wheel] != sinking) condVar.wait(lck);
		lck.unlock();

		sleep(700);
		wheelStatus[wheel] = ok;

		lck.lock();
		stuck = false;
		lck.unlock();

		condVar.notify_all();

		printf("Lowering wheel %d...\n", wheel + 1);
	}
}

void Rover::earth(const int wheel) {
	while (!finished) {
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
void Rover::generateProblem(const int wheel) {
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
		sleep(1500);
	}
}

// A sensor consists of a random problem generator.
auto Rover::wheelSensor(int wheel) -> std::thread {
	return std::thread(&Rover::generateProblem, this, wheel);
}
auto Rover::earthComms(int wheel) -> std::thread {
	return std::thread(&Rover::earth, this, wheel);
}
auto Rover::freewheelingController(int wheel) -> std::thread {
	return std::thread(&Rover::lock, this, wheel);
}
auto Rover::blockedWheelController(int wheel) -> std::thread {
	return std::thread(&Rover::raise, this, wheel);
}
auto Rover::sinkingWheelController(int wheel) -> std::thread {
	return std::thread(&Rover::lower, this, wheel);
}


// 6 wheels by default, all OK
Rover::Rover(int wheels) : wheelSystems(wheels), wheelStatus(wheels, ok) {
	for (int i = 0; i < wheels; ++i) {
		// The state of each wheel has to be monitored by a sensor.
		wheelSystems[i].push_back( wheelSensor(i) );

		// Enable remote-control via communication with Earth.
		wheelSystems[i].push_back( earthComms(i) );

		// Add a separate controller for each possible state
		wheelSystems[i].push_back( freewheelingController(i) );
		wheelSystems[i].push_back( blockedWheelController(i) );
		wheelSystems[i].push_back( sinkingWheelController(i) );
	}
}

Rover::~Rover() {
	for (auto &wheel : wheelSystems) {
		for (auto &controller : wheel)
			controller.join();
	}
}
