/*
     UDP NTP Client for ESP8266

     Get the time from a Network Time Protocol (NTP) time server
     Demonstrates use of UDP sendPacket and ReceivePacket
     For more on NTP time servers and the messages needed to communicate with them,
     see http://en.wikipedia.org/wiki/Network_Time_Protocol

     Created for ESP8266 on Feb 26 2017
     By: Rajendra Jain
     Routines for packing and unpacking the NTP packet are based on the following
     original work: (4 Sep 2010) by Michael Margolis and Tom Igoe (9 Apr 2012)

     This project is licensed under the terms of the MIT license.
     Read this to ve on the good side of the US Government http://tf.nist.gov/tf-cgi/servers.cgi
*/

#include <WiFiConnect.h> //download here - https://github.com/jainrk/WiFiConnect
#include <NTPClient.h>   //download here - https://github.com/jainrk/NTPClient

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SSD1306.h> //https://github.com/squix78/esp8266-oled-ssd1306 

#define aSSID  "mySSID"
#define aPASSWORD "myPWD"

//D1(GPIO5)->SDA
//D2(GPIO4)->SCL
SSD1306  display(0x3c, D1, D2);
//SSD1306  display(0x3c, 5, 4);

void drawTime(String _time, String message = "") {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  display.drawString(64, 20, _time);
  if (message != String("")) {
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 46, message);
  }

  display.display();
}

void drawMessage(String m1 = "", String m2 = "", String m3 = "", int _delay = 10) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  if (m1 != String(""))
    display.drawString(64, 4, m1); 
  if (m2 != String(""))
    display.drawString(64, 24, m2); 
  if (m3 != String(""))
    display.drawString(64, 44, m3); 
  display.display();
  delay(_delay);
}

WiFiConnect* wifiCon = WiFiConnect::Instance(aSSID, aPASSWORD);
/*
 * 200=time NTP server given to respond. If the clock acts goofy 
 * then increase it in intervals of 100
 */
NTPClient* ntpClient = NTPClient::Instance(200); 

void setup() { 
  Serial.begin(115200);
  wifiCon->Connect();
  Wire.begin(D1, D2);
  display.init();
  display.flipScreenVertically();
  drawMessage("IP: " + WiFi.localIP().toString(), "SSID: " + WiFi.SSID(), "RSSI: " + String(WiFi.RSSI(), DEC) + " dBm", 2000);
  display.clear();
}

String localUNIXTimeString;
unsigned long currentSyncTime = 0;
unsigned long syncInterval = 10000UL; // good for testing, change to 10 minutes = 600000UL
unsigned long previousMillis_S = 0;
unsigned long  localClock = 0;
bool timeAcquired = false; //flag to force esp to contact time server for the first time.

int messageNum = 0;
const char* message[] = {"Check Router", "NIST Internet Time", "Sync"};
enum messagesType {CheckRouter, NISTTime, Sync};
/*
   1. Acquire NTP every syncInterval seconds
   2. else Update localClock every second.
   3. Display the time in 00:00:00 format.
*/
void loop() {
  unsigned long currentMillis = millis();
  unsigned long localUNIXTime = 0;
  if ((unsigned long)((currentMillis - currentSyncTime) >= syncInterval) || !timeAcquired)  { // Acquire NTP 
    timeAcquired = true;
    currentSyncTime = currentMillis;
    previousMillis_S = currentMillis;
    localUNIXTime = ntpClient->getTime(localUNIXTimeString, -4); //-4 = EDT US Eastern Daylight Time
    if (localUNIXTime) {
      localClock = localUNIXTime;
      messageNum = Sync;
      Serial.println (localUNIXTimeString+ " " + message[messageNum]);
      drawTime(localUNIXTimeString, message[messageNum]);
    } else {
      Serial.println ("00:00:00, Check Router");
      messageNum = CheckRouter;
    }
  } else {//update with local clock
    if (((unsigned long)(currentMillis - previousMillis_S) >= 1000L )) { //every second
      previousMillis_S = currentMillis;
      localClock += 1;
      localUNIXTimeString = ntpClient->convertUNIXTimeToString(localClock);
      if (messageNum != CheckRouter) messageNum = NISTTime;
      Serial.println (localUNIXTimeString+ " " + message[messageNum]);
      drawTime(localUNIXTimeString, message[messageNum]);
    }
  }  
}//loop


