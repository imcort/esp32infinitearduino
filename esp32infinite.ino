#include <WiFi.h>
#include "AsyncUDP.h"
#include <ArduinoJson.h>

#include <SPI.h>
#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>
#include "hidjoystickrptparser.h"

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//const char* ssid = "tplink";
//const char* password = "11392590";

#define WIFI_LED 22
#define CONNECT_LED 32
#define PRESS_KEY

AsyncUDP udp;
WiFiClient client;
#define ListenUdpPort 15000  // local port to listen on
#include "ifparser.h"

#include "EEPROM.h"
#define EEPROM_SIZE 8

USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);

struct IFClient {
  IPAddress IP;
  uint16_t Port;
  bool updated = false;
} ClientAddr;

DynamicJsonDocument doc;


void SaveClientAddr(IFClient addr) {
  int address = 0;
  EEPROM.writeUInt(address, uint32_t(addr.IP));
  address += sizeof(uint32_t(addr.IP));
  EEPROM.writeUShort(address, addr.Port);
  EEPROM.commit();
}

bool LoadClientAddr(IFClient& cli) {
  int address = 0;
  uint32_t ip = EEPROM.readUInt(address);
  address += sizeof(ip);
  uint16_t port = EEPROM.readUShort(address);

  if (ip & port) {

    cli.IP = ip;
    cli.Port = port;
    return true;
  } else {
    return false;
  }

}

bool ConnectClient() {

  if (ClientAddr.updated) {

    if (client.connect(ClientAddr.IP, ClientAddr.Port)) {
      ClientAddr.IP.printTo(Serial);
      Serial.println("Connected.From UDP");
      ClientAddr.updated = false;
      SaveClientAddr(ClientAddr);
      return true;
    }

  }

  IFClient lastClient;
  if (LoadClientAddr(lastClient)) {
    if (client.connect(lastClient.IP, lastClient.Port)) {
      Serial.println("Connected.From EEPROM");
      return true;
    }

  }

  return false;

}


uint8_t ReceiveClientData() {
  // Read all the lines of the reply from server and print them to Serial

  int PacketSize = client.available();

  if (PacketSize > 0) {

    uint32_t recvSize;
    uint8_t *recvString;
    uint8_t offset = 0;

    if (PacketSize == 4) {

      uint8_t rawRecvSize[4];
      client.read(rawRecvSize, 4);
      recvSize = *(uint32_t*)rawRecvSize;

      recvString = (uint8_t*)malloc(recvSize + 1);
      //PacketSize = client.available();
      client.read(recvString, recvSize);
      recvString[recvSize] = '\0';

    } else {

      recvString = (uint8_t*)malloc(PacketSize + 1);
      client.read(recvString, PacketSize);
      recvString[PacketSize] = '\0';

      recvSize = *(uint32_t*)recvString;
      offset = 4;

    }
    client.flush();
    Serial.println(recvSize);
    Serial.println(PacketSize);


    if ((recvSize == (PacketSize - 4)) || (PacketSize == 4)) {

      //recvSize = PacketSize;



      doc.clear();
      DeserializationError error = deserializeJson(doc, (recvString + offset));
      free(recvString);

      if (error) {
        Serial.print(F("###Json Parser Failed: "));
        Serial.println(error.c_str());
        return -1;
      }

      JsonObject root = doc.as<JsonObject>();

      String MsgType = root["Type"];
      if (MsgType == "Fds.IFAPI.APIAircraftState") {
        Serial.println("Fds.IFAPI.APIAircraftState");
        APIAircraftStateParser(root);
        return 1;

      } else if (MsgType == "Fds.IFAPI.APIAircraftInfo") {
        Serial.println("Fds.IFAPI.APIAircraftInfo");
        APIAircraftInfoParser(root);
        return 2;

      } else if (MsgType == "Fds.IFAPI.IFAPIStatus") {
        Serial.println("Fds.IFAPI.IFAPIStatus");
        APIDeviceInfoParser(root);
        return 3;

      } else {
        Serial.print(F("###Msg Type Not Support: "));
        Serial.println(MsgType.c_str());
        return -1;
      }
    } else {

      //Serial.print("Packet Size Wrong:");
      //Serial.print(PacketSize);
      //Serial.print(",");
      //Serial.println(recvSize);
      client.flush();
      return -1;
    }

  }
  //Serial.println("client not available");
  return -1;

}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  pinMode(WIFI_LED,OUTPUT);
  digitalWrite(WIFI_LED,1);
  pinMode(CONNECT_LED,OUTPUT);
  digitalWrite(CONNECT_LED,1);
  //pinMode(PRESS_KEY,INPUT_PULLUP);

  //////////////////////////////////////////////////
  Serial.printf("Connecting to Wifi.");
  //////////////////////////////////////////////////
  //  WiFi.begin(ssid, password);
  //  while (WiFi.status() != WL_CONNECTED)
  //  {
  //    delay(500);
  //    Serial.print(".");
  //  }
  //  Serial.println(" connected");
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...yeey :)");
  digitalWrite(WIFI_LED,0);
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

      //Serial.write(packet.data(), packet.length());
      auto err = deserializeJson(doc, packet.data(), packet.length());

      if (err) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(err.c_str());
        return;
      }

      JsonObject obj = doc.as<JsonObject>();

      JsonArray arr = obj["Addresses"];

      for (int i = 0; i < arr.size(); i++) {

        if (ClientAddr.IP.fromString(arr[i].as<String>())) {

          ClientAddr.Port = obj["Port"];
          ClientAddr.updated = true;
          return;

        }
      }


    });
  }

}

unsigned long timer = 0;

void loop() {
  digitalWrite(CONNECT_LED,1);
  while (!ConnectClient());
  digitalWrite(CONNECT_LED,0);




  //while (ReceiveClientData() < 0);

  for (;;) {

    //Realtime Task: Connection test
    if (!client.connected()) {
      Serial.println("disconnected.");
      break;
    }

    //Realtime Task: USB
    Usb.Task();

//    //Realtime Task: Receive Client
//    ReceiveClientData();
//
//    unsigned long nowtime = millis();
//    if (timer < nowtime) {
//
//      SendCommandToClient("Airplane.Getstate");
//
//    } else if ((timer + 1000) < nowtime) {
//
//      SendCommandToClient("Airplane.GetInfo");
//
//    } else if ((timer + 2000) < nowtime) {
//
//      SendCommandToClient("InfiniteFlight.GetStatus");
//
//    } else if ((timer + 3000) < nowtime) {
//
//
//
//    }


  }

}
