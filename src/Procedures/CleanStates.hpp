#pragma once
#include <KPState.hpp>
#include <Application/Constants.hpp>

namespace CleanStateNames {
	constexpr const char * SETUP	= "clean-state-setup";
	constexpr const char * IDLE		= "clean-state-idle";
	constexpr const char * FLUSH	= "clean-state-flush";
	constexpr const char * SAMPLE	= "clean-state-sample";
	constexpr const char * STOP		= "clean-state-stop";
	constexpr const char * FINISHED = "clean-state-finished";
}  // namespace CleanStateNames

class CleanStateIdle : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = DefaultTimesClean::IDLE_TIME;
};

class CleanStateSetup : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 6;
};

class CleanStateFlush : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = DefaultTimesClean::FLUSH_TIME;
};

class CleanStateSample : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	void leave(KPStateMachine & sm) override;
	int time = DefaultTimesClean::SAMPLE_TIME;
};

class CleanStateStop : public KPState {
public:
	void enter(KPStateMachine & sm) override;
};

class CleanStateFinished : public KPState {
public:
	void enter(KPStateMachine & sm) override;
};