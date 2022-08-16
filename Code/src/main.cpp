#include <Application/Application.hpp>
#ifdef WATCHDOG
	#include <Adafruit_SleepyDog.h>
#endif
Application app;

void setup() {
	app.setup();

	// Watchdog timer
	#ifdef WATCHDOG
		Watchdog.enable(12000);
	#endif
}

void loop() {
	app.update();

	// Watchdog timer
#ifdef WATCHDOG
	Watchdog.reset();
#endif
}