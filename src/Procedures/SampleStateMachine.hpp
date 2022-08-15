#pragma once

#include <Components/StateMachine.hpp>
#include <Procedures/SampleStates.hpp>

class SampleStateMachine : public StateMachine {
public:
	SampleStateMachine()
		: StateMachine("sample-state-machine", SampleStateNames::SETUP, SampleStateNames::STOP,
			SampleStateNames::IDLE, SampleStateNames::FINISHED) {}

	void setup() override {
		registerState(SampleStateIdle(), SampleStateNames::IDLE);
		registerState(SampleStateFinished(), SampleStateNames::FINISHED);

		registerState(SampleStateSetup(),
			SampleStateNames::SETUP,
			SampleStateNames::FILL_TUBE_ONRAMP);
		registerState(SampleStateFillTubeOnramp(),
			SampleStateNames::FILL_TUBE_ONRAMP,
			SampleStateNames::FILL_TUBE);
		registerState(SampleStateFillTube(),
			SampleStateNames::FILL_TUBE,
			SampleStateNames::PRESSURE_TARE);
		registerState(SampleStatePressureTare(),
			SampleStateNames::PRESSURE_TARE,
			SampleStateNames::ONRAMP);

		registerState(SampleStateOnramp(), SampleStateNames::ONRAMP, SampleStateNames::FLUSH);
		registerState(SampleStateFlush(), SampleStateNames::FLUSH, SampleStateNames::BETWEEN_PUMP);

		registerState(SampleStateBetweenPump(),
			SampleStateNames::BETWEEN_PUMP,
			SampleStateNames::LOAD_BUFFER);

		registerState(SampleStateLoadBuffer(),
			SampleStateNames::LOAD_BUFFER,
			SampleStateNames::BETWEEN_VALVE);

		registerState(SampleStateBetweenValve(),
			SampleStateNames::BETWEEN_VALVE,
			SampleStateNames::SAMPLE);

		registerState(SampleStateSample(), SampleStateNames::SAMPLE, SampleStateNames::STOP);

		registerState(SampleStateStop(), SampleStateNames::STOP, SampleStateNames::LOG_BUFFER);
		registerState(SampleStateLogBuffer(), SampleStateNames::LOG_BUFFER, SampleStateNames::IDLE);
	}
};