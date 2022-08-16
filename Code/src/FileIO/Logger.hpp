#pragma once
#include <FileIO/CSVWriter.hpp>
#include <KPFoundation.hpp>
#include <string>
class Logger : public KPComponent {
public:
	CSVWriter writer;
	unsigned long last_time;
	unsigned long buffer_time;
	Logger(const char * name, KPController * controller, int buffer_time = 5000)
		: KPComponent(name, controller), writer("debug.csv"), buffer_time(buffer_time) {
		last_time = millis();
	}
	void setup() override {}
	void update();
	void log();
	void writeStrings(const std::string * strs, int len);
};