#include <WiFi.h>
#include "AsyncUDP.h"
#include <ArduinoJson.h>

#include <SPI.h>
#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>
#include "hidjoystickrptparser.h"

const char* ssid = "tplink";
const char* password = "11392590";

AsyncUDP udp;
WiFiClient client;
#define ListenUdpPort 15000  // local port to listen on

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

void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  //////////////////////////////////////////////////
  Serial.printf("Connecting to %s ", ssid);
  //////////////////////////////////////////////////
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

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

void loop() {

  while (!ConnectClient());

  while (1) {
    Usb.Task();
    delay(10);

    if(!client.connected()){
      Serial.println("disconnected.");
      break;
      
      }
    

  }

}
