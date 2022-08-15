#pragma once

#include <KPFoundation.hpp>
#include <Application/Constants.hpp>
#include <Adafruit_NeoPixel.h>
#include <array>
#include <map>

struct Light {
	Light(unsigned short priority, unsigned short r, unsigned short b, unsigned short g)
		: priority(priority), r(r), b(b), g(g){};
	unsigned short priority;
	unsigned short r;
	unsigned short b;
	unsigned short g;
};

namespace LEDNames {
	constexpr const char * IDLE	   = "idle";
	constexpr const char * RUN	   = "run";
	constexpr const char * BATTERY = "battery";
};	// namespace LEDNames

class LED : public KPComponent {
public:
	unsigned short r;
	unsigned short g;
	unsigned short b;
	Adafruit_NeoPixel pixel;
	const unsigned short no_levels		 = 3;
	std::array<Light *, 3> lights_active = {nullptr, nullptr, nullptr};
	std::map<const char *, Light> lights;
	LED(const char * name, KPController * controller)
		: KPComponent(name, controller), pixel(1, HardwarePins::PIXEL, NEO_RGB + NEO_KHZ800) {}
	void setup() {
		pixel.begin();
		setIdle();

		lights.insert(std::make_pair(LEDNames::BATTERY, Light(0, 30, 0, 0)));
		lights.insert(std::make_pair(LEDNames::IDLE, Light(2, 0, 0, 30)));
		lights.insert(std::make_pair(LEDNames::RUN, Light(2, 0, 30, 0)));
	}

	void setColor(unsigned short nr, unsigned short ng, unsigned short nb) {
		pixel.clear();
		r = nr;
		g = ng;
		b = nb;
		pixel.setPixelColor(0, g, r, b);
		pixel.show();
	}

	void refreshLights() {
		bool finished = false;
		for (int i = 0; i < no_levels && false == finished; ++i) {
			if (nullptr != lights_active.at(i)) {
				setColor(lights_active.at(i)->r, lights_active.at(i)->g, lights_active.at(i)->b);
				finished = true;
			}
		}
	}

	void setLight(Light * lt) {
		lights_active.at(lt->priority) = lt;
		refreshLights();
	}

	void setLight(const char * name) {
		setLight(&(lights.at(name)));
	}

	void strip() {
		lights_active.at(0) = nullptr;
		lights_active.at(1) = nullptr;
	}

	void setRun() {
		setColor(0, 30, 0);
	}
	void setIdle() {
		setColor(0, 0, 30);
	}
	void setSoftwareError() {}	//
	void setBlockage() {
		setColor(15, 15, 0);
	}
	void setFinished() {
		setIdle();
	}
	void setClean() {
		setColor(10, 10, 10);
	}
};