#include <Procedures/SampleStates.hpp>
#include <Application/Application.hpp>
#include <FileIO/SerialSD.hpp>

void writeLatch(bool controlPin, ShiftRegister & shift) {
	shift.setPin(controlPin, HIGH);
	shift.write();
	delay(80);
	shift.setPin(controlPin, LOW);
	shift.write();
}

// Idle
void SampleStateIdle::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	if (app.sm.current_cycle < app.sm.last_cycle)
		setTimeCondition(time, [&]() { sm.transitionTo(SampleStateNames::ONRAMP); });
	else
		sm.transitionTo(SampleStateNames::FINISHED);
}

// Stop
void SampleStateStop::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.pump.off();

	app.shift.writeAllRegistersLow();
	app.shift.writeLatchOut();
	/*	digitalWrite(HardwarePins::WATER_VALVE, LOW);
		digitalWrite(HardwarePins::FLUSH_VALVE, LOW);*/
	// testing
	/*
	Serial.print("Load @ end of cycle ");
	Serial.print(app.sm.current_cycle);
	Serial.print(": ");
	Serial.println(app.load_cell.getLoad());
	*/
	sm.next();
}

void SampleStateOnramp::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	app.shift.setAllRegistersLow();
	app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH);  // write in skinny
	app.shift.write();								   // write shifts wide*/
	/*digitalWrite(HardwarePins::WATER_VALVE, LOW);
	digitalWrite(HardwarePins::FLUSH_VALVE, HIGH);*/

	setTimeCondition(time, [&]() { sm.next(); });
}

// Flush
void SampleStateFlush::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	app.shift.setAllRegistersLow();
	app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH);  // write in skinny
	app.shift.write();								   // write shifts wide*/
	/*digitalWrite(HardwarePins::WATER_VALVE, LOW);
	digitalWrite(HardwarePins::FLUSH_VALVE, HIGH);*/
	app.pump.on();

	setTimeCondition(time, [&]() { sm.next(); });
}

// Sample
void SampleStateSample::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.shift.setAllRegistersLow();
	app.shift.setPin(TPICDevices::WATER_VALVE, HIGH);
	app.shift.write();
	/*digitalWrite(HardwarePins::WATER_VALVE, HIGH);
	digitalWrite(HardwarePins::FLUSH_VALVE, LOW);*/
	app.pump.on();
	Serial.print("Max pressure: ");
	Serial.println(app.pressure_sensor.max_pressure);
	Serial.print("Min pressure: ");
	Serial.println(app.pressure_sensor.min_pressure);

	/*
	// testing
	Serial.print("Load @ beginning of cycle ");
	Serial.print(app.sm.current_cycle);
	Serial.print(": ");
	Serial.println(app.load_cell.getLoad());*/

	auto const condition = [&]() {
		bool t		  = timeSinceLastTransition() >= secsToMillis(time);
		bool pressure = !app.pressure_sensor.checkPressure();
		bool load	  = app.load_cell.getTaredLoad() >= volume;
		if (t)
			Serial.println("Sample state ended due to: time");
		if (pressure)
			Serial.println("Sample state ended due to: pressure");
		if (load)
			Serial.println("Sample state ended due to: load");
		return t || load || pressure;
	};
	setCondition(condition, [&]() { sm.next(); });
}

// Sample leave
void SampleStateSample::leave(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	// testing
	Serial.print("Load @ end of cycle ");
	Serial.print(app.sm.current_cycle);
	Serial.print(": ");
	Serial.println(app.load_cell.getLoad());

	app.sm.current_cycle += 1;
	app.load_cell.reTare();
}

// Finished
void SampleStateFinished::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.led.setFinished();
	app.sm.reset();
}
// Purge (obsolete)
void SampleStatePurge::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.shift.setAllRegistersLow();
	app.shift.setPin(TPICDevices::WATER_VALVE, HIGH);
	app.shift.write();
	/*digitalWrite(HardwarePins::WATER_VALVE, HIGH);
	digitalWrite(HardwarePins::FLUSH_VALVE, LOW);*/
	app.pump.on(Direction::reverse);
	setTimeCondition(time, [&]() { sm.next(); });
}

// Setup
void SampleStateSetup::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.led.setRun();
	app.load_cell.reTare();
	setTimeCondition(time, [&]() { sm.next(); });
}

void SampleStateBetweenPump::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.pump.off();
	setTimeCondition(time, [&]() { sm.next(); });
}

void SampleStateBetweenValve::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.shift.setAllRegistersLow();
	app.shift.setPin(TPICDevices::WATER_VALVE, HIGH);
	app.shift.write();
	/*digitalWrite(HardwarePins::FLUSH_VALVE, LOW);
	digitalWrite(HardwarePins::WATER_VALVE, HIGH);*/
	setTimeCondition(time, [&]() { sm.next(); });
}

void SampleStateFillTubeOnramp::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	app.shift.setAllRegistersLow();
	app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH);  // write in skinny
	app.shift.write();								   // write shifts wide*/
	/*digitalWrite(HardwarePins::FLUSH_VALVE, HIGH);*/

	setTimeCondition(time, [&]() { sm.next(); });
}

void SampleStateFillTube::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	app.shift.setAllRegistersLow();
	app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH);  // write in skinny
	app.shift.write();								   // write shifts wide*/
	/*digitalWrite(HardwarePins::FLUSH_VALVE, HIGH);*/

	app.pump.on();

	setTimeCondition(time, [&]() { sm.next(); });
}

void SampleStatePressureTare::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.shift.setAllRegistersLow();
	app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH);  // write in skinny
	app.shift.write();								   // write shifts wide*/
	/*digitalWrite(HardwarePins::WATER_VALVE, LOW);
	digitalWrite(HardwarePins::FLUSH_VALVE, HIGH);*/

	app.pump.on();

	sum	  = 0;
	count = 0;

	setTimeCondition(time, [&]() { sm.next(); });
}

void SampleStatePressureTare::update(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	sum += app.pressure_sensor.getPressure();
	++count;
}

void SampleStatePressureTare::leave(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
#ifndef DISABLE_PRESSURE_TARE
	int avg = sum / count;
	Serial.print("Normal pressure set to value: ");
	Serial.println(avg);

	app.pressure_sensor.max_pressure = avg + range_size;
	app.pressure_sensor.min_pressure = avg - range_size;
#endif
#ifdef DISABLE_PRESSURE_TARE
	Serial.println("Pressure tare state is disabled.");
	Serial.println(
		"If this is a mistake, please remove DISABLE_PRESSURE_TARE from the buildflags.");
	Serial.print("Max pressure (set manually): ");
	Serial.println(app.pressure_sensor.max_pressure);
	Serial.print("Min pressure (set manually): ");
	Serial.println(app.pressure_sensor.min_pressure) :
#endif
}