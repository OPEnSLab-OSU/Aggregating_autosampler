#include <Components/Shell.hpp>
#include <Application/Application.hpp>
#include <Application/Constants.hpp>
#include <KPFoundation.hpp>
#include <SD.h>
#include <ArduinoJson.h>
#include <time.h>
#define cmnd_lambda [](Application & app, const std::string * args)
#define CALL		(app, args)

namespace Utility {
	// returns true if str is a pos int
	// assumes string correctness
	bool is_posint(const char * str) {
		for (int i = 0; str[i] != '\0'; ++i) {
			if (str[i] > '9' || str[i] < '0') {
				return false;
			}
		}
		return true;
	}

	// returns true if str is a pos int
	// displays a msg if it isn't
	bool msg_posint(const char * str, int arg) {
		bool posint = is_posint(str);
		if (!posint) {
			Serial.print("Error: argument ");
			Serial.print(arg);
			Serial.print(" needs to be a pos int.");
		}
		return posint;
	}

	std::string readEntireFile(File & file) {
		std::string contents;
		while (-1 != file.peek()) {
			contents.push_back(file.read());
		}
		return contents;
	}
}  // namespace Utility

void Shell::runFunction(const std::string * args, const unsigned short length) {
	if (commands.find(args[0]) != commands.end()) {
		if (length - 1 == commands.at(args[0]).n_args) {
			Application & app = *static_cast<Application *>(controller);
			commands.at(args[0]).function(app, args);
		} else {
			Serial.print("Bad arguments\n");
		}
	} else {
		Serial.print("Bad command.\n");
	}
}

/*
	addFunction(
		"name",
		n args,
		cmnd_lambda { function });

	You will have access to app and args[]. args[0] should always
	be the same as the name of the command. They are C++ strings
*/

