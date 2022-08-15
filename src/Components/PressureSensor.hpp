#pragma once
#include <MS5803_02.h>
#include <KPFoundation.hpp>
#include <application/Constants.hpp>
#include <Wire.h>
#define PRESSURE_ADDR 0x77

inline bool checkForConnection(unsigned char addr) {
	Wire.begin();
	Wire.requestFrom(addr, 1);
	return Wire.read() != -1;
}

class PressureSensor : public KPComponent {
public:
	bool connected;
	MS_5803 sensor;
	int min_pressure			  = DefaultPressures::MIN_PRESSURE;
	int max_pressure			  = DefaultPressures::MAX_PRESSURE;

	PressureSensor(const char * name, KPController * controller)
		: KPComponent(name, controller), sensor(PRESSURE_ADDR) {}

	void setup() override {
		if (sensor.initializeMS_5803()) {
			Serial.println("OK: MS5803 pressure sensor online");
		} else {
			Serial.println("ERR: MS5803 pressure sensor offline");
		}
	}

	void update() override {}

	float getPressure() {
		sensor.readSensor();
		return sensor.pressure();
	}

	float getTemp() {
		sensor.readSensor();
		return sensor.temperature();
	}

	int error_code() {
		return 0;
	}

	bool isWithinPressure() {
		float sum = 0;
		int qty = 5;
		int count = 0;
		for (int i = 0; i < qty; ++i) {
			float p_inst = getPressure();
			#ifdef PRESSURE_CHECK
				print("Pressure instant mbar;;;;; ");
				println(p_inst);
			#endif
			if (p_inst>0){
				count = count +1;
				sum += p_inst;
			}
		}
		float p_avg = sum/count;
		print("Pressure average mbar;;;;; ");
		println(p_avg);
		if (p_avg >= min_pressure && p_avg <= max_pressure) {
			return true;
		} else {
			Serial.print("Not within pressure, value: ");
			Serial.println(p_avg);
			return false;
		}
	}
};