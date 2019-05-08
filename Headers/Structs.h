#ifndef STRUCTS_H
#define STRUCTS_H

#define LINEUP 0x55AA

#define DATA_SIZE 16 // Just easier... 
#define TOTAL_PACKET_SIZE 24
// define ARDUINO on the arduinos

///////////////////
// Output structs
///////////////////

enum {
	MSG_TYPE_ERROR = 0,
	MSG_TYPE_DATA = 1,
	MSG_TYPE_INSTRUCTION = 2,
	MSG_TYPE_BMP280 = 3,
	MSG_TYPE_SYNC_CLOCK = 4,
	MSG_TYPE_PING = 7,
	MSG_TYPE_NOTHING = 19
};

enum {
	INSTRUCTION_START_AUTO_PING = 1,
	INSTRUCTION_SET_TIMER = 2,
	INSTRUCTION_SYNC_CLOCK = 3,
	INSTRUCTION_START_STOP_TRANSMIT_SATELLITE = 4,
	INSTRUCTION_PING = 5
};

#ifdef ARDUINO
typedef struct {
	int16_t _lineup = LINEUP;
	int16_t pid;
	unsigned long message_type = MSG_TYPE_INSTRUCTION;

	int16_t instruction;
	int8_t response[14]; // padding
} Instruction;
#else
typedef struct {
	__int16 _lineup = LINEUP;
	__int16 pid;
	unsigned long message_type = MSG_TYPE_INSTRUCTION;

	__int16 instruction;
	__int8 response[14]; // padding
} Instruction;

#endif

/////////////////
// Input structs
/////////////////

#ifdef ARDUINO
typedef struct {
	int16_t _lineup = LINEUP;
	int16_t pid;
	unsigned long message_type = MSG_TYPE_BMP280;

	unsigned long timestamp_1;
	int32_t hum;
	int32_t temp;
	int32_t pres;
} BMP280SensorData;
#else
typedef struct {
	__int16 _lineup = LINEUP;
	__int16 pid;
	unsigned long message_type = MSG_TYPE_BMP280;

	unsigned long timestamp_1;
	__int32 hum; // just padding
	__int32 temp;
	__int32 pres;
} BMP280SensorData;
#endif

#ifdef ARDUINO
typedef struct {
	int16_t _lineup = LINEUP;
	int16_t pid;
	unsigned long message_type;

	int8_t data[DATA_SIZE];
} Data, Placeholder;
#else
typedef struct {
	__int16 _lineup = LINEUP;
	__int16 pid;
	unsigned long message_type;

	__int8 data[DATA_SIZE];
} Data, Placeholder;
#endif

#ifdef ARDUINO
typedef struct {
	int16_t _lineup = LINEUP;
	int16_t pid;
	unsigned long message_type = MSG_TYPE_PING;

	unsigned long roundabout;
	unsigned long timestamp;

	int16_t RSSI;
	int8_t padding[6];
} Ping;
#else
typedef struct {
	__int16 _lineup = LINEUP;
	__int16 pid;
	unsigned long message_type = MSG_TYPE_PING;

	unsigned long roundabout;
	unsigned long timestamp;

	__int16 RSSI;
	__int8 padding[6];
} Ping;
#endif


#ifdef ARDUINO
typedef struct {
	int16_t _lineup = LINEUP;
	int16_t pid;
	unsigned long message_type = MSG_TYPE_SYNC_CLOCK;

	unsigned long timestamp_satellite;
	unsigned long timestamp_ground;

	int8_t padding[8];
} SyncClock;
#else
typedef struct {
	__int16 _lineup = LINEUP;
	__int16 pid;
	unsigned long message_type = MSG_TYPE_SYNC_CLOCK;

	unsigned long timestamp_satellite;
	unsigned long timestamp_ground;

	__int8 padding[8];
} SyncClock;
#endif

#endif // STRUCTS_H