void Shell::setup() {
	// run button
	addFunction(
		"sample_button_press",
		0,
		cmnd_lambda { app.sm.begin(); });

	// clean button
	addFunction(
		"clean_button_press",
		0,
		cmnd_lambda { app.csm.begin(); });

	// halt the machine in the sample state
	addFunction(
		"sample_halt",
		0,
		cmnd_lambda { app.sm.halt(); });

	// halt the clean state machine
	addFunction(
		"clean_halt",
		0,
		cmnd_lambda { app.csm.halt(); });

	// halt all state machines
	addFunction(
		"halt",
		0,
		cmnd_lambda {
			app.sm.halt();
			app.csm.halt();
		});

	// print free ram
	addFunction(
		"mem",
		0,
		cmnd_lambda { Serial.println(free_ram()); });

	addFunction(
		"state_read",
		0,
		cmnd_lambda {
			File file = SD.open("state.js", FILE_READ);
			Serial.println(Utility::readEntireFile(file).c_str());
			file.close();
		});

	addFunction(
		"check_sample_flush_time",
		0,
		cmnd_lambda {
			Serial.println(app.sm.getState<SampleStateFlush>(SampleStateNames::FLUSH).time);
		});

	addFunction(
		"set_time",
		1,
		cmnd_lambda {
			if (Utility::msg_posint(args[1].c_str(), 1)) {
				app.power.set(std::stoi(args[1]));
			}
		});
	addFunction(
		"get_time",
		0,
		cmnd_lambda {
			std::string time = std::to_string(app.power.getTime());
			Serial.println(time.c_str());
		});

	addFunction(
		"get_pressure",
		0,
		cmnd_lambda { Serial.println(app.pressure_sensor.getPressure()); });

	addFunction(
		"get_load",
		0,
		cmnd_lambda { Serial.println(app.load_cell.getLoad()); });

	addFunction(
		"get_tared_load",
		0,
		cmnd_lambda { Serial.println(app.load_cell.getTaredLoad()); });

	addFunction(
		"volt_load",
		0,
		cmnd_lambda { Serial.println(app.load_cell.getVoltage()); });

	addFunction(
		"sample_no_runs",
		1,
		cmnd_lambda {
			const char * loc[2] = {"sample", "last_cycle"};
			app.reWrite(loc, app.sm.last_cycle, std::stoi(args[1]));
		});

	addFunction(
		"sample_flush_time",
		1,
		cmnd_lambda {
			const char * loc[2] = {"sample", "flush_time"};
			app.reWrite(loc,
				app.sm.getState<SampleStateFlush>(SampleStateNames::FLUSH).time,
				std::stoi(args[1]));
		});

	addFunction(
		"sample_sample_time",
		1,
		cmnd_lambda {
			const char * loc[2] = {"sample", "sample_time"};
			app.reWrite(loc,
				app.sm.getState<SampleStateSample>(SampleStateNames::SAMPLE).time,
				std::stoi(args[1]));
		});

	addFunction(
		"sample_sample_volume",
		1,
		cmnd_lambda {
			const char * loc[2] = {"sample", "sample_volume"};
			app.reWrite(loc,
				app.sm.getState<SampleStateSample>(SampleStateNames::SAMPLE).volume,
				std::stoi(args[1]));
		});

	addFunction(
		"sample_idle_time",
		1,
		cmnd_lambda {
			const char * loc[2] = {"sample", "idle_time"};
			app.reWrite(loc,
				app.sm.getState<SampleStateIdle>(SampleStateNames::IDLE).time,
				std::stoi(args[1]));
		});

	addFunction(
		"sample_between_time",
		1,
		cmnd_lambda {
			const char * loc[2] = {"sample", "idle_time"};
			app.reWrite(loc,
				app.sm.getState<SampleStateIdle>(SampleStateNames::IDLE).time,
				std::stoi(args[1])
					- app.sm.getState<SampleStateOnramp>(SampleStateNames::ONRAMP).time
					- app.sm.getState<SampleStateFlush>(SampleStateNames::FLUSH).time
					- app.sm.getState<SampleStateSample>(SampleStateNames::SAMPLE).time);
		});

	addFunction(
		"sample_setup_time",
		1,
		cmnd_lambda {
			const char * loc[2] = {"sample", "setup_time"};
			app.reWrite(loc,
				app.sm.getState<SampleStateSetup>(SampleStateNames::SETUP).time,
				std::stoi(args[1]));
		});

	addFunction(
		"sample_setup_tod_enabled",
		1,
		cmnd_lambda {
			const char * loc[2] = {"sample", "setup_tod_enabled"};
			app.reWrite(loc,
				app.sm.getState<SampleStateSetup>(SampleStateNames::SETUP).tod_enabled,
				std::stoi(args[1]));
		});
	addFunction(
		"sample_setup_tod",
		1,
		cmnd_lambda {
			const char * loc[2] = {"sample", "setup_tod"};
			app.reWrite(loc,
				app.sm.getState<SampleStateSetup>(SampleStateNames::SETUP).tod,
				std::stoi(args[1]));
		});
	addFunction(
		"min_pressure",
		1,
		cmnd_lambda {
			const char * loc[1] = {"min_pressure"};
			app.reWrite(loc, app.pressure_sensor.min_pressure, std::stoi(args[1]));
		});

	addFunction(
		"max_pressure",
		1,
		cmnd_lambda {
			const char * loc[1] = {"max_pressure"};
			app.reWrite(loc, app.pressure_sensor.max_pressure, std::stoi(args[1]));
		});

	addFunction(
		"led_set",
		1,
		cmnd_lambda { app.led.setLight(args[1].c_str()); });
	addFunction(
		"led_clear",
		0,
		cmnd_lambda { app.led.strip(); });

	addFunction(
		"file_read",
		1,
		cmnd_lambda {
			File file = SD.open(args[1].c_str());
			Serial.println(Utility::readEntireFile(file).c_str());
			file.close();
		});

	addFunction(
		"temp_read",
		0,
		cmnd_lambda { Serial.println(app.pressure_sensor.getTemp()); });

	addFunction(
		"shift_manip",
		2,
		cmnd_lambda {
			app.shift.setAllRegistersLow();
			app.shift.setPin(std::stoi(args[1]), stoi(args[2]));  // write in skinny
			app.shift.write();									  // write shifts wide*/
		});

	addFunction(
		"pin_manip",
		2,
		cmnd_lambda { digitalWrite(stoi(args[1]), stoi(args[2])); });
	// Note: Requires restart
	// addFunction("factory_file_reset", 0, cmnd_lambda{/*app.createStateFile();*/});
}

void Shell::addFunction(const char * name, const unsigned short n_args, ShellSpace::func function) {
	ShellSpace::func_args entry;
	entry.function = function;
	entry.n_args   = n_args;
	commands[name] = entry;
}

/*

struct BadArgs : public std::exception {
const char * what() const throw() {
	return "Bad args";
}
};
try {
	ShellSpace::func_args access = commands.at(args[0]);
	if (length != access.args) {
		throw BadArgs();
	} else {
		Application & app = *static_cast<Application *>(controller);
		access.function(app, args);
	}
} catch (std::exception & e) {
	Serial.print("Exception: ");
	Serial.print(e.what());
	Serial.print("\n");
}*/