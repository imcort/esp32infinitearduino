#include <WiFi.h>
#include "AsyncUDP.h"
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//#include <ArduinoJson.h>

#include <SPI.h>
#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>
#include "hidjoystickrptparser.h"

#include "ifparser.h"
#include "EEPROM.h"

#include <Ticker.h>

#define WIFI_LED 32
#define CONNECT_LED 33
#define BUTTON_PIN 26

AsyncUDP udp;
AsyncClient client;
#define ListenUdpPort 15000  // local port to listen on

#define EEPROM_SIZE 8

USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);

//for LED status

Ticker ticker;

bool ConnectFlag = 0;

void blinkLED() {
  static bool WifiLedState = 0;
  digitalWrite(WIFI_LED, WifiLedState);
  WifiLedState = !WifiLedState;
}

void configModeCallback (WiFiManager *myWiFiManager) {

  ticker.attach(0.2, blinkLED);
}


void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(WIFI_LED, OUTPUT);
  //digitalWrite(WIFI_LED, 1);
  ticker.attach(0.6, blinkLED);
  pinMode(CONNECT_LED, OUTPUT);
  digitalWrite(CONNECT_LED, 1);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //////////////////////////////////////////////////
  Serial.printf("Connecting to Wifi.");
  //////////////////////////////////////////////////
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect("InfiniteController");
  Serial.println("connected...yeey :)");
  //digitalWrite(WIFI_LED, 0);
  ticker.detach();
  //////////////////////////////////////////////////
  Serial.println("Initalize USB");
  //////////////////////////////////////////////////
  if (Usb.Init() == -1) {
    Serial.println("OSC did not start. Restarting");
    ESP.restart();
  }

  delay(200);

  if (!Hid.SetReportParser(0, &Joy))
    ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);

  //////////////////////////////////////////////////
  Serial.println("Initalize EEPROM");
  //////////////////////////////////////////////////
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  //////////////////////////////////////////////////
  Serial.println("Initalize UDP Listen");
  //////////////////////////////////////////////////
  if (udp.listen(ListenUdpPort)) {
    Serial.println("UDP connected");
    udp.onPacket([](AsyncUDPPacket packet) {

      size_t len = packet.length();
      ParseUDPRecivedData(packet.data(), len);

    });
  }
  //////////////////////////////////////////////////
  Serial.println("Initalize TCP Listen");
  //////////////////////////////////////////////////
  client.onConnect([](void* obj, AsyncClient * c) {
    Serial.println("TCP Connected..");
    digitalWrite(CONNECT_LED, 0);
    ConnectFlag = 1;
  });     //on successful connect
  client.onData([](void* obj, AsyncClient * c, void *data, size_t len) {
    if (len > 4) {
      ParseTCPRecivedData((uint8_t*)data, len);
    }
  });           //data received
  client.onDisconnect([](void* obj, AsyncClient * c) {
    digitalWrite(CONNECT_LED, 1);
    ConnectFlag = 0;
  });
  /*
      client.onConnect(onConnect);     //on successful connect
      client.onDisconnect(onDisconnect);  //disconnected
      client.onAck(onAck);             //ack received
      client.onError(onError);         //unsuccessful connect or error
      client.onData(onData);           //data received
      client.onTimeout(onTimeout);     //ack timeout
      client.onPoll(onPoll);        //every 125ms when connected
  */
}

unsigned long timer = 0;

void loop() {
  
  while (!ConnectFlag) {
    ConnectClient();
    delay(500);
  }
  

  for (;;) {

    //Realtime Task: Connection test
    if (!ConnectFlag) {
      Serial.println("TCP disconnected.");
      break;
    }

    //Realtime Task: USB
    Usb.Task();

  }

}
