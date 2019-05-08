#pragma once

#include <string>
#include <stdio.h>
#include <Windows.h>

#include "C:\Users\jacob\Documents\CanSat 2019\Final\Headers\Structs.h"


#define MAX_FILE_NUMBER 99


class ImageCollector {

	std::string PersistentFileName;
	FILE* FileHandle;

public:
	ImageCollector(std::string FileName, std::string FileType);
	~ImageCollector();

	bool CheckFile();

	std::string GetFileName() {
		return PersistentFileName;
	}

	int WriteJPGFile(Data source);

};



