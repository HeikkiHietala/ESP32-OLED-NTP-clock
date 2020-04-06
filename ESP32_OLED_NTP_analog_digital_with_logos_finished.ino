#include <WiFi.h>
#include "time.h"
#include "heltec.h"
#include "images.h"

// Replace with your network credentials
const char* ssid     = "YOUR_SSID_HERE";
const char* password = "YOUR_PASSWD_HERE";

// Define NTP Client to get time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600 + 3600 + 3600;

//analog clock drawing settings
int x_origo = 64;
int y_origo = 32;
int arc_second = 28;  //second hand length
int arc_minute = 25;  //minute hand length
int arc_hour = 18;    //hour hand length

//this pin defines whether to display analog or digital - when 18 is grounded, show digital
int switchPin = 18;
boolean boolSwitchPin;
boolean showAnalog;

void setup() {
  pinMode(switchPin, INPUT_PULLUP); //make switch pin high

  //anything that starts with HELTEC is related to the display
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);

  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 0, "Hello world");
  delay(2000);
  Heltec.display->clear();
  //this draws the Haaga-Helia logo - delete up to the delay if you don't need it
  Heltec.display -> drawXbm(0, 0, hhlogo_width, hhlogo_height, hhlogo_bits);
  Heltec.display -> display();
  delay(2000);
  //this draws the 3D + Robo Lab logo - delete up to the delay if you don't need it
  Heltec.display -> clear();
  Heltec.display -> drawXbm(0, 0, robolablogo_width, robolablogo_height, robolablogo_bits);
  Heltec.display -> display();
  delay(2000);
  Heltec.display->clear();
  //set font to 16 pt
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0, 0, "Connecting to ");
  Heltec.display->drawString(0, 15, ssid);
  Heltec.display->display();
  delay(10);
  WiFi.begin(ssid, password);
  delay(10);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  // Print local IP address and start web server
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "WiFi connected.");
  Heltec.display->drawString(0, 15, "IP address: ");
  Heltec.display->drawString(0, 30, WiFi.localIP().toString());
  Serial.println(WiFi.localIP());
  Heltec.display->display();
  delay(2000);
  Heltec.display->clear();

  // Initialize a NTPClient to get time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  WiFi.disconnect(); //end Wifi connection
}

boolean checkModePin() { //check if the mode switch is in analog or digital mode setting
  if (digitalRead(switchPin) == HIGH) {
    Serial.println("Analog mode");
    return true;
  }
  else {
    Serial.println("Digital mode");
    return false;
  }
}

void loop() {

  Heltec.display -> clear();
  boolSwitchPin = checkModePin();
  printLocalTime();
  delay(1000);
}


void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  //this routine makes the strings for the digital display
  Serial.println(&timeinfo, "%d.%m.%Y %H:%M:%S");
  char timeStringBuff[50]; //50 chars should be enough
  char dateStringBuff[50]; //50 chars should be enough
  char secStringBuff[3];
  char minStringBuff[3];
  char hourStringBuff[3];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &timeinfo);
  strftime(dateStringBuff, sizeof(dateStringBuff), "%d.%m.%Y", &timeinfo);
  strftime(secStringBuff, sizeof(secStringBuff), "%SS", &timeinfo);
  strftime(minStringBuff, sizeof(minStringBuff), "%M", &timeinfo);
  strftime(hourStringBuff, sizeof(hourStringBuff), "%H", &timeinfo);

  // This creates the analog clock angles for the hands
  String TimeAsString(timeStringBuff);
  String DateAsString(dateStringBuff);
  String SecAsString(secStringBuff);
  String MinAsString(minStringBuff);
  String HourAsString(hourStringBuff);
  int analogSec = SecAsString.toInt();
  int analogMin = MinAsString.toInt();
  int analogHour = HourAsString.toInt();
  float hourDegrees = (analogHour / float(12) * 360);
  hourDegrees = hourDegrees + ((float(analogMin) / 60) * 15);
  if (hourDegrees > 360) {
    hourDegrees = hourDegrees - 360;
  }

  //if the jumper wire is not attached, show digital
  if (boolSwitchPin == false) {
    Heltec.display -> clear();
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display -> drawString(25, 5, DateAsString);
    Heltec.display -> drawString(25, 25, TimeAsString);
    Heltec.display -> display();
    Heltec.display->setFont(ArialMT_Plain_10);
    //the mac address is shown just for fun
    Heltec.display -> drawString(15, 48, WiFi.macAddress());
    Heltec.display -> display();
  }
  
//if the jumper between 8 and grond IS connected, show analog
  else {
    float xsec = arc_second * sin(analogSec * 6 * PI / 180);     // Second needle TIP position, arc=needle length
    float ysec = arc_second * cos(analogSec * 6 * PI / 180);
    float xmin = arc_minute * sin(analogMin * 6 * PI / 180);      // Minute needle TIP position, arc=needle length
    float ymin = arc_minute * cos(analogMin * 6 * PI / 180);
    float xhour = arc_hour * sin(hourDegrees * PI / 180);      // Hour needle TIP position, arc=needle length
    float yhour = arc_hour * cos(hourDegrees * PI / 180);
    Heltec.display->drawCircle(x_origo, y_origo, 31);
    Heltec.display->drawLine(x_origo, y_origo, x_origo + xsec, y_origo - ysec); //Second hand
    Heltec.display->drawLine(x_origo, y_origo, x_origo + xmin, y_origo - ymin); //Minute hand
    Heltec.display->drawLine(x_origo, y_origo, x_origo + xhour, y_origo - yhour); //Hour hand
    Heltec.display->display();
  }
}
