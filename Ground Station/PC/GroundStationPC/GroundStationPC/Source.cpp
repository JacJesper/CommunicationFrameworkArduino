#include <thread>


#include "ArduinoSerial.h"
#include "DataCollection.h"
// #include "ImageCollector.h"

#define STR2(x) #x
#define STR(x) STR2(x)


void PrintProgramOutput(std::string msg) {
	printf(">> %s\n", msg.c_str());
}

int PCInputInstruction;
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

	short InstructionCounter = 0;
	Instruction ins;

	DataCollection* SatelliteData = new DataCollection("BMP280SensorData", "csv", "Time,Pressure,Temperature,Humidity");
	DataCollection* PingPongData = new DataCollection("PingPongData", "csv", "Time,Ping time,RSSI");

	Placeholder p;
	bool AutoPing = false;

	while (ContinueLoop) {
		DWORD start_time = GetTickCount();

		if (PCInputInstruction) {
			if (PCInputInstruction >= 1 && PCInputInstruction <= 5) {
				Instruction ins;
				ins.instruction = (short) PCInputInstruction;

				as->SendInstruction(ins);

				if (PCInputInstruction == INSTRUCTION_START_AUTO_PING)
					AutoPing = AutoPing ? false : true;
			}

			PCInputInstruction = 0;
		}

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
			printf("\b\b\b\b\b\b\b\b\b>> ERROR: %.*s\nYou - ", DATA_SIZE, p.data);
			break;
		case MSG_TYPE_NOTHING: // Nothing to read
			break;
		case MSG_TYPE_PING: {
			// Add a file for ping pong data
			Ping ping_temp;
			memcpy(&ping_temp, &p, sizeof(Placeholder));
			PingPongData->WritePingPongFile(ping_temp);
			if(!AutoPing)
				printf("\b\b\b\b\b\b\b\b\b>> Ping received, roundabout time: %d\nYou - ", ping_temp.roundabout);

		} break;
		case MSG_TYPE_DATA: { // data from rover
			Data temp;
			memcpy(&temp, &p, sizeof(Data));
			printf("\b\b\b\b\b\b\b\b\bRover - %.*s\nYou - ", DATA_SIZE, temp.data);

//			MarsPicture->WriteJPGFile(temp);
		} break;
		case MSG_TYPE_BMP280: {
			BMP280SensorData temp_bmp;
			memcpy(&temp_bmp, &p, sizeof(BMP280SensorData));
			SatelliteData->WriteBMP280File(temp_bmp);
			break;
		}
		case MSG_TYPE_INSTRUCTION: {
			Instruction temp_instruction;
			memcpy(&temp_instruction, &p, sizeof(Instruction));
			printf("\b\b\b\b\b\b\b\b\b>> Response to instruction: %s\nYou - ", temp_instruction.response);
		} break;
		default:
			printf("\b\b\b\b\b\b\b\b\b>> Unknown message type received, moving on...\nYou - ");
		}
		
		if (as->GetElementsInReadQue() == 0 && (GetTickCount() - start_time) < CYCLE_TIME) {
			Sleep(CYCLE_TIME - (GetTickCount() - start_time));
		}
	}

		return 0;
}

int main(int argc, char* argv[]) {
	printf("sizeof() [X]: %d\n\n", sizeof(Ping));

	static bool INTERRUPT = false;

	PCInputInstruction = 0;
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

		if (string_buffer[0] == '/') {
			std::string temp = string_buffer;
			PCInputInstruction = atoi(temp.substr(1).c_str());
		}
		else {
			bPCInputData = true;
			memcpy(PCInputData, string_buffer, DATA_SIZE);
		}
	}

	return 0;
}