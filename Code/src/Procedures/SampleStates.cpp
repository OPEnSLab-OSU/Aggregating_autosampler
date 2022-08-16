#include <Procedures/SampleStates.hpp>
#include <Application/Application.hpp>
#include <time.h>
#include <sstream>
#include <String>

bool pumpOff = 1;
bool flushVOff = 1;
bool sampleVOff = 1;
bool pressureEnded = 0;
unsigned long sample_start_time;
unsigned long sample_end_time;
short load_count = 0;
float prior_load = 0;
int sampler = 1;

// Setup file to log data to
CSVWriter csvw{"data.csv"};

//Function for interacting with the valves
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

// Setup: Change LED color, wait SETUP_TIME to allow for delayed sampling start
void SampleStateSetup::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.led.setRun();
	//get and print time
	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	std::string strings[2] = {time_string, ",New Sampling Sequence"};
	csvw.writeStrings(strings, 2);
	setTimeCondition(time, [&]() { sm.next();});
}

//Fill tube onramp: turn on flush valve and wait for preset time to allow valve to fully open
void SampleStateFillTubeOnramp::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	if (flushVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH);// write in skinny
		app.shift.write();								   // write shifts wide*/
		flushVOff = 0;
		println("Flush valve turning on");
		sampleVOff = 1;
	}
	setTimeCondition(time, [&]() { sm.next();});
}

//Fill tube: turn on pump and wait for preset time to fill tubes with liquid
void SampleStateFillTube::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	if (flushVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH); // write in skinny
		app.shift.write();								   // write shifts wide*/
		flushVOff = 0;
		println("Flush valve turning on");
		sampleVOff = 1;
		//give valve 6 seconds to turn on
		delay(6000);
	}
	if (pumpOff){
		app.pump.on();
		pumpOff = 0;
		println("Pump on");
	}

	setTimeCondition(time, [&]() { sm.next();});
}

//Pressure tare (enter): Make sure flush valve and pump are on, and take pressure measurements for preset time
void SampleStatePressureTare::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	if (flushVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH);  // write in skinny
		app.shift.write();								   // write shifts wide*/
		flushVOff = 0;
		println("Flush valve turning on");
		sampleVOff = 1;		
		//give valve 6 seconds to turn on
		delay(6000);
	}
	if (pumpOff){
		app.pump.on();
		pumpOff = 0;
		println("Pump on");
	}

	sum	  = 0;
	count = 0;

	setTimeCondition(time, [&]() { sm.next();});
}

//Take instantaneous pressure measurements
void SampleStatePressureTare::update(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	sum += app.pressure_sensor.getPressure();
	++count;
}

// Average all pressure measurements and set pressure tare
void SampleStatePressureTare::leave(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
#ifndef DISABLE_PRESSURE_TARE
	// get time for SD printing
	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	// Average pressure
	int avg = sum / count;
	//Prepare values for printing
	char press_string[50];
	sprintf(press_string, "%d.%02u", (int)avg, (int)((avg - (int)avg) * 100));
	// Composite strings and print to SD
	std::string strings[3] = {time_string,",Pressure,,, ", press_string};
	csvw.writeStrings(strings, 3);
	// Print pressure to serial monitor
	print("Normal pressure set to value: ");
	println(avg);

	// Set min and max pressure valves for pressure stopping criteria
	app.pressure_sensor.max_pressure = avg + range_size;
	app.pressure_sensor.min_pressure = avg - range_size;
	print("Max pressure: ");
	println(app.pressure_sensor.max_pressure);
	print("Min pressure: ");
	println(app.pressure_sensor.min_pressure);
#endif
#ifdef DISABLE_PRESSURE_TARE
	SSD.println("Pressure tare state is disabled.");
	SSD.println("If this is a mistake, please remove DISABLE_PRESSURE_TARE from the buildflags.");
	SSD.print("Max pressure (set manually): ");
	SSD.println(app.pressure_sensor.max_pressure);
	SSD.print("Min pressure (set manually): ");
	SSD.println(app.pressure_sensor.min_pressure);
#endif
}

//States that run every cycle

//Onramp: turn on flush valve and wait preset time to allow valve to fully open
void SampleStateOnramp::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	// Get time and cycle and print to serial monitor and SD
	//cycle to serial
	print("Starting cycle number;");
	println(app.sm.current_cycle);
	//cycle prep for SD
	char cycle_string[50];
	sprintf(cycle_string, "%u", (int)app.sm.current_cycle);
	// get time
	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	// put together string for SD
	std::string strings[3] = {time_string,",Starting cycle ", cycle_string};
	//write to SD
	csvw.writeStrings(strings, 3);

	// turn on flush valve if off
	if (flushVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH);  // write in skinny
		app.shift.write();								   // write shifts wide*/
		flushVOff = 0;
		println("Flush valve turning on");
		sampleVOff = 1;
	}

	setTimeCondition(time, [&]() { sm.next();});
}

// Flush: turn on pump and run for preset time to replace water in all tubing
void SampleStateFlush::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	// make sure flush valve is on
	if (flushVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH); // write in skinny
		app.shift.write();								   // write shifts wide*/
		flushVOff = 0;
		println("Flush valve turning on");
		sampleVOff = 1;
		//give valve 6 seconds to turn on
		delay(6000);
	}

	if (pumpOff){
		app.pump.on();
		pumpOff = 0;
		println("Pump on");
	}

	setTimeCondition(time, [&]() { sm.next();});
}

//Between pump: Pump turned off, wait for preset time to reduce noise in load measurement
void SampleStateBetweenPump::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.pump.off();
	pumpOff = 1;
	println("Pump off");
	setTimeCondition(time, [&]() { sm.next();});
}

// Load buffer: Temperature and load measured
void SampleStateLoadBuffer::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	// Measure initial water temperature
	tempC = app.pressure_sensor.getTemp();
	//print to serial monitor
	print("Temp: ");
	println(tempC);
	// Get cycle and time to include with temperature print to SD
	char cycle_string[50];
	sprintf(cycle_string, "%u", (int)app.sm.current_cycle);
	char tempC_string[50];
	sprintf(tempC_string, "%d.%02u", (int)tempC, (int)((tempC - (int)tempC) * 100));
	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	std::string strings[5] = {time_string,",Starting temperature for cycle ", cycle_string,",,", tempC_string};
	csvw.writeStrings(strings, 5);

	// Take 25 measurements with load cell for initial mass value
	current_tare = app.load_cell.reTare(25);
	print("Tare load;");
	println(current_tare);

	// Move on to next state
	sm.next();
}

// Between valve: Flush valve turned off, sample valve turned on, wait preset time for valve to fully open
void SampleStateBetweenValve::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	
	if (sampleVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::WATER_VALVE, HIGH);
		app.shift.write();
		sampleVOff = 0;
		println("Sample valve turning on");
		flushVOff = 1;
	}
	setTimeCondition(time, [&]() { sm.next();});
}

// Sample: turn on pump, and run until stopped by mass, pressure, or time
void SampleStateSample::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	wt_offset = 0;
	current_tare = app.sm.getState<SampleStateLoadBuffer>(SampleStateNames::LOAD_BUFFER).current_tare;

	//open sample valve if not open
	if (sampleVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::WATER_VALVE, HIGH);
		app.shift.write();
		sampleVOff = 0;
		println("Sample valve turning on");
		flushVOff = 1;
		//give valve 6 seconds to turn on
		delay(6000);
	}

	//time and cycle to SD
	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	char cycle_string[50];
	sprintf(cycle_string, "%u", (int)app.sm.current_cycle);
	std::string strings[3] = {time_string, "Sample Start Cycle: ", cycle_string};
	csvw.writeStrings(strings, 3);

	// relative time for serial monitor and timing
	sample_start_time = millis();
	print("sample_start_time ms ;;;");
	println(sample_start_time);

	// turn on pump if not on
	if (pumpOff){
		app.pump.on();
		pumpOff = 0;
		println("Pump on");
	}

	//check for stopping criteria
	auto const condition = [&]() {
		bool load = 0;
		// get instantaneous load
		new_load = app.load_cell.getLoad(1);
		new_time = millis();
		print("New mass reading;");
		println(new_load);
		print("New time;;;");
		println(new_time);
		// compensate for poor measurements for first 4
		if (sampler==2){
			if(load_count==4){
				wt_offset = ((new_load - prior_load)/(new_time - prior_time))*(new_time - sample_start_time);
				print("Weight offset;;;;");
				println(wt_offset);
			}
		}
		// use basic offset for sampler 1
		else{
			wt_offset = 0.05*mass;
		}
		
		//get time and cycle values for SD output
		const auto timenow = now();
		std::stringstream ss;
		ss << timenow;
		std::string time_string = ss.str();
		char cycle_string[50];
		sprintf(cycle_string, "%u", (int)app.sm.current_cycle);

		// check for meeting load target
		load = new_load - current_tare >= mass - wt_offset;
		if (load){
			std::string temp[3] = {time_string,",Ended due to load cycle: ",cycle_string};
			csvw.writeStrings(temp, 3);
			println("Sample state ended due to: load ");
			pressureEnded = 0;
			return load;
		}

			//if not exiting due to sample load, check overall load, time, and pressure
		else{
			// check load reading relative to cap of 2900 g
			bool total_load = 0;
			total_load = new_load > 2900;
			if (total_load){
				std::string temp[4] = {time_string,",Ended due to total load cycle: ",cycle_string};
				csvw.writeStrings(temp, 4);
				println("Sample state ended due to: total load ");
				pressureEnded = 0;
				// trigger end of all sampling
				app.sm.current_cycle = app.sm.last_cycle;
				return total_load;
			}
			
			// check for time stopping criteria: t_max = SAMPLE_TIME; t_adj is iteratively calculated
			bool t_max = timeSinceLastTransition() >= secsToMillis(time);
			bool t_adj = timeSinceLastTransition() >= time_adj_ms;
			if (t_max || t_adj){
				std::string temp[4] = {time_string,",Ended due to time cycle: ",cycle_string};
				csvw.writeStrings(temp, 4);
				println("Sample state ended due to: time");
				pressureEnded = 0;
				return t_max || t_adj;
			}
			//if not exiting due to load and time, check pressure
			else{
				bool pressure = !app.pressure_sensor.isWithinPressure();
				if (pressure){
					std::string temp[4] = {time_string, ",Ended due to pressure cycle: ",cycle_string};
					csvw.writeStrings(temp, 4);
					println("Sample state ended due to: pressure");
					pressureEnded = 1;
					return pressure;
				}
				//if continuing, check pumping rate
				else{
					if (load_count > 0){
						accum_load = new_load - current_tare;
						accum_time = new_time - sample_start_time;
						avg_rate = 1000*(accum_load/accum_time);
						print("Average rate in g/s;;;;");
						println(avg_rate,4);
						//check to see if sampling time is appropriate
						print("Coded time remaining in millis;;;");
						code_time_est = time_adj_ms - timeSinceLastTransition();
						println(code_time_est);
						// update time if more than 10% off and new load - tare > 1
						if (load_count > 5){
							if (new_load - current_tare > 1){
								weight_remaining = mass - (new_load - current_tare);
								print("Weight remaining (mass - (new_load - current_tare));");
								println(weight_remaining);
								// calculate new time based upon average rate
								new_time_est = weight_remaining/((new_load - current_tare)/(new_time - sample_start_time));
								print("Estimated time remaining in ms: weight_remaining/average rate;;;");
								println(new_time_est);
								if (abs((code_time_est - new_time_est)/code_time_est) > 0.1){
									time_adj_ms = new_time_est + timeSinceLastTransition();
									print("Code time outside 10 percent of estimated time. Updated sampling time in millis;;;");
									println(time_adj_ms);
								}
							}
						}
					}
					// update for comparison in next loop
					prior_load = new_load;
					prior_time = new_time;
					prior_rate = new_rate;
					prior_time_est = new_time_est;
					load_count += 1;
					return load || total_load || t_max || t_adj || pressure;
					}
				}
			}
	};
	setCondition(condition, [&]() { sm.next();});
}

