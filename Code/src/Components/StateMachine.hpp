#pragma once
#include <KPFoundation.hpp>
#include <KPStateMachine.hpp>
#include <KPState.hpp>

class StateMachine : public KPStateMachine {
public:
	const char * entryStateName	   = nullptr;
	const char * stopStateName	   = nullptr;
	const char * idleStateName	   = nullptr;
	const char * finishedStateName = nullptr;
	int current_cycle			   = 0;
	int last_cycle				   = 24;
	void reset() {
		current_cycle = 0;
	}
	void halt() {
		current_cycle = last_cycle;
		stop();
	}
	StateMachine(const char * name, const char * entryStateName, const char * stopStateName,
		const char * idleStateName, const char * finishedStateName)
		: KPStateMachine(name),
		  entryStateName(entryStateName),
		  stopStateName(stopStateName),
		  idleStateName(idleStateName),
		  finishedStateName(finishedStateName) {}

	void begin() {
		transitionTo(entryStateName);
	}
	void stop() {
		transitionTo(stopStateName);
	}
	void idle() {
		transitionTo(idleStateName);
	}

	void setup() override {
		Serial.print("StateMachine setup");
	}

	const char * getCurrentStateName() {
		return getCurrentState()->getName();
	}

	bool isBusy() const {
		return !(getCurrentState() == nullptr
			|| 0 == strcmp(getCurrentState()->getName(), finishedStateName));
	}

	bool isRunning() const {
		return (isBusy() && 0 != strcmp(getCurrentState()->getName(), idleStateName));
	}
};