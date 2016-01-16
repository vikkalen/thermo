//----------------------------------------------------------------------------------------------------------------------
// TinyTX - An ATtiny84 and RFM12B Wireless Temperature Sensor Node
// By Nathan Chantrell. For hardware design see http://nathan.chantrell.net/tinytx
//
// Using the Dallas DS18B20 temperature sensor
//
// Licenced under the Creative Commons Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0) licence:
// http://creativecommons.org/licenses/by-sa/3.0/
//
// Requires Arduino IDE with arduino-tiny core: http://code.google.com/p/arduino-tiny/
// and small change to OneWire library, see: http://arduino.cc/forum/index.php/topic,91491.msg687523.html#msg687523
//----------------------------------------------------------------------------------------------------------------------

#include <OneWire.h> // http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip
#include <DallasTemperature.h> // http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_LATEST.zip
//#include <JeeLib.h> // https://github.com/jcw/jeelib

//#include <avr/sleep.h>
#include <RFM12B.h> // https://github.com/LowPowerLab/RFM12B
#include <Adafruit_SleepyDog.h>

//ISR(WDT_vect) { Sleepy::watchdogEvent(); } // interrupt handler for JeeLabs Sleepy power saving

#define NODEID        4  //network ID used for this unit
#define NETWORKID    99  //the network ID we are on
#define GATEWAYID     1  //the node ID we're sending to
#define ACK_TIME     50  // # of ms to wait for an ack

//#define ONE_WIRE_BUS 10   // DS18B20 Temperature sensor is connected on D10/ATtiny pin 13
//#define ONE_WIRE_POWER 9  // DS18B20 Power pin is connected on D9/ATtiny pin 12
#define ONE_WIRE_BUS 0   // DS18B20 Temperature sensor is connected on D10/ATtiny pin 13
#define ONE_WIRE_POWER 1  // DS18B20 Power pin is connected on D9/ATtiny pin 12

OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance

DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature

//########################################################################################################################
//Data Structure to be sent
//########################################################################################################################

 typedef struct {
      int temp; // Temperature reading
      int supplyV;  // Supply voltage
 } Payload;

 Payload tinytx;

 RFM12B radio;

// wait a few milliseconds for proper ACK, return true if received
bool waitForAck() {
  long now = millis();
  while (millis() - now <= ACK_TIME)
    if (radio.ACKReceived(GATEWAYID))
      return true;
  return false;
}

//--------------------------------------------------------------------------------------------------
// Read current supply voltage
//--------------------------------------------------------------------------------------------------
 long readVcc() {
   bitClear(PRR, PRADC); ADCSRA |= bit(ADEN); // Enable the ADC
   long result;
   // Read 1.1V reference against Vcc
   #if defined(__AVR_ATtiny84__) 
    ADMUX = _BV(MUX5) | _BV(MUX0); // For ATtiny84
   #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega328
   #endif 
   delay(2); // Wait for Vref to settle
   ADCSRA |= _BV(ADSC); // Convert
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   result = 1126400L / result; // Back-calculate Vcc in mV
   ADCSRA &= ~ bit(ADEN); bitSet(PRR, PRADC); // Disable the ADC to save power
   return result;
} 
//########################################################################################################################

void setup() {
  
  radio.Initialize(NODEID, RF12_433MHZ, NETWORKID, 0, 20);
//  radio.Encrypt(KEY);
  radio.Sleep(); //sleep right away to save power

  pinMode(ONE_WIRE_POWER, OUTPUT); // set power pin for DS18B20 to output
  
  PRR = bit(PRTIM1); // only keep timer 0 going
  
  ADCSRA &= ~ bit(ADEN); bitSet(PRR, PRADC); // Disable the ADC to save power
}

void loop() {
  
  digitalWrite(ONE_WIRE_POWER, HIGH); // turn DS18B20 sensor on

  //Sleepy::loseSomeTime(5); // Allow 5ms for the sensor to be ready
  Watchdog.sleep(5);
//  delay(5); // The above doesn't seem to work for everyone (why?)
 
  sensors.begin(); //start up temp sensor
  sensors.setResolution(11);
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures(); // Get the temperature
  Watchdog.sleep(500);
  tinytx.temp=(sensors.getTempCByIndex(0)*100); // Read first sensor and convert to integer, reversed at receiving end//
  digitalWrite(ONE_WIRE_POWER, LOW); // turn DS18B20 off
  tinytx.supplyV = readVcc(); // Get supply voltage

  radio.Wakeup();
  radio.Send(GATEWAYID, &tinytx, sizeof tinytx, false);
//  waitForAck();
  radio.Sleep();

  // sleep for 64 secs
  for (int i=0; i<8; i++) {
    Watchdog.sleep(8000);
  }
}

