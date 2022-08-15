#pragma once
#include <KPFoundation.hpp>
#include <SD.h>
#include <string>

class CSVWriter {
public:
	const char * dir;
	CSVWriter(const char * dir) : dir(dir) {
	}
	void writeStrings(const std::string * strs, int len) {
		File file = SD.open(dir, FILE_WRITE);
		for (int i = 0; i < len; ++i) {
			file.print(strs[i].c_str());
			//if (i != len - 1)
			//	file.print(" ");
		}
		file.write("\n");
#ifdef WRITERDEBUG
		if (file) {
			Serial.print("Written to ");
			Serial.print(dir);
			Serial.print("\n");
		}
#endif
		file.close();
	}
};