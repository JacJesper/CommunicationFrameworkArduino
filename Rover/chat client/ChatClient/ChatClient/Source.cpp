#include <thread>

#include "C:\Users\jacob\Documents\CanSat 2019\Final\Ground Station\PC\GroundStationPC\GroundStationPC\ArduinoSerial.h"

#define STR2(x) #x
#define STR(x) STR2(x)


void PrintProgramOutput(std::string msg) {
	printf(">> %s\n", msg.c_str());
}

bool ContinueLoop;

int bPCInputData;
char PCInputData[DATA_SIZE];

int SerialLoop(std::string ComPort) {
	ArduinoSerial* as = new ArduinoSerial();
	int i = as->ConnectToArduino(ComPort);
	if (i == 0) {
		PrintProgramOutput("Connected to arduino!");
	}
	else {
		PrintProgramOutput("Couldnt connect to the supplied com port, exiting...");
		return -1;
	}

	Placeholder p;

	while (ContinueLoop) {
		DWORD start_time = GetTickCount();

		if (bPCInputData) {
			bPCInputData = false;

			// Send a data packet to the arduino

			Data dataTemp;
			memcpy(dataTemp.data, PCInputData, DATA_SIZE);
			dataTemp.message_type = MSG_TYPE_DATA;

			as->SendData(dataTemp);
		}

		p = as->ReadFromSerial();
		switch (p.message_type) {
		case MSG_TYPE_ERROR: // Some kind of error
			printf("\b\b\b\b\b\b\b\b\b>>> ERROR: %.*s\nYou - ", DATA_SIZE, p.data);
			break;
		case MSG_TYPE_NOTHING: // Nothing to read
			break;
		case MSG_TYPE_DATA: { // data from rover
			Data temp;
			memcpy(&temp, &p, sizeof(Data));
			printf("\b\b\b\b\b\b\b\b\b>Ground Station - %.*s\nYou - ", DATA_SIZE, temp.data);

			//			MarsPicture->WriteJPGFile(temp);
		} break;
		default:
			printf("\b\b\b\b\b\b\b\b\b>>> Unknown message type received, moving on...\nYou - ");
		}

		if (as->GetElementsInReadQue() == 0 && (GetTickCount() - start_time) < CYCLE_TIME) {
			Sleep(CYCLE_TIME - (GetTickCount() - start_time));
		}
	}

	return 0;
}

int main(int argc, char* argv[]) {
	printf("sizeof() [X]: %d\n\n", sizeof(Ping));

	ContinueLoop = true;

	std::string ComPort = argv[1];

	std::thread SerialThread(SerialLoop, ComPort);

	Sleep(30);

	while (ContinueLoop) {
		int instruction = 0;

		char string_buffer[DATA_SIZE];
		memset(string_buffer, '\0', DATA_SIZE);
		printf("You - ");
		fgets(string_buffer, DATA_SIZE, stdin);
		string_buffer[strcspn(string_buffer, "\n")] = 0;

		bPCInputData = true;
		memcpy(PCInputData, string_buffer, sizeof(Data));
	}

	return 0;
}