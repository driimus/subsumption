#include "utils.hpp"
#include "rover.hpp"

void Rover::lock(const int wheel) {
	while (!finished) {
		u_lock lck(mtx);
		while (wheelStatus[wheel] != freewheeling) condVar.wait(lck);

		printf("Locking wheel %d...\n", wheel + 1);
		updateWheelStatus(lck, wheel, ok);
	}
}

void Rover::raise(const int wheel) {
	while (!finished) {
		u_lock lck(mtx);
		while (wheelStatus[wheel] != blocked) condVar.wait(lck);

		printf("Raising wheel %d...\n", wheel + 1);
		updateWheelStatus(lck, wheel, ok);
	}
}

void Rover::lower(const int wheel) {
	while (!finished) {
		u_lock lck(mtx);
		while (wheelStatus[wheel] != sinking) condVar.wait(lck);

		printf("Lowering wheel %d...\n", wheel + 1);
		updateWheelStatus(lck, wheel, ok);
	}
}

void Rover::earth(const int wheel) {
	while (!finished) {
		u_lock lck(mtx);
		while (wheelStatus[wheel] != unknown) condVar.wait(lck);

		printf("Passing control to Earth\n");
		updateWheelStatus(lck, wheel, ok);
	}
}

// Random problem generator
void Rover::generateProblem(const int wheel) {
	while (!finished) {
		u_lock lck(mtx);
		// Wait until the rover solves the current problem if already stuck
		while (stuck) condVar.wait(lck);

		int nextProblem = getRandomInt(1, 3);
		updateWheelStatus(lck, wheel, nextProblem);
	}
}

void Rover::updateWheelStatus(u_lock &lck, int wheel, int newStatus) {
	lck.unlock();
	wheelStatus[wheel] = newStatus;

	lck.lock();
	stuck = newStatus != ok;
	lck.unlock();

	condVar.notify_all();
	sleep(getRandomInt(300, 999));
}


// A sensor consists of a random problem generator.
auto Rover::wheelSensor(int wheel) -> std::thread {
	return std::thread(&Rover::generateProblem, this, wheel);
}
auto Rover::earthComms() -> std::thread {
	return std::thread(&Rover::earth, this);
}
auto Rover::freewheelingController() -> std::thread {
	return std::thread(&Rover::lock, this);
}
auto Rover::blockedWheelController() -> std::thread {
	return std::thread(&Rover::raise, this);
}
auto Rover::sinkingWheelController() -> std::thread {
	return std::thread(&Rover::lower, this);
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
	for (auto &wheel : wheelSystems) {
		for (auto &controller : wheel)
			controller.join();
	}
}

Rover::~Rover() {
}
