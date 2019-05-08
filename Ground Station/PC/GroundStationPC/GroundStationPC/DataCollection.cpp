#include "DataCollection.h"

DataCollection::DataCollection(std::string FileName, std::string FileType, std::string FirstLine) { // First line is meant for a CSV style document
	char FILENAME[MAX_PATH];
	sprintf(FILENAME, "%s.%s", FileName.c_str(), FileType.c_str()); // Creates .csv file
	PersistentFileName = FILENAME;

	FileHandle = fopen(PersistentFileName.c_str(), "wx"); // Creates a new file, and if a file with the name already exists, it will throw error;
	if (FileHandle == NULL) { // If the file already exists
		for (int i = 1; i < MAX_FILE_NUMBER; i++) { // Cycles through all the numbers it and tries to create a filename that isnt taken.
			char temp_name[MAX_PATH];
			memset(temp_name, 0, MAX_PATH);
			sprintf(temp_name, "%s_%d.%s", FileName.c_str(), i, FileType.c_str());
			PersistentFileName = temp_name;
			FileHandle = fopen(PersistentFileName.c_str(), "wx");

			if (FileHandle != NULL)
				break;
		}
	}
	
	int len = FirstLine.length();
	if (FirstLine.length() > 0) {
		if (FirstLine.back() != '\n') { // Create check if there even is a 'FirstLine'
			FirstLine.push_back('\n');
		}

		fputs(FirstLine.c_str(), FileHandle);
		fflush(FileHandle);
	}
}

DataCollection::~DataCollection() {
	fclose(FileHandle);
}

bool DataCollection::CheckFile() {
	return FileHandle != NULL;
}

int DataCollection::WriteBMP280File(BMP280SensorData source) { // Not sure how to write this in a modular way, make one for every datatype ???
	int rValue = 0;

	rValue = fprintf(FileHandle, "%d,%d,%d,%d\n", source.timestamp_1, source.pres, source.temp, source.hum);
	if (rValue > 0) {
		fflush(FileHandle); // Writes it to the file
		return rValue;
	}

	return -1;
}

int DataCollection::WritePingPongFile(Ping source) { // Not sure how to write this in a modular way, make one for every datatype ???
	int rValue = 0;

	rValue = fprintf(FileHandle, "%d,%d,%d\n", source.timestamp, source.roundabout, source.RSSI);
	if (rValue > 0) {
		fflush(FileHandle); // Writes it to the file
		return rValue;
	}

	return -1;
}