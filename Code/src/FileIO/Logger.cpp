#include <FileIO/Logger.hpp>
#include <Application/Application.hpp>
#include <time.h>
#include <sstream>

// Write out to debug.csv the time, currentStateName, and pressure.
void Logger::log() {
	// Only every buffer_time millis
	if (millis() - last_time > buffer_time) {
#ifdef WRITERDEBUG
		Serial.print("Writing...");
#endif
		Application & app  = *static_cast<Application *>(controller);
		const auto timenow = app.clock.getTime();
		std::stringstream ss;
		ss << timenow;
		std::string time_string = ss.str();

		const auto pressnow = app.pressure_sensor.getPressure();
		std::stringstream ps;
		ps << pressnow;
		std::string current_pressure = ps.str();

		std::string strings[5] = {time_string," ", app.sm.getCurrentStateName(),", ", current_pressure};
		writer.writeStrings(strings, 5);
		last_time = millis();
	}
}

void Logger::writeStrings(const std::string * strs, int len) {
	writer.writeStrings(strs, len);
}

void Logger::update() {
	Application & app = *static_cast<Application *>(controller);
	if (app.sm.isRunning())
		log();
}