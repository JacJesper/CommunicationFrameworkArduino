#include <SPI.h>
#include <SD.h>

#include <RFM69.h>

#include "C:\Users\jacob\Documents\CanSat 2019\Final\Headers\Structs.h"

#define BAUD_RATE 115200
#define READ_LOOP 330

#define RETRIES 3
#define TIMEOUT 80

#define THIS_ID 2
#define SATELLITE_ID 1



int8_t tBuffer[TOTAL_PACKET_SIZE];

RFM69 Radio;
Placeholder p;

void setup() {
	Serial.begin(BAUD_RATE);

    Radio.initialize(RF69_868MHZ, THIS_ID);
    Radio.setHighPower();
    Radio.setPowerLevel(23);
    Radio.promiscuous(false);
}


void loop() {
    if ((Serial.available() / sizeof(Placeholder)) >= 1) { // Tests if there is enough bytes to read an entire packet
        byte tempSerialBuffer[sizeof(Placeholder)]; // Creates a blank buffer in memory
        for(int i = 0; i < sizeof(Placeholder); i++) // Reads the serial data into the buffer
            tempSerialBuffer[i] = Serial.read();

        memcpy(&p, tempSerialBuffer, sizeof(Placeholder)); // Copies the buffer into the struct, where we can desipher the meaning

        if (p.message_type == MSG_TYPE_DATA) { // if its not actually an instruction, but rather a data message            
            bool bMsgReceived = Radio.sendWithRetry(SATELLITE_ID, tempSerialBuffer, sizeof(Data), RETRIES, TIMEOUT); // send the pure buffer received from the poc
        
            if(!bMsgReceived) { // MSG not received
                p.message_type = MSG_TYPE_ERROR;
                memcpy(p.data, "MSG NOT RECEIVED", DATA_SIZE);

                Serial.write((const uint8_t*)&p, sizeof(Placeholder));
            }
        }
    } 


    // Checks the radio

    bool Reading = true;
    unsigned long ReadingLoopEndTime = millis() + READ_LOOP;

    while(Reading) { // Loop just reads messages from readio and sends them to the PC;
        if(Radio.receiveDone() && Radio.SENDERID == SATELLITE_ID) { // Message received (and its from the satellite and not directly from the rover :))
            if (Radio.ACK_REQUESTED) {
                Radio.sendACK();
            }

            memcpy(&p, Radio.DATA, sizeof(Placeholder));
            memcpy(tBuffer, Radio.DATA, sizeof(Placeholder));

            if(p.message_type == MSG_TYPE_PING) {// If the packet is a response to a ping
                Radio.send(SATELLITE_ID, tBuffer, sizeof(Data));
            }
            else // if its data ... (send it to connected pc)
                Serial.write((const uint8_t*)&p, sizeof(Placeholder));
        }

        if (millis() >= ReadingLoopEndTime) // If the time has elapsed, break the loop
            Reading = false;
    }
}
