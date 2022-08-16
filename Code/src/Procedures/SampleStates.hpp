#pragma once
#include <KPState.hpp>
#include <Application/Constants.hpp>

namespace SampleStateNames {
	constexpr const char * IDLE				= "sample-state-idle";
	constexpr const char * FLUSH			= "sample-state-flush";
	constexpr const char * SAMPLE			= "sample-state-sample";
	constexpr const char * STOP				= "sample-state-stop";
	constexpr const char * FINISHED			= "sample-state-finished";
	constexpr const char * SETUP			= "sample-state-setup";
	constexpr const char * ONRAMP			= "sample-state-onramp";
	constexpr const char * BETWEEN_PUMP		= "sample-state-between-pump";
	constexpr const char * BETWEEN_VALVE	= "sample-state-between-valve";
	constexpr const char * FILL_TUBE_ONRAMP = "sample-state-fill-tube-onramp";
	constexpr const char * FILL_TUBE		= "sample-state-fill-tube";
	constexpr const char * PRESSURE_TARE	= "sample-state-pressure-tare";
	constexpr const char * LOG_BUFFER		= "sample-state-log-buffer";
	constexpr const char * LOAD_BUFFER		= "sample-state-load-buffer";
};	// namespace SampleStateNames

class SampleStateIdle : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = DefaultTimes::IDLE_TIME;
};

class SampleStateFlush : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = DefaultTimes::FLUSH_TIME;
};

class SampleStateSample : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	void leave(KPStateMachine & sm) override;
	bool badPressure();
	int time   = DefaultTimes::SAMPLE_TIME;
	int time_adj_ms = secsToMillis(time);
	int mass = 100;
	double avg_rate = 0;
	float current_tare;
	float accum_load;
	int accum_time;
	float weight_remaining;
	float new_time_est;
	float prior_time_est;
	float code_time_est;
	float new_load = 0;
	unsigned long prior_time;
	unsigned long new_time;
	float prior_rate = 0;
	float new_rate;
	float wt_offset;
};

class SampleStateStop : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 45;
};

class SampleStateFinished : public KPState {
public:
	void enter(KPStateMachine & sm) override;
};

class SampleStateSetup : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = DefaultTimes::SETUP_TIME;
	int tod_enabled;
	int tod;
};

class SampleStateOnramp : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 5;
};

class SampleStateBetweenPump : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 6;
};

class SampleStateBetweenValve : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 7;
};

class SampleStateFillTubeOnramp : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 7;
};

class SampleStateFillTube : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 5;
};

// This sets the normal pressure range on the first cycle
class SampleStatePressureTare : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	void update(KPStateMachine & sm) override;
	void leave(KPStateMachine & sm) override;
	int time = 5;
	long sum;
	int count;
	int range_size = 350;
};

class SampleStateLogBuffer : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	float final_load;
	float current_tare;
	float sampledLoad;
	int mass;
	int sampledTime;
	float average_pump_rate;
	float load_diff;
};

class SampleStateLoadBuffer : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	float current_tare;
	float tempC;
};