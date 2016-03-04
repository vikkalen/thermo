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

RFM12B radio;

char serialBuffer[8];
uint8_t serialBufferIdx = 0;
int txBuffer[(RF12_MAXDATA>>1) + 1];
uint8_t txBufferIdx = 0;

void setup () {

  Serial.begin(9600);

  radio.Initialize(NODEID, RF12_433MHZ, NETWORKID, 0, 20);
  radio.rssi_pin = A0;

//  PRR = bit(PRTIM1); // only keep timer 0 going
  
//  ADCSRA &= ~ bit(ADEN); bitSet(PRR, PRADC); // Disable the ADC to save power

}

void loop() {

  if (radio.ReceiveComplete())
  {
    int sender = radio.GetSender();
    uint8_t strength = radio.rssi;
    Serial.print(sender);
    Serial.print(";");
    for(uint8_t idx = 0; idx < *radio.DataLen && idx < RF12_MAXDATA; idx += 2)
    {
      Serial.print(*(int*) (radio.Data + idx));
      Serial.print(";");
    }
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

  if(Serial.available())
  {
    char c = Serial.read();
    if((c == ';') || (serialBufferIdx == sizeof(serialBuffer) - 1))
    {
      serialBuffer[serialBufferIdx] = '\0';
      String s = serialBuffer;
      txBuffer[txBufferIdx++] = s.toInt();
      serialBufferIdx = 0;
    }
    else if ((c == '\n') || (txBufferIdx == sizeof(txBuffer) - 1))
    {
      if(serialBufferIdx > 0)
      {
        serialBuffer[serialBufferIdx] = '\0';
        String s = serialBuffer;
        txBuffer[txBufferIdx++] = s.toInt();
      }
      if(txBufferIdx > 0)
      {
        int node = (int)txBuffer[0];
        int* payload = (int*)txBuffer;
        payload++;

        //radio.SendStart(node, payload, (txBufferIdx - 1) << 1, false, false, 0);
        //Serial.println("transmitting");
        radio.Send(node, payload, (txBufferIdx - 1) << 1, false, 0);
        //Serial.println("transmitted");
      }
      serialBufferIdx = 0;
      txBufferIdx = 0;
    }
    else
    {
      serialBuffer[serialBufferIdx++] = c;
    }
  }  
}
