#include <SPI.h>

#include "C:\Users\jacob\Documents\CanSat 2019\Final\Headers\Structs.h"
//#include "C:\Users\jacob\Documents\CanSat 2019\Final\Headers\cactus_io_BME280_I2C.h" save for the satellite
#include <RFM69.h>

#define BAUD_RATE 115200
#define READ_LOOP 330

#define RETRIES 3
#define TIMEOUT 80

#define THIS_ID 3
#define SATELLITE_ID 1


RFM69 Radio;
long satellite_to_ground_timer_offset;

bool AutoPing;

void setup() {
	Serial.begin(BAUD_RATE);

    Radio.initialize(RF69_868MHZ, THIS_ID);
    Radio.setHighPower();
    Radio.setPowerLevel(23);
    Radio.promiscuous(false);

    satellite_to_ground_timer_offset = 0;
    AutoPing = false;
}


Instruction rInstruction; // RX buffer for PC comm
Placeholder p; // RX Buffer for radio comm

int8_t tBuffer[TOTAL_PACKET_SIZE];

int ping_pid;
unsigned long ping_send_time;

void loop() {
    if(AutoPing) {
        Ping autoping;
        autoping.pid = random(32668) + 1; // max number (2^16 /2 - padding) (+1 to make sure it cant be 0)
        autoping.timestamp = millis();

        ping_pid = autoping.pid;
        memcpy(tBuffer, &autoping, sizeof(Ping));

        ping_send_time = millis(); // set the time the ping packet was sent
        Radio.send(SATELLITE_ID, tBuffer, sizeof(Ping));
    }

    // Reads instruction from PC

    if ((Serial.available() / sizeof(Placeholder)) >= 1) { // Tests if there is enough bytes to read an entire packet
        byte tempSerialBuffer[sizeof(Placeholder)]; // Creates a blank buffer in memory
        for(int i = 0; i < sizeof(Placeholder); i++) // Reads the serial data into the buffer
            tempSerialBuffer[i] = Serial.read();

        memcpy(&p, tempSerialBuffer, sizeof(Placeholder)); // Copies the buffer into the struct, where we can desipher the meaning

        if(p.message_type == MSG_TYPE_INSTRUCTION) {
            memcpy(&rInstruction, &p, sizeof(Instruction));
            switch(rInstruction.instruction) {
                case INSTRUCTION_START_AUTO_PING: {
                    AutoPing = AutoPing ? false : true;

                    p.message_type = MSG_TYPE_ERROR;
                    p.pid = rInstruction.instruction;
                    
                    if(AutoPing)
                        memcpy(p.data, "STARTED AUTOPING", DATA_SIZE);
                    else
                        memcpy(p.data, "STOPPED AUTOPING", DATA_SIZE);

                    Serial.write((const uint8_t*)&p, sizeof(Placeholder));
                } break;

                case INSTRUCTION_START_STOP_TRANSMIT_SATELLITE: {
                    Radio.send(SATELLITE_ID, tempSerialBuffer, sizeof(Instruction));
                    
                    p.message_type = MSG_TYPE_ERROR;
                    p.pid = rInstruction.instruction;
                    memcpy(p.data, "SENDING BME", DATA_SIZE);

                    Serial.write((const uint8_t*)&p, sizeof(Placeholder));

                } break;

                case INSTRUCTION_SYNC_CLOCK: {
                    SyncClock sc;
                    sc.timestamp_ground = millis();
                    memcpy(tBuffer, &sc, sizeof(SyncClock));
                    Radio.send(SATELLITE_ID, tBuffer, sizeof(Ping));

                    p.message_type = MSG_TYPE_ERROR;
                    p.pid = rInstruction.instruction;
                    memcpy(p.data, "SYNCING CLOCK", DATA_SIZE);

                    Serial.write((const uint8_t*)&p, sizeof(Placeholder));

                } break;

                case INSTRUCTION_PING: {
                    if(!AutoPing) {
                        Ping ping;
                        ping.pid = random(32668) + 1; // max number (2^16 /2 - padding) (+1 to make sure it cant be 0)
                        ping.timestamp = millis();

                        ping_pid = ping.pid;
                        memcpy(tBuffer, &ping, sizeof(Ping));

                        ping_send_time = millis(); // set the time the ping packet was sent
                        Radio.send(SATELLITE_ID, tBuffer, sizeof(Ping));

                        p.message_type = MSG_TYPE_ERROR;
                        p.pid = rInstruction.instruction;
                        memcpy(p.data, "SENDING PING", DATA_SIZE);

                        Serial.write((const uint8_t*)&p, sizeof(Placeholder));
                    }
                    else {
                        p.message_type = MSG_TYPE_ERROR;
                        p.pid = rInstruction.instruction;
                        memcpy(p.data, "NO, AUTOPING ON!", DATA_SIZE);

                        Serial.write((const uint8_t*)&p, sizeof(Placeholder));
                    }
                } break;

                default: {
                    p.message_type = MSG_TYPE_ERROR;
                    p.pid = rInstruction.instruction;
                    memcpy(p.data, "ME NO UNDERSTAND", DATA_SIZE);

                    Serial.write((const uint8_t*)&p, sizeof(Placeholder));
                } break;
            }
        } 
        else if (p.message_type == MSG_TYPE_DATA) { // if its not actually an instruction, but rather a data message            
            bool bMsgReceived = Radio.sendWithRetry(SATELLITE_ID, tempSerialBuffer, sizeof(Data), RETRIES, TIMEOUT); // send the pure buffer received from the poc
        
            if(!bMsgReceived) { // MSG not received
                p.message_type = MSG_TYPE_ERROR;
                p.pid = rInstruction.instruction;
                memcpy(p.data, "MSG NOT RECEIVED", DATA_SIZE);

                Serial.write((const uint8_t*)&p, sizeof(Placeholder));
            }
        }
    } 

    // Reads data from radio, and transmits it too pc

    bool Reading = true;
    unsigned long ReadingLoopEndTime = millis() + READ_LOOP;

    while(Reading) { // Loop just reads messages from readio and sends them to the PC;
        if(Radio.receiveDone() && Radio.SENDERID == SATELLITE_ID) { // Message received (and its from the satellite and not directly from the rover :))
            if (Radio.ACK_REQUESTED) {
                Radio.sendACK();
            }

            memcpy(&p, Radio.DATA, sizeof(Placeholder));
            if(p.message_type == MSG_TYPE_PING && p.pid == ping_pid) {// If the packet is a response to a ping
                ping_pid = 0;
                
                Ping p_ping;
                memcpy(&p_ping, &p, sizeof(Placeholder));
                p_ping.roundabout = millis() - ping_send_time;
                p_ping.RSSI = Radio.RSSI;
                p_ping.timestamp += satellite_to_ground_timer_offset;
                Serial.write((const uint8_t*)&p_ping, sizeof(Placeholder));
            }
            else if(p.message_type == MSG_TYPE_SYNC_CLOCK) {
                SyncClock sc;
                memcpy(&sc, &p, sizeof(SyncClock));

                satellite_to_ground_timer_offset = sc.timestamp_satellite - sc.timestamp_ground;

                Instruction ins;
                memcpy(&ins.response, "CLOCK SYNCED", 14);
                Serial.write((const uint8_t*)&ins, sizeof(Instruction));
            }
            else
                Serial.write((const uint8_t*)&p, sizeof(Placeholder));
        }

        if (millis() >= ReadingLoopEndTime) // If the time has elapsed, break the loop
            Reading = false;
    }  

//    Serial.write((const uint8_t*)&p, sizeof(Placeholder));
}
