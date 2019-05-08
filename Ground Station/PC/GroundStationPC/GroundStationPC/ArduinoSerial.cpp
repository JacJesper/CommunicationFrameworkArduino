#include "ArduinoSerial.h"

ArduinoSerial::ArduinoSerial() {
	this->bConnected = false;
}

ArduinoSerial::~ArduinoSerial() {
	CloseHandle(serialDeviceHandle);
}



int ArduinoSerial::ConnectToArduino(std::string ComPort) { // Sets up the device handle to communicate with an arduino on the specified ComPort

	bool tRes = true; // Used for error checking

	serialDeviceHandle = CreateFile(ComPort.c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (serialDeviceHandle == INVALID_HANDLE_VALUE) {
		return -1; // Caller should yse GetLastError when -1 is returned
	}

	tRes = GetCommState(serialDeviceHandle, &serialDeviceSettings);
	if(tRes == NULL){
		return -1;
	}

	serialDeviceSettings.BaudRate = CBR_115200;
	serialDeviceSettings.ByteSize = 8;
	serialDeviceSettings.Parity = NOPARITY;
	serialDeviceSettings.fDtrControl = DTR_CONTROL_ENABLE;
	serialDeviceSettings.StopBits = ONESTOPBIT;

	tRes = SetCommState(serialDeviceHandle, &serialDeviceSettings);
	if (tRes == NULL) {
		return -1;
	}

	tRes = PurgeComm(serialDeviceHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
	if (tRes == NULL) {
		return -1;
	}

	bConnected = true;

	return 0;
}


Placeholder ArduinoSerial::ReadFromSerial() { // 

	int nResult = false; // Used for errorchecking only
	DWORD SerialDeviceErrors = 0;
	DWORD BytesRead;

	bool SerialRXNotAlligned = false;

	Placeholder ReturnValue;
	memset(&ReturnValue, NULL, sizeof(Placeholder));

	nResult = ClearCommError(serialDeviceHandle, &SerialDeviceErrors, &SerialDeviceInfo); // Gets an update in the serial port, ie. gets the que
	if (nResult == NULL) {
		memset(&ReturnValue, 0, sizeof(Placeholder)); // Something went wrong! 
		memcpy(&ReturnValue.data, "NO ACCESS SERIAL", DATA_SIZE);
		return ReturnValue; // Returns a nullified version of a placeholder object
	}

	// Actual reading of the serial:

	if (SerialDeviceInfo.cbInQue >= sizeof(Placeholder)) { // Tests if there is enough data to read an entire packet
		if (SerialDeviceInfo.cbInQue % sizeof(Placeholder) > 0) { // If number of bytes doesn't lign up with Placeholder data type
			if (AllignRXSerial(SerialDeviceInfo.cbInQue) == 0) // if it alligned
				SerialRXNotAlligned = true;
			else {
				PurgeRX(); // Couldnt line up, so delete everything and move on
				memset(&ReturnValue, 0, sizeof(Placeholder));
				memcpy(&ReturnValue.data, "PACKETS NO MATCH", DATA_SIZE);
				return ReturnValue;
			}
				
		}

		char* buffer = (char*) malloc(SerialDeviceInfo.cbInQue);
		
		if(SerialRXNotAlligned) // Missmatched by 2 bytes
			nResult = ReadFile(serialDeviceHandle, &ReturnValue.pid, (sizeof(Placeholder) - sizeof(short)), &BytesRead, NULL); // Gets the data
		else
			nResult = ReadFile(serialDeviceHandle, &ReturnValue, sizeof(Placeholder), &BytesRead, NULL); // Gets the data
		if (nResult == NULL) {
			memset(&ReturnValue, 0, sizeof(Placeholder));
			ReturnValue.message_type = 19;
			memcpy(&ReturnValue.data, "NO ACCESS SERIAL", DATA_SIZE);
			return ReturnValue;
		}
	}
	else { // if no data in the que
		memset(&ReturnValue, 0, sizeof(Placeholder));
		ReturnValue.message_type = 19;
		return ReturnValue;
	}

	return ReturnValue;
}


// #################

DWORD ArduinoSerial::SendInstruction(Instruction instruction) {

	int tRes = 0;

	DWORD bytesWritten;
	
	tRes = PurgeComm(serialDeviceHandle, PURGE_TXCLEAR); // Clear the write buffer
	if (tRes == NULL) {
		return -1;
	}

	tRes = WriteFile(serialDeviceHandle, &instruction, sizeof(Instruction), &bytesWritten, NULL);
	if (tRes == NULL)
		return -1;

	return bytesWritten;
}

DWORD ArduinoSerial::SendData(Data sData) {
	int tRes = 0;

	DWORD bytesWritten;

	tRes = PurgeComm(serialDeviceHandle, PURGE_TXCLEAR); // Clear the write buffer
	if (tRes == NULL) {
		return -1;
	}

	tRes = WriteFile(serialDeviceHandle, &sData, sizeof(Data), &bytesWritten, NULL);
	if (tRes == NULL)
		return -1;

	return bytesWritten;
}

bool ArduinoSerial::PurgeRX() {
	return PurgeComm(serialDeviceHandle, PURGE_RXCLEAR);
}


int ArduinoSerial::AllignRXSerial(int SizeOfQueue) {
	short buffer;
	DWORD BytesRead; // doesnt matter, leave it be;
	DWORD SerialDeviceErrors;
	int nResult;
	bool DataInRXBuffer = false;
	int tSizeOfQueue = SizeOfQueue;

		
	while (tSizeOfQueue >= sizeof(Placeholder)) {
		nResult = ReadFile(serialDeviceHandle, &buffer, sizeof(buffer), &BytesRead, NULL); // 2 bytes of data

		if (nResult) { // if it succeeded
			if (buffer == LINEUP) // Checks if it read the lineup bytes
				return 0; // Return success
			
			if (tSizeOfQueue >= sizeof(Placeholder)) // more data in rx
				DataInRXBuffer = true;
			else
				return -1;

			tSizeOfQueue -= sizeof(buffer);
		}
		else
			return -1; // ReadFile failed
	}
}