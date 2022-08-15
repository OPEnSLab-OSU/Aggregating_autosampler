#pragma once

namespace HardwarePins {
	constexpr int SHFT_REG_LATCH = 9;
	constexpr int SHFT_REG_CLOCK = 11;
	constexpr int SHFT_REG_DATA	 = 12;
	constexpr int MOTOR_REVERSE	 = 5;
	constexpr int MOTOR_FORWARDS = 6;
	constexpr int RUN_BUTTON	 = A5;
	constexpr int CLEAN_BUTTON	 = 13;
	constexpr int SD			 = 10;
	constexpr int DOUT			 = A3;
	constexpr int SCLK			 = 0;
	constexpr int PDWN			 = 1;
	constexpr int PIXEL		  	 = A4;
}  // namespace HardwarePins

namespace DefaultTimes {
	const int FLUSH_TIME  = 50;
	const int SAMPLE_TIME = 60;
	const int IDLE_TIME	  = 3600 - FLUSH_TIME - SAMPLE_TIME;
	const int SETUP_TIME  = 0;
}  // namespace DefaultTimes

namespace DefaultTimesClean {
	const int SAMPLE_TIME = 5;
	const int IDLE_TIME	  = 0;
	const int FLUSH_TIME  = 20;
}  // namespace DefaultTimesClean

namespace DefaultTimesButton {
	const unsigned long DEBOUNCE_TIME = 100;
}

namespace TPICDevices {
	constexpr int INTAKE_POS  = 0;
	constexpr int INTAKE_NEG  = 1;
	constexpr int FLUSH_VALVE = 3;
	constexpr int WATER_VALVE = 2;
}  // namespace TPICDevices

namespace DefaultPressures {
	constexpr int MIN_PRESSURE = 400;	// 600
	constexpr int MAX_PRESSURE = 1300;	// 990
}  // namespace DefaultPressures