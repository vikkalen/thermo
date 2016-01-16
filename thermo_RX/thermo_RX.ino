//----------------------------------------------------------------------------------------------------------------------
// TinyTX Simple Receive Example
// By Nathan Chantrell.
//
// Licenced under the Creative Commons Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0) licence:
// http://creativecommons.org/licenses/by-sa/3.0/
//----------------------------------------------------------------------------------------------------------------------

//#include <JeeLib.h> // https://github.com/jcw/jeelib
#include <RFM12B.h> // https://github.com/LowPowerLab/RFM12B
#define NODEID           1  //network ID used for this unit
#define NETWORKID       99  //the network ID we are on
#define RF_RSSI A0

// Fixed RF12 settings

#define MYNODE 30            //node ID of the receiever
#define freq RF12_433MHZ     //frequency
#define group 210            //network group

typedef struct {
  int rxD;              // sensor value
  int supplyV;          // tx voltage
} Payload;
Payload rx;

RFM12B radio;

void setup () {

  Serial.begin(9600);

  radio.Initialize(NODEID, RF12_433MHZ, NETWORKID, 0, 20);
  //radio.XFER(0xC650);
  //radio.XFER(0x94A4);
  radio.rssi_pin = A0;
}

void loop() {

  if (radio.ReceiveComplete())
  {
    rx = *(Payload*) radio.Data;
    int value = rx.rxD;
    int millivolts = rx.supplyV;
    int sender = radio.GetSender();
    uint8_t strength = radio.rssi;
    Serial.print(sender);
    Serial.print(";");
    Serial.print(value);
    Serial.print(";");
    Serial.print(millivolts);
    Serial.print(";");
    Serial.print(strength);
    Serial.print(";");
    if (!radio.CRCPass())
    {
      Serial.println("ERR");
    }
    else
    {
      Serial.println("OK");
      if (radio.ACKRequested())
      {
        radio.SendACK();
      }
    }
  }
}
