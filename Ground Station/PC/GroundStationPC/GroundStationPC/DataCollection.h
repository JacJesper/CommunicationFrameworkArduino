#pragma once

#include <string>
#include <stdio.h>
#include <Windows.h>

#include "C:\Users\jacob\Documents\CanSat 2019\Final\Headers\Structs.h"


#define MAX_FILE_NUMBER 99
 

class DataCollection {

	std::string PersistentFileName;
	FILE* FileHandle;

public:
	DataCollection(std::string FileName, std::string FileType, std::string FirstLine);
	~DataCollection();

	bool CheckFile();

	std::string GetFileName() {
		return PersistentFileName;
	}

	int WriteBMP280File(BMP280SensorData source);
	int WritePingPongFile(Ping source);

};



