// Elemental USB Gateway firmware
// Updated on 01/29/2020

//*****************************************************************************************************************************
// libraries in use

#include <RFM69.h>         //http://github.com/lowpowerlab/rfm69
//#include <SPIFlash.h>      //http://github.com/lowpowerlab/spiflash
#include <SPI.h>           

//*****************************************************************************************************************************
// configurable global variables

#define ENCRYPTKEY      "Tt-Mh=SQ#dn#JY3_" //has to be same 16 characters/bytes on all nodes, not more not less!
#define NODEID          1
#define NETWORKID       1
#define FREQUENCY       RF69_915MHZ //(others: RF69_433MHZ, RF69_868MHZ)
#define FREQUENCY_EXACT 905000000   // change the frequency in areas of interference (default: 915MHz)
#define IS_RFM69HW      //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define LED             9
#define SERIAL_BAUD     115200

// other global variables and objects
RFM69 radio;
//SPIFlash flash(FLASH_CS, 0xEF30); //EF40 for 16mbit windbond chip
char data[100];
char dataPacket[150];
char _rssi[5];
char _i[4];

//*****************************************************************************************************************************

void setup()
{
//  pinMode(10, OUTPUT);
  Serial.begin(SERIAL_BAUD);

  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.encrypt(ENCRYPTKEY);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
#ifdef FREQUENCY_EXACT
  radio.setFrequency(FREQUENCY_EXACT); //set frequency to some custom frequency
#endif
  //flash.initialize();

  fadeLED(LED);
}

//*****************************************************************************************************************************

void loop()
{
  if (radio.receiveDone())
  {
    int rssi = radio.RSSI;
    int nodeID = radio.SENDERID;
    
    data[0] = 0; // clear array by adding a null character

    if (radio.DATALEN > 0)
    {
      for (byte i = 0; i < radio.DATALEN; i++)
        data[i] = (char)radio.DATA[i];
    }

    data[sizeof(data)] = 0; // add null character to the end of data array

    if (radio.ACKRequested())
    {
      radio.sendACK();

      dtostrf(nodeID, 1, 0, _i);  // convert decimal to string; 1 is minimal width, 0 is decimal precision
      dtostrf(rssi, 3, 0, _rssi);

      dataPacket[0] = 0;  // first value of dataPacket should be a 0 (null) to clear it
      strcat(dataPacket, "i:");
      strcat(dataPacket, _i);  // append node ID
      strcat(dataPacket, ",");
      strcat(dataPacket, data);  // append actual data
      strcat(dataPacket, ",r:");
      strcat(dataPacket, _rssi); // append RSSI

      Serial.println(dataPacket); // send packet over serial
      Blink(LED,5);   // blink LED to indicate packet receive and send + give delay to let serial complete sending

      // clear all char arrays
      memset(data, 0, sizeof data);
      memset(dataPacket, 0, sizeof dataPacket);
      memset(_i, 0, sizeof _i);
      memset(_rssi, 0, sizeof _rssi);
    }
  }
}

//*****************************************************************************************************************************
// Blink LED

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

//*****************************************************************************************************************************
// Fade LED 

void fadeLED(int pin)
{
  int brightness = 0;
  int fadeAmount = 5;
  for(int i=0; i<510; i=i+5)  // 255 is max analog value, 255 * 2 = 510
  {
    analogWrite(pin, brightness);  // pin 9 is LED
  
    // change the brightness for next time through the loop:
    brightness = brightness + fadeAmount;  // increment brightness level by 5 each time (0 is lowest, 255 is highest)
  
    // reverse the direction of the fading at the ends of the fade:
    if (brightness <= 0 || brightness >= 255)
    {
      fadeAmount = -fadeAmount;
    }
    // wait for 20-30 milliseconds to see the dimming effect
    delay(10);
  }
  digitalWrite(pin, LOW); // switch LED off at the end of fade
}

//*****************************************************************************************************************************
