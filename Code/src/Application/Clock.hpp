#pragma once

#include <KPFoundation.hpp>
#include <DS3232RTC.h>
#include <Application/Constants.hpp>
#include <Wire.h>
#include <SPI.h>
#include <time.h>

#define RTC_ADDR 0x68

class Clock : public KPComponent {
public:
	DS3232RTC rtc;
	Clock(const char * name) : KPComponent(name), rtc(false) {}
	void setup() {
		//waitForConnection();
		rtc.begin();
		rtc.squareWave(SQWAVE_NONE);
		setTime(rtc.get());
	}
	void set(unsigned long long seconds) {
		rtc.set(seconds);
		setTime(rtc.get());
	}

	time_t cmpTime(time_t cmp) {
		return cmp - now();
	}

	time_t cmpTime(const tmElements_t & cmp) {
		return makeTime(cmp) - now();
	}

	time_t getTime() {
		return now();
	}

	// don't need
	void waitForConnection() {
		Wire.begin();
		for (;; delay(5000)) {
			Wire.requestFrom(RTC_ADDR, 1, false);
			if (Wire.read() == -1) {
				Serial.println("RTC not connected.");
			} else {
				Serial.println("RTC connected");
				break;
			}
		}

		Wire.end();
	}
};
