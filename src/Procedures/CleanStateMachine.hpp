#pragma once

#include <Components/StateMachine.hpp>
#include <Procedures/CleanStates.hpp>

class CleanStateMachine : public StateMachine {
public:
	CleanStateMachine()
		: StateMachine("clean-state-machine", CleanStateNames::SETUP, CleanStateNames::STOP,
			CleanStateNames::IDLE, CleanStateNames::FINISHED) {}			

	void setup() override {
		registerState(CleanStateIdle(), CleanStateNames::IDLE);
		registerState(CleanStateFinished(), CleanStateNames::FINISHED);

		registerState(CleanStateStop(), CleanStateNames::STOP);
		registerState(CleanStateSetup(), CleanStateNames::SETUP);
		registerState(CleanStateFlush(), CleanStateNames::FLUSH);
		registerState(CleanStateSample(), CleanStateNames::SAMPLE);

	}
};