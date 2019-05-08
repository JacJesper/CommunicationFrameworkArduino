#include <SPI.h>

#include "C:\Users\jacob\Documents\CanSat 2019\Final\Headers\Structs.h"
#include "cactus_io_BME280_I2C.h"
#include <RFM69.h>


#define BAUD_RATE 115200
#define CYCLE_TIME 50
#define READ_LOOP 330

#define RETRIES 3
#define TIMEOUT 80

#define THIS_ID 1
#define GROUNDSTATION_ID 3
#define ROVER_ID 2

void InstructionHandler(Instruction ins);

RFM69 Radio;
BME280_I2C bme(0x76); // The BME object
bool bTransmitData;

void setup() {
	Serial.begin(BAUD_RATE);

    bool RadioGood = Radio.initialize(RF69_868MHZ, THIS_ID);
    if(!RadioGood) {
        Serial.println("Couldn't start radio, sure its connected?");
        while(true) {
        }
    }
    Radio.setHighPower();
    Radio.setPowerLevel(23);
    Radio.promiscuous(false);

    bool BME_OK= bme.begin();
    if(!BME_OK) {
        while (true) { // block execution
            delay(1000);
        }
    }

    Serial.println("Ready to go!");

    bTransmitData = false;
}

BMP280SensorData bme_send_value;
int8_t bme_buffer[TOTAL_PACKET_SIZE];
int8_t tBuffer[TOTAL_PACKET_SIZE];
bool Reading = true;
int ReadingLoopEndTime;
Placeholder p;

void loop() {  

    // Receive data    
    bool Reading = true;
    unsigned long ReadingLoopEndTime = millis() + READ_LOOP; // Remaining time of CYCLE_TIME

    while(Reading) { // Loop just reads messages from readio and sends them to the PC;
        if(Radio.receiveDone()) { // Message received            
            if (Radio.ACK_REQUESTED) {
                Radio.sendACK();
                Serial.println("Sent ack");
            }

            memcpy(&p, Radio.DATA, sizeof(Placeholder));
            memcpy(tBuffer, Radio.DATA, sizeof(Placeholder));

            switch(Radio.SENDERID) {
                case GROUNDSTATION_ID: {
                    Serial.println("received from groundstation");

                    if(p.message_type == MSG_TYPE_DATA) {
                        Radio.sendWithRetry(ROVER_ID, tBuffer, sizeof(Data), RETRIES, TIMEOUT);
                    }
                    else if (p.message_type == MSG_TYPE_PING) { // Just send the ping on to the receiver
                        Serial.println("Sending ping to rover");
                        Radio.send(ROVER_ID, tBuffer, sizeof(Data)); 
                    }
                    else if (p.message_type == MSG_TYPE_INSTRUCTION) {                        
                        Instruction ins;
                        memcpy(&ins, &p, sizeof(Placeholder));

                        if(ins.instruction == INSTRUCTION_START_STOP_TRANSMIT_SATELLITE) {


                            bTransmitData = bTransmitData ? false : true; // Switches the transmitter on / off
                        
                            if(bTransmitData)
                                memcpy(&ins.response, "STARTING BME", 14);
                            else
                                memcpy(&ins.response, "STOPPING BME", 14);
                            
                            memcpy(tBuffer, &ins, TOTAL_PACKET_SIZE);
                            Radio.send(GROUNDSTATION_ID, tBuffer, sizeof(Instruction));
                        }
                    }
                    else if (p.message_type == MSG_TYPE_SYNC_CLOCK) {
                        Serial.println("Received sync request!");
                        
                        SyncClock sc;
                        memcpy(&sc, &p, sizeof(SyncClock));

                        sc.timestamp_satellite = millis();

                        memcpy(tBuffer, &sc, sizeof(SyncClock));
                        Radio.send(GROUNDSTATION_ID, tBuffer, sizeof(SyncClock));
                    }

                } break;
                case ROVER_ID: {
                    Serial.println("Received from rover");

                    if(p.message_type == MSG_TYPE_DATA) {
                        Radio.sendWithRetry(GROUNDSTATION_ID, tBuffer, sizeof(Data), RETRIES, TIMEOUT);
                    }
                    else if (p.message_type == MSG_TYPE_PING) {
                        Radio.send(GROUNDSTATION_ID, tBuffer, sizeof(Data));
                        Serial.println("Sending the ping back to PC");
                    }
                } break;
            }
        }

        if (millis() >= ReadingLoopEndTime) // If the time has elapsed, break the loop
            Reading = false;
    }

    // Collect and send BME data 

    if(bTransmitData) {
        bme.readSensor();
        bme_send_value.timestamp_1 = millis();
        bme_send_value.pres = bme.getPressure_MB() * 100;
        bme_send_value.temp = bme.getTemperature_C() * 100;
        bme_send_value.hum = bme.getHumidity() * 100;
        memcpy(bme_buffer, &bme_send_value, sizeof(BMP280SensorData));

        bool bDataRecveived = Radio.sendWithRetry(GROUNDSTATION_ID, bme_buffer, sizeof(BMP280SensorData), RETRIES, TIMEOUT);
        if (!bDataRecveived) {
            Serial.println("BME data not received!");
        }
    }
}