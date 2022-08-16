#pragma once
#include <KPFoundation.hpp>
#include <ADS1232.h>
//#include <FileIO/SerialSD.hpp>
#include <time.h>
#include <Application/Constants.hpp>
#include <FileIO/CSVWriter.hpp>
#include <String>
#include <sstream>

#define _dout HardwarePins::DOUT
#define _sclk HardwarePins::SCLK
#define _pdwn HardwarePins::PDWN

class LoadCell : public KPComponent {
public:
	CSVWriter csvw{"data.csv"};
	ADS1232 weight = ADS1232(_pdwn, _sclk, _dout);
	float tare;
	float factor = 0.002348;//0.002324227;

	float offset = -19857.150;//-19691.0843;
	long reading = 0;
	long sum;
	short count;

	LoadCell(const char * name, KPController * controller)
		: KPComponent(name, controller) {}
	void setup() override {
		weight.power_up();
		// reset library values to default
  		weight.OFFSET = 0;
  		weight.SCALE = 1.0;
		tare = 0;

		print("Initial load;");
		println(reTare(50));
	}
	long read(int qty) {
		//println("in read");

		println();
		sum = 0;
		count = 0;
		//display every reading
		for (int i = 0; i < qty; ++i) {
			reading = weight.raw_read(1);
			#ifdef LOAD_CAL
				print("Load reading;");
				print(i);
				print(";");
				println(reading);
			#endif
				if (qty>4){
					//don't include first 5 readings in average due to unreliability
					if (i>3){
						sum += reading;
						count ++;
						//print("tally of readings to average: ");
						//println(count);
					}
				}
				//if there aren't more than 5 readings, just use all of them
				else {
					sum += reading;
					count ++;
				}
			}
			reading = sum/(count);

		return reading;
	}

	float getLoad(int qty) {
		// gets factor and offset from this file during setup, gets factor and offset from SD after
		//println("in getLoad");
		return factor * read(qty) + offset;
	}

	float getLoadPrint(int qty) {
		float load = getLoad(qty);
		char load_string[50];
		sprintf(load_string, "%d.%02u", (int)load, (int)((load - (int)load) * 100));
		std::string strings[2] = {"FLAGGED LOAD, ", load_string};
		csvw.writeStrings(strings, 2);
		println("FLAGGED LOAD;", load_string);
		return load;
	}

	float reTare(int qty) {
		//println("in reTare");
		tare = getLoad(qty);
		return tare;
	}

	float getTaredLoad(int qty) {
		return getLoad(qty) - tare;
	}

	long getVoltage() {
		return weight.raw_read(1);
	}

	float readGrams() {
		//println("in readGrams");
		return factor * (long)weight.raw_read(1) + offset;
	}
};