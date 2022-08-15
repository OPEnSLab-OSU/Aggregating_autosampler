#pragma once
#include <KPController.hpp>
#include <KPFileLoader.hpp>
#include <KPStateMachine.hpp>

#include <KPSerialInputObserver.hpp>
#include <KPSerialInput.hpp>

#include <Procedures/SampleStateMachine.hpp>
#include <Procedures/CleanStateMachine.hpp>

#include <Application/Constants.hpp>

#include <Components/ShiftRegister.hpp>
#include <Components/Pump.hpp>

#include <Components/Button.hpp>

#include <Components/Shell.hpp>

#include <string>

//#include <FileIO/Logger.hpp>

#include <SD.h>
#include <ArduinoJson.h>

#include <Application/Clock.hpp>
#include <Components/LED.hpp>
#include <Components/PressureSensor.hpp>
#include <Components/LoadCell.hpp>

class Application : public KPController, public KPSerialInputObserver {
public:
	// Component add
	Clock clock{"clock"};
	//Logger logger{"logger", this};
	SampleStateMachine sm;
	CleanStateMachine csm;
	Button run_button{"run-button", this, HardwarePins::RUN_BUTTON, sm};
	Button clean_button{"clean-button", this, HardwarePins::CLEAN_BUTTON, csm};
	Pump pump{"pump", HardwarePins::MOTOR_FORWARDS, HardwarePins::MOTOR_REVERSE};
	ShiftRegister shift{"shift-register",
		32,
		HardwarePins::SHFT_REG_DATA,
		HardwarePins::SHFT_REG_CLOCK,
		HardwarePins::SHFT_REG_LATCH};
	Shell shell{"shell", this};
	LED led{"led", this};
	PressureSensor pressure_sensor{"pressure-sensor", this};
	StaticJsonDocument<512> doc;
	LoadCell load_cell{"load-cell", this};
	void setup() override {
		Serial.begin(9600);
		delay(3000);
		// Component setup
		Serial.println("OK: Serial monitor online");
		addComponent(sm);
		addComponent(csm);
		addComponent(pump);
		addComponent(shift);
		addComponent(KPSerialInput::sharedInstance());
		addComponent(shell);
		//addComponent(logger);
		addComponent(clock);
		addComponent(led);
		addComponent(pressure_sensor);
		SD.begin(HardwarePins::SD);
		addComponent(load_cell);
		KPSerialInput::sharedInstance().addObserver(this);
		loadInfo();
	}

	bool isBusy() {
		return sm.isBusy() || csm.isBusy();
	}

	void update() override {
		if (!csm.isBusy()) {
			run_button.listen();
		}
		if (!sm.isBusy()) {
			clean_button.listen();
		}
		KPController::update();
#ifdef INFO_SPAM
		// Note: this probably has severe impacts on performance.
		// Only use in the case of a weird load/pressure bug.
		print("Load 100: ");
		load_cell.getLoadPrint(100);
		println("; Pressure: ", pressure_sensor.getPressure());
#endif
	}
	// Serial Monitor
	void commandReceived(const char * line, size_t size) override {
		std::string * args = new std::string[5];
		char str[80];
		strcpy(str, line);
		println("Received: ", str);
		const char delim[2] = " ";
		const char * tok	= strtok(str, delim);
		int i				= 0;
		while (tok != NULL) {
			args[i] = tok;
			tok		= strtok(NULL, delim);
			++i;
		}
		shell.runFunction(args, i);
	}

	std::string readEntireFile(File & file) {
		std::string contents;
		while (-1 != file.peek()) {
			contents.push_back(file.read());
		}
		return contents;
	}
	void loadInfo() {
		if (SD.exists("state.js")) {
			File file = SD.open("state.js", FILE_READ);
			if (file) {
				std::string contents = readEntireFile(file);
				deserializeJson(doc, contents);
				if (doc.containsKey("sample")) {
					sm.getState<SampleStateFlush>(SampleStateNames::FLUSH).time
						= doc["sample"]["flush_time"];
					Serial.print("Flush time loaded from SD: ");
					Serial.println(sm.getState<SampleStateFlush>(SampleStateNames::FLUSH).time);
					sm.getState<SampleStateSample>(SampleStateNames::SAMPLE).time
						= doc["sample"]["sample_time"];
					Serial.print("Sample time loaded from SD: ");
					Serial.println(sm.getState<SampleStateSample>(SampleStateNames::SAMPLE).time);	
					sm.getState<SampleStateSample>(SampleStateNames::SAMPLE).mass
						= doc["sample"]["sample_mass"];
					Serial.print("Sample mass loaded from SD: ");
					Serial.println(sm.getState<SampleStateSample>(SampleStateNames::SAMPLE).mass);
					sm.getState<SampleStateIdle>(SampleStateNames::IDLE).time
						= doc["sample"]["idle_time"];
					Serial.print("Idle time loaded from SD: ");
					Serial.println(sm.getState<SampleStateIdle>(SampleStateNames::IDLE).time);
					sm.getState<SampleStateSetup>(SampleStateNames::SETUP).time
						= doc["sample"]["setup_time"];
					Serial.print("Setup time loaded from SD: ");
					Serial.println(sm.getState<SampleStateSetup>(SampleStateNames::SETUP).time);
					sm.last_cycle = doc["sample"]["last_cycle"];
					Serial.print("Last sample cycle number loaded from SD: ");
					Serial.println(sm.last_cycle);
				}
				if (doc.containsKey("clean")) {
					csm.getState<CleanStateSample>(CleanStateNames::SAMPLE).time
						= doc["clean"]["sample_time"];
					Serial.print("Clean sample time loaded from SD: ");
					Serial.println(csm.getState<CleanStateSample>(CleanStateNames::SAMPLE).time);
					csm.getState<CleanStateIdle>(CleanStateNames::IDLE).time
						= doc["clean"]["idle_time"];
					Serial.print("Clean idle time loaded from SD: ");
					Serial.println(csm.getState<CleanStateIdle>(CleanStateNames::IDLE).time);
					csm.getState<CleanStateFlush>(CleanStateNames::FLUSH).time
						= doc["clean"]["flush_time"];
					Serial.print("Clean flush time loaded from SD: ");
					Serial.println(csm.getState<CleanStateFlush>(CleanStateNames::FLUSH).time);	
					csm.last_cycle = doc["clean"]["last_cycle"];
					Serial.print("Last clean cycle number loaded from SD: ");
					Serial.println(csm.last_cycle);
				}

				// this is an alternative way of doing containsKey
				// which is more efficient when checking indiv objs
				JsonVariant pressure = doc["pressure"]["max_pressure"];
				if (!pressure.isNull()){
					pressure_sensor.max_pressure = pressure;
					Serial.print("Max pressure loaded from SD: ");
					Serial.println(pressure_sensor.max_pressure);
				}
				pressure = doc["pressure"]["min_pressure"];
				if (!pressure.isNull()){
					pressure_sensor.min_pressure = pressure;
					Serial.print("Min pressure loaded from SD: ");
					Serial.println(pressure_sensor.min_pressure);
				}
				if (doc.containsKey("load_cell")) {
					load_cell.factor = doc["load_cell"]["factor"];
					load_cell.offset = doc["load_cell"]["offset"];
					Serial.print("From SD card: Load cell factor ");
					Serial.print(load_cell.factor,6);
					Serial.print(" and offset ");
					Serial.println(load_cell.offset,3);
				}
			} else {
				Serial.println("Error file read");
			}
			file.close();
		} else {
			// createStateFile();
			Serial.println("Error file read");
		}
	}
	// Future: more than two "levels" in loc, new value not int?
	// Rewrites into the JSON and file
	// Make it a template
	void reWrite(const char ** loc, int & value, int new_value) {
		value				= new_value;
		doc[loc[0]][loc[1]] = value;
		std::string contents;
		serializeJson(doc, contents);
		const char * contents_const = contents.c_str();
		if (SD.exists("state.js"))
			SD.remove("state.js");
		File file = SD.open("state.js", FILE_WRITE);
		file.write(contents_const);
		file.close();
	}

	void reWrite(const char ** loc, float & value, float new_value) {
		value				= new_value;
		doc[loc[0]][loc[1]] = value;
		std::string contents;
		serializeJson(doc, contents);
		const char * contents_const = contents.c_str();
		if (SD.exists("state.js"))
			SD.remove("state.js");
		File file = SD.open("state.js", FILE_WRITE);
		file.write(contents_const);
		file.close();
	}
};