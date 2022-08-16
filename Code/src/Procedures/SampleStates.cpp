#include <Procedures/SampleStates.hpp>
#include <Application/Application.hpp>
//#include <FileIO/SerialSD.hpp>
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


//SerialSD SSD;
CSVWriter csvw{"data.csv"};

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
	sample_end_time = millis();
	print("sample_end_time ms;;;");
	println(sample_end_time);
	pumpOff = 1;
	println("Pump off");

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

void SampleStateOnramp::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	print("Starting cycle number;");
	println(app.sm.current_cycle);
	char cycle_string[50];
	sprintf(cycle_string, "%u", (int)app.sm.current_cycle);
	std::string strings[3] = {time_string,",Starting cycle ", cycle_string};
	csvw.writeStrings(strings, 3);

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

// Flush
void SampleStateFlush::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	if (flushVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH); // write in skinny
		app.shift.write();								   // write shifts wide*/
		flushVOff = 0;
		println("Flush valve turning on");
		sampleVOff = 1;
	}

	if (pumpOff){
		app.pump.on();
		pumpOff = 0;
		println("Pump on");
	}

	setTimeCondition(time, [&]() { sm.next();});
}

// Sample
void SampleStateSample::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	//SSD.print("Input total sampling mass in g;;;");
	//SSD.println(mass);
	//SSD.print("Input total sampling time in ms;;;");
	//SSD.println(time);
	wt_offset = 0;
	current_tare = app.sm.getState<SampleStateLoadBuffer>(SampleStateNames::LOAD_BUFFER).current_tare;

	if (sampleVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::WATER_VALVE, HIGH);
		app.shift.write();
		sampleVOff = 0;
		println("Sample valve turning on");
		flushVOff = 1;
	}

	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	char cycle_string[50];
	sprintf(cycle_string, "%u", (int)app.sm.current_cycle);
	std::string strings[3] = {time_string, "Sample Start Cycle: ", cycle_string};
	csvw.writeStrings(strings, 3);
	sample_start_time = millis();
	print("sample_start_time ms ;;;");
	println(sample_start_time);

	if (pumpOff){
		app.pump.on();
		pumpOff = 0;
		println("Pump on");
	}

	auto const condition = [&]() {
		bool load = 0;
		bool load_rate = 0;
		// check load, exit if matching mass
		new_load = app.load_cell.getLoad(1);
		new_time = millis();
		print("New mass reading;");
		println(new_load);
		print("New time;;;");
		println(new_time);
		// compensate for poor measurements for first 4
		if(load_count==4){
			wt_offset = ((new_load - prior_load)/(new_time - prior_time))*(new_time - sample_start_time);
			print("Weight offset;;;;");
			println(wt_offset);
		}
		
		const auto timenow = now();
		std::stringstream ss;
		ss << timenow;
		std::string time_string = ss.str();
		char cycle_string[50];
		sprintf(cycle_string, "%u", (int)app.sm.current_cycle);
		//print("Current_tare in sample state;;;;");
		//println(current_tare);
		//print("mass var in sample state;;;; ");
		//println(mass);
		load = new_load - current_tare >= mass - wt_offset;
		if (load){
			//SSD.println("Sample state ended due to: load ");
			std::string temp[3] = {time_string,",Ended due to load cycle: ",cycle_string};
			csvw.writeStrings(temp, 3);
			println("Sample state ended due to: load ");
			pressureEnded = 0;
			return load;
		}

			//if not exiting due to sample load, check overall load, time and exit if over time
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
				//if not exiting due to load, time, and pressure, check pumping rate
				else{
					if (load_count > 0){
						//print("prior_load in sample states;;;;");
						//println(prior_load);
						//print("prior_time in sample states;;;;");
						//println(prior_time);
						//new_rate = (new_load - prior_load) / ((new_time - prior_time));
						//print("new_load - prior_load;;;");
						//println(new_load - prior_load);
						//print("(new_time - prior_time);;;");
						//println((new_time - prior_time));
						//print("New rate grams/ms;;;");
						//println(new_rate);
						//print("average rate: new_load - current_tare / new_time - sample_start_time;;;");
						accum_load = new_load - current_tare;
						//print("Accumulated load (g);");
						//println(accum_load);
						accum_time = new_time - sample_start_time;
						//print("Accumulated time (ms);;;");
						//println(accum_time);
						avg_rate = 1000*(accum_load/accum_time);
						print("Average rate in g/s;;;;");
						println(avg_rate,4);
						//println(avg_rate,3);
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
								// check in to see if pumping rate is really slow (half expected rate) meaning getting a lot of air
								load_rate = abs(avg_rate) < 0.004;
								/*if (load_rate){
									//SSD.println("Sample state ended due to: load ");
									std::string temp[1] = {"Sample state ended due to: low load rate"};
									csvw.writeStrings(temp, 1);
									println("Sample state ended due to: low load rate");
									pressureEnded = 0;
									return load_rate;
								}*/
							}
						}
					}
					prior_load = new_load;
					prior_time = new_time;
					prior_rate = new_rate;
					prior_time_est = new_time_est;
					load_count += 1;
					return load || total_load || t_max || t_adj || pressure;// || load_rate;
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

// Finished
void SampleStateFinished::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.led.setFinished();
	app.sm.reset();
}

// Setup
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

void SampleStateBetweenPump::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	app.pump.off();
	pumpOff = 1;
	println("Pump off");
	setTimeCondition(time, [&]() { sm.next();});
}

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

void SampleStateFillTube::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);

	if (flushVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH); // write in skinny
		app.shift.write();								   // write shifts wide*/
		flushVOff = 0;
		println("Flush valve turning on");
		sampleVOff = 1;
	}
	if (pumpOff){
		app.pump.on();
		pumpOff = 0;
		println("Pump on");
	}

	setTimeCondition(time, [&]() { sm.next();});
}

void SampleStatePressureTare::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	if (flushVOff){
		app.shift.setAllRegistersLow();
		app.shift.setPin(TPICDevices::FLUSH_VALVE, HIGH);  // write in skinny
		app.shift.write();								   // write shifts wide*/
		flushVOff = 0;
		println("Flush valve turning on");
		sampleVOff = 1;		
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

void SampleStatePressureTare::update(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	sum += app.pressure_sensor.getPressure();
	++count;
}

void SampleStatePressureTare::leave(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
#ifndef DISABLE_PRESSURE_TARE
	int avg = sum / count;
	char press_string[50];
	sprintf(press_string, "%d.%02u", (int)avg, (int)((avg - (int)avg) * 100));
	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	std::string strings[3] = {time_string,",Pressure,,, ", press_string};
	csvw.writeStrings(strings, 3);
	print("Normal pressure set to value: ");
	println(avg);

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

void SampleStateLogBuffer::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	print("Load at end of cycle;");//+ app.sm.current_cycle + ": " + (double)app.load_cell.getLoad());
	print(app.sm.current_cycle);
	print(";");
	final_load = app.load_cell.getLoad(25);
	println(final_load,3);
	//print to SD
	char cycle_string[50];
	sprintf(cycle_string, "%u", (int)app.sm.current_cycle);
	/*char load_string[50];
	sprintf(load_string, "%d.%02u", (int)final_load, (int)((final_load - (int)final_load) * 100));
	std::string strings[4] = {"Full load at end of cycle ", cycle_string,",, ",load_string};
	csvw.writeStrings(strings, 4);*/

	//evaluate load and sampling time
	current_tare = app.sm.getState<SampleStateLoadBuffer>(SampleStateNames::LOAD_BUFFER).current_tare;

	//print("current_tare in log buffer;;;;");
	//println(current_tare);
	sampledLoad = final_load - current_tare;
	print("sampledLoad: final_load - current_tare;");
	println(sampledLoad);
	char sampledload_string[50];
	sprintf(sampledload_string, "%d.%02u", (int)sampledLoad, (int)((sampledLoad - (int)sampledLoad) * 100));
	const auto timenow = now();
	std::stringstream ss;
	ss << timenow;
	std::string time_string = ss.str();
	std::string strings[5] = {time_string,",Sampled load at end of cycle ", cycle_string,",",sampledload_string};
	csvw.writeStrings(strings, 5);
	//print("sample_start_time ms in log buffer;;;");
	//println(sample_start_time);
	//print("sample_end_time ms in log buffer;;;");
	//println(sample_end_time);
	sampledTime = (sample_end_time - sample_start_time);
	print("sampledTime period in ms;;");
	println(sampledTime);
	//calculate average pumping rate
	average_pump_rate = (sampledLoad / sampledTime)*1000;
	print("Average pumping rate grams/sec: 1000*sampledLoad/sampledTime;;;");
	println(average_pump_rate,4);
	//change time opposite sign of load diff (increase for negative, decrease for positive)
	mass = app.sm.getState<SampleStateSample>(SampleStateNames::SAMPLE).mass;
	print("load_diff: mass - sampledLoad;;;");
	load_diff = mass - sampledLoad;
	println(load_diff);

	//update time if sample didn't end due to pressure
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

void SampleStateLoadBuffer::enter(KPStateMachine & sm) {
	Application & app = *static_cast<Application *>(sm.controller);
	print("Temp: ");
	tempC = app.pressure_sensor.getTemp();
	println(tempC);
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
	//SSD.println("Get load");
	//println(app.load_cell.getLoad(1));
	print("Tare load;");
	current_tare = app.load_cell.reTare(25);
	println(current_tare);
	/*char tare_string[50];
	sprintf(tare_string, "%d.%02u", (int)current_tare, (int)((current_tare - (int)current_tare) * 100));
	std::string tare_strings[4] = {"Tare load for cycle ", cycle_string,",, ",tare_string};
	csvw.writeStrings(tare_strings, 4);*/
	sm.next();
}