#include <WiFi.h>
#include "AsyncUDP.h"
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <SPI.h>
#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>
#include "hidjoystickrptparser.h"

#include "EEPROM.h"
#include <Ticker.h>

#include "ifparser.h"

#define ListenUdpPort 15000  // local port to listen on

#define WIFI_LED 32
#define CONNECT_LED 33
#define BUTTON_PIN 26

#define EEPROM_SIZE 8

AsyncUDP udp;
AsyncClient client;
Ticker MakeConnectTicker, LEDBlinkTicker;

USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);

void MakeConnectTask() {

  Serial.println("connecting");
  ConnectClient();

}

void blinkWiFiLED() {
  static bool WifiLedState = 0;
  digitalWrite(WIFI_LED, WifiLedState);
  WifiLedState = !WifiLedState;
}

void configModeCallback (WiFiManager *myWiFiManager) {

  LEDBlinkTicker.attach(0.2, blinkWiFiLED);

}

void codeForTask1( void * parameter )
{
  for (;;) {
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  //////////////////////////////////////////////////
  Serial.printf("LED Mode Set.");
  //////////////////////////////////////////////////
  pinMode(WIFI_LED, OUTPUT);
  digitalWrite(WIFI_LED, 1);     //CLOSE
  LEDBlinkTicker.attach(0.6, blinkWiFiLED); //Slow Flash

  pinMode(CONNECT_LED, OUTPUT);
  digitalWrite(CONNECT_LED, 1);  //CLOSE

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //////////////////////////////////////////////////
  Serial.printf("Connecting to Wifi.");
  //////////////////////////////////////////////////
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback); //Fast Flash
  wifiManager.autoConnect("InfiniteController");
  Serial.println("connected...yeey :)");
  LEDBlinkTicker.detach();
  digitalWrite(WIFI_LED, 0); //LightOn

  //////////////////////////////////////////////////
  Serial.println("Initalize EEPROM");
  //////////////////////////////////////////////////
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  LoadClientAddr(CurrentAirplane.ClientAddress, CurrentAirplane.ClientPort);
  //Load Last client address EEPROM -> CurrentAirplane

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
  Serial.println("Initalize UDP Listen");
  //////////////////////////////////////////////////
  if (udp.listen(ListenUdpPort)) {
    Serial.println("UDP listening");
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
    MakeConnectTicker.detach();
    SaveClientAddr(CurrentAirplane.ClientAddress, CurrentAirplane.ClientPort);

  });
  client.onDisconnect([](void* obj, AsyncClient * c) {

    digitalWrite(CONNECT_LED, 1);
    MakeConnectTicker.attach(0.5, MakeConnectTask);
    Serial.print("onDisconnect");

  });
  MakeConnectTicker.attach(0.5, MakeConnectTask);
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

void loop() {

  Usb.Task();
  if (digitalRead(BUTTON_PIN) == LOW)
    ESP.restart();

}
