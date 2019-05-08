#pragma once

#include <string>
#include <windows.h>

#include "C:\Users\jacob\Documents\CanSat 2019\Final\Headers\Structs.h"

#define BUFFER_READ_SIZE 24
#define CYCLE_TIME 50

class ArduinoSerial {
private:
	bool bConnected;

	DCB serialDeviceSettings;
	HANDLE serialDeviceHandle;
	COMSTAT SerialDeviceInfo;

	int AllignRXSerial(int SizeOfQueue); // 0: success, !0 failure
public:

	ArduinoSerial();
	~ArduinoSerial();

	int ConnectToArduino(std::string ComPort); // 0: success
	
	Placeholder ReadFromSerial();
	DWORD SendInstruction(Instruction sInstruction); 
	DWORD SendData(Data sData);

	bool PurgeRX();
	bool PurgeTX();


	bool Connected() { // Check if the arduino is still there
		return bConnected;
	}

	int GetElementsInReadQue() {
		return SerialDeviceInfo.cbInQue / sizeof(Placeholder);
	}
};