
//Ethernet PB0/CS is 53

// Screen dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128
#define DC_PIN   3
#define CS_PIN   45
#define RST_PIN  47

// Color definitions
#define BLACK           0x0000
#define RED            0x001F
#define BLUE             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

#include "FujiHeatPump.h"
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <PubSubClient.h>

FujiHeatPump hp;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 249);
IPAddress server(192, 168, 0, 217);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String payload_str;
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    payload_str = payload_str + (char)payload[i];
}
  Serial.println();
    if (payload_str == "on"){
    hp.setOnOff(true);
    }
    if (payload_str == "off"){
    hp.setOnOff(false);
    }    
  
  Serial.println();
}


EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

unsigned long previousTime = millis(); // or millis()
long timeInterval = 5000;

bool OldOnOff = false;
byte OldTemp= B0;
byte OldMode = B0;
byte OldFan = B0;
byte RoomTemp = B0;



void setup() {

pinMode(46, OUTPUT); // set up enable pin of LIN transceiver
digitalWrite(46, HIGH); // set enable pin high to activate normal mode of LIN transceiver
pinMode(12, OUTPUT); // set up enable pin of OLED
digitalWrite(12, HIGH); // set enable pin high to activate OLED

client.setServer(server, 1883);
client.setCallback(callback);

Serial.begin(9600);

hp.connect(&Serial2, true); // use Serial2, and bind as a secondary controller
//hp.debugPrint = true; 
Serial.println("starting");

Ethernet.init(53);
tft.begin();

tft.setRotation(1);
  tft.fillScreen(BLACK);
  tft.setCursor(18, 10);
  tft.setTextColor(WHITE);  
  tft.setTextSize(1);
  tft.println("Fujitsu MQTT");
  tft.setCursor(18, 22);
  tft.println("Controller");
  tft.drawFastHLine(0, 35, 127, WHITE);
  tft.setCursor(18, 45);
  tft.setTextColor(WHITE);  
  tft.setTextSize(1);
  tft.println("AC Bound:");
  tft.fillCircle(98, 49, 3, RED);
  tft.setCursor(18, 60);
  tft.println("Ethernet:");
  tft.fillCircle(98, 64, 3, RED);
  tft.setCursor(18, 75);
  tft.println("MQTT:");
  tft.fillCircle(98, 79, 3, RED);
  
  Ethernet.begin(mac,ip);
  Serial.println(millis());
}

void loop() {
  
hp.waitForFrame();
unsigned long currentTime = millis(); // or millis()

if(hp.isBound()){tft.fillCircle(98, 49, 3, GREEN);}
else{tft.fillCircle(98, 49, 3, RED);}

auto link = Ethernet.linkStatus();
switch (link) {
    case LinkON:
        tft.fillCircle(98, 64, 3, GREEN);
      break;
    case LinkOFF:
        tft.fillCircle(98, 64, 3, RED);
      break;
  }

if (hp.getOnOff() == OldOnOff){}
else{
  OldOnOff = hp.getOnOff();
  if (OldOnOff){
    client.publish("fuji/ison", "true");
  }
  else{
    client.publish("fuji/ison", "false");
  }
}


if (currentTime - previousTime > timeInterval) 
  {
   Serial.println("Timer is working");
    if (!client.connected()) {
    tft.fillCircle(98, 79, 3, RED);
    if(client.connect("homeassistant", "user", "passw")){
      client.subscribe("fuji/ison/command");
      }

  }
  if(client.connected()) {
    tft.fillCircle(98, 79, 3, GREEN);
  }
    previousTime = currentTime;
  }
    

    hp.sendPendingFrame();
    client.loop();
  
}
