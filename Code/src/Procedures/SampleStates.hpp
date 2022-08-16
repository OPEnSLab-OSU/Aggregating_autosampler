#pragma once
#include <KPState.hpp>
#include <Application/Constants.hpp>

namespace SampleStateNames {
	constexpr const char * IDLE				= "sample-state-idle";
	constexpr const char * SETUP			= "sample-state-setup";
	constexpr const char * FILL_TUBE_ONRAMP = "sample-state-fill-tube-onramp";
	constexpr const char * FILL_TUBE		= "sample-state-fill-tube";
	constexpr const char * PRESSURE_TARE	= "sample-state-pressure-tare";
	constexpr const char * ONRAMP			= "sample-state-onramp";
	constexpr const char * FLUSH			= "sample-state-flush";
	constexpr const char * BETWEEN_PUMP		= "sample-state-between-pump";
	constexpr const char * LOAD_BUFFER		= "sample-state-load-buffer";
	constexpr const char * BETWEEN_VALVE	= "sample-state-between-valve";
	constexpr const char * SAMPLE			= "sample-state-sample";
	constexpr const char * STOP				= "sample-state-stop";
	constexpr const char * LOG_BUFFER		= "sample-state-log-buffer";
	constexpr const char * FINISHED			= "sample-state-finished";
};	// namespace SampleStateNames

class SampleStateIdle : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = DefaultTimes::IDLE_TIME;
};

// Time before first cycle starts: SETUP_TIME
class SampleStateSetup : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = DefaultTimes::SETUP_TIME;
	int tod_enabled;
	int tod;
};

// Flush valve turned on; wait for preset time
class SampleStateFillTubeOnramp : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 7;
};

// Pump turned on; wait for preset time
class SampleStateFillTube : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 5;
};

// This sets the normal pressure range on the first cycle with measurements over preset time
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

// Flush valve turned on; wait for preset time for valve to open
class SampleStateOnramp : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 7;
};

// Pump turned on; wait for FLUSH_TIME
class SampleStateFlush : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = DefaultTimes::FLUSH_TIME;
};

// Pump turned off; wait for preset time
class SampleStateBetweenPump : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 5;
};

// Temperature and load measured
class SampleStateLoadBuffer : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	float current_tare;
	float tempC;
};

//Flush valve turned off, sample valve turned on; wait for preset time for valve to open
class SampleStateBetweenValve : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 7;
};

// Pump turned on, load and pressure measured until stopping criteria or SAMPLE_TIME
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

// Sample valve and pump turned off. Wait preset time to reduce noise in final load measurement
class SampleStateStop : public KPState {
public:
	void enter(KPStateMachine & sm) override;
	int time = 60;
};

// Measure final load for cycle
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

//Exit sample machine after all cycles complete
class SampleStateFinished : public KPState {
public:
	void enter(KPStateMachine & sm) override;
};