// Sample leave
void SampleStateSample::leave(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	load_count = 0;
	prior_load = 0;
}

// Stop: Sample valve and pump turned off. Wait preset time to reduce noise in final load measurement
void SampleStateStop::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	//turn off pump
	app.pump.off();
	pumpOff = 1;
	println("Pump off");

	// get and print relative end time
	sample_end_time = millis();
	print("Sample_end_time ms;;;");
	println(sample_end_time);

	//turn off both valves
	app.shift.writeAllRegistersLow();
	app.shift.writeLatchOut();
	flushVOff = 1;
	sampleVOff = 1;

	//get and print to SD pressure after pump and valves are off
	long curr_pressure = app.pressure_sensor.getPressure();
	char press_string[50];
	sprintf(press_string, "%Lu", (long)curr_pressure);
	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	char cycle_string[50];
	sprintf(cycle_string, "%u", (int)app.sm.current_cycle);
	std::string strings[5] = {time_string,",Ending pressure for cycle: ",cycle_string,",,, ", press_string};
	csvw.writeStrings(strings, 5);

	setTimeCondition(time, [&]() { sm.next();});
}

//Log buffer: Measure final load for cycle
void SampleStateLogBuffer::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	// Take 25 measurements and average for total load at end of cycle	
	final_load = app.load_cell.getLoad(25);
	// Print to serial monitor
	print("Load at end of cycle;");
	print(app.sm.current_cycle);
	print(";");
	println(final_load,3);

	//evaluate load and sampling time
	current_tare = app.sm.getState<SampleStateLoadBuffer>(SampleStateNames::LOAD_BUFFER).current_tare;
	// Calculate cycle load
	sampledLoad = final_load - current_tare;
	print("sampledLoad: final_load - current_tare;");
	println(sampledLoad);
	//Prepare and print to SD
	char cycle_string[50];
	sprintf(cycle_string, "%u", (int)app.sm.current_cycle);
	char sampledload_string[50];
	sprintf(sampledload_string, "%d.%02u", (int)sampledLoad, (int)((sampledLoad - (int)sampledLoad) * 100));
	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	std::string strings[5] = {time_string,",Sampled load at end of cycle ", cycle_string,",",sampledload_string};
	csvw.writeStrings(strings, 5);

	// Calculate and print to serial cycle time
	sampledTime = (sample_end_time - sample_start_time);
	print("sampledTime period in ms;;");
	println(sampledTime);

	//calculate and print to serial average pumping rate
	average_pump_rate = (sampledLoad / sampledTime)*1000;
	print("Average pumping rate grams/sec: 1000*sampledLoad/sampledTime;;;");
	println(average_pump_rate,4);

	// calculate and print to serial cycle load relative to target
	mass = app.sm.getState<SampleStateSample>(SampleStateNames::SAMPLE).mass;
	load_diff = mass - sampledLoad;
	print("load_diff: mass - sampledLoad;;;");
	println(load_diff);

	//update time if sample didn't end due to pressure
	//change time opposite sign of load diff (increase for negative, decrease for positive)
	if (!pressureEnded){	
		// change sampling time if load was +- 5% off from set weight
		if (abs(mass - sampledLoad)/mass > 0.05){
			println("Sample mass outside of 5 percent tolerance");
			sampledTime += (load_diff)/average_pump_rate;
			print("new sampling time period in ms: load diff/avg rate;;");
		}
		else {
			print("Sampling time period set to match last sample time;;");
		}
		println(sampledTime);
		//set new sample time
		app.sm.getState<SampleStateSample>(SampleStateNames::SAMPLE).time_adj_ms = sampledTime;
	}
	//advance sample number
	app.sm.current_cycle += 1;
	sm.next();
}

//State after all cycles completed

// Finished
void SampleStateFinished::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.led.setFinished();
	app.sm.reset();
}