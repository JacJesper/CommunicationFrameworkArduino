#include "ImageCollector.h"

ImageCollector::ImageCollector(std::string FileName, std::string FileType) { // First line is meant for a CSV style document
	char FILENAME[MAX_PATH];
	sprintf(FILENAME, "%s.%s", FileName.c_str(), FileType.c_str()); // Creates .csv file
	PersistentFileName = FILENAME;

	FileHandle = fopen(PersistentFileName.c_str(), "wbx"); // Creates a new file, and if a file with the name already exists, it will throw error;
	if (FileHandle == NULL) { // If the file already exists
		for (int i = 1; i < MAX_FILE_NUMBER; i++) { // Cycles through all the numbers it and tries to create a filename that isnt taken.
			char temp_name[MAX_PATH];
			memset(temp_name, 0, MAX_PATH);
			sprintf(temp_name, "%s_%d.%s", FileName.c_str(), i, FileType.c_str());
			PersistentFileName = temp_name;
			FileHandle = fopen(PersistentFileName.c_str(), "wbx");

			if (FileHandle != NULL)
				break;
		}
	}
}

ImageCollector::~ImageCollector() {
	fclose(FileHandle);
}

bool ImageCollector::CheckFile() {
	return FileHandle != NULL;
}

int ImageCollector::WriteJPGFile(Data source) { // Not sure how to write this in a modular way, make one for every datatype ???
	int rValue = 0;

	char* buffer = (char*) malloc(sizeof(Data));
	memcpy(buffer, &source, sizeof(Data));

	rValue = fwrite(buffer, sizeof(char), sizeof(Data), FileHandle);
	if (rValue > 0) {
		fflush(FileHandle); // Writes it to the file
		free(buffer);
		return rValue;
	}

	free(buffer);
	return -1;
}