#include "ifparser.h"

AirplaneState CurrentAirplane;

IFClient ClientAddr;
DynamicJsonDocument doc;

void APIAircraftStateParser(JsonObject& root) {

  CurrentAirplane.Result = root["Result"];
  CurrentAirplane.AccelerationX = root["AccelerationX"];
  CurrentAirplane.AccelerationY = root["AccelerationY"];
  CurrentAirplane.AccelerationZ = root["AccelerationZ"];
  CurrentAirplane.AltitudeAGL = root["AltitudeAGL"];
  CurrentAirplane.AltitudeMSL = root["AltitudeMSL"];
  CurrentAirplane.ApproachAirportICAO = root["ApproachAirportICAO"].as<const char*>();
  CurrentAirplane.ApproachDistance = root["ApproachDistance"];
  CurrentAirplane.ApproachHorizontalAngle = root["ApproachHorizontalAngle"];
  CurrentAirplane.ApproachRunway = root["ApproachRunway"].as<const char*>();
  CurrentAirplane.ApproachVerticalAngle = root["ApproachVerticalAngle"];
  CurrentAirplane.Bank = root["Bank"];
  CurrentAirplane.CourseTrue = root["CourseTrue"];
  CurrentAirplane.FlapsIndex = root["FlapsIndex"];
  CurrentAirplane.GForce = root["GForce"];
  CurrentAirplane.GearState = root["GearState"];
  CurrentAirplane.GroundSpeed = root["GroundSpeed"];
  CurrentAirplane.GroundSpeedKts = root["GroundSpeedKts"];
  CurrentAirplane.HeadingMagnetic = root["HeadingMagnetic"];
  CurrentAirplane.HeadingTrue = root["HeadingTrue"];
  CurrentAirplane.IndicatedAirspeed = root["IndicatedAirspeed"];
  CurrentAirplane.IndicatedAirspeedKts = root["IndicatedAirspeedKts"];
  CurrentAirplane.IsAutopilotOn = root["IsAutopilotOn"];
  CurrentAirplane.IsBraking = root["IsBraking"];
  CurrentAirplane.IsCrashed = root["IsCrashed"];
  CurrentAirplane.IsLanded = root["IsLanded"];
  CurrentAirplane.IsOnGround = root["IsOnGround"];
  CurrentAirplane.IsOnRunway = root["IsOnRunway"];
  CurrentAirplane.IsOverLandingWeight = root["IsOverLandingWeight"];
  CurrentAirplane.IsOverTakeoffWeight = root["IsOverTakeoffWeight"];
  CurrentAirplane.IsPushbackActive = root["IsPushbackActive"];

  JsonObject Location = root["Location"].as<JsonObject>();
  //Location
  CurrentAirplane.AltitudeLight = Location["AltitudeLight"];
  CurrentAirplane.Latitude = Location["Latitude"];
  CurrentAirplane.Longitude = Location["Longitude"];

  CurrentAirplane.MachNumber = root["MachNumber"];
  CurrentAirplane.MagneticDeviation = root["MagneticDeviation"];
  CurrentAirplane.Pitch = root["Pitch"];
  CurrentAirplane.ReverseThrustState = root["ReverseThrustState"];
  CurrentAirplane.SideForce = root["SideForce"];
  CurrentAirplane.SpoilersPosition = root["SpoilersPosition"];
  CurrentAirplane.StallProximity = root["StallProximity"];
  CurrentAirplane.StallWarning = root["StallWarning"];
  CurrentAirplane.Stalling = root["Stalling"];
  CurrentAirplane.TrueAirspeed = root["TrueAirspeed"];
  CurrentAirplane.Velocity = root["Velocity"];
  CurrentAirplane.VerticalSpeed = root["VerticalSpeed"];
  CurrentAirplane.Weight = root["Weight"];
  CurrentAirplane.WeightPercentage = root["WeightPercentage"];

}

void APIAircraftInfoParser(JsonObject& root) {

  //Aircraftinfo
  CurrentAirplane.EngineCount = root["EngineCount"];

  JsonArray FlapsConfArray = root["FlapsConfiguration"].as<JsonArray>();
  CurrentAirplane.FlapsCount = FlapsConfArray.size();
  for (int i = 0; i < CurrentAirplane.FlapsCount; i++) {

    JsonObject EachFlaps = FlapsConfArray[i].as<JsonObject>();
    CurrentAirplane.FlapsConfiguration[i].FlapsAngle = EachFlaps["FlapsAngle"];
    CurrentAirplane.FlapsConfiguration[i].FlapsName = EachFlaps["Name"].as<const char*>();
    CurrentAirplane.FlapsConfiguration[i].ShortName = EachFlaps["ShortName"].as<const char*>();
    CurrentAirplane.FlapsConfiguration[i].SlatsAngle = EachFlaps["SlatsAngle"];

  }
  //CurrentAirplane.FlapsCount = root["Result"];
  CurrentAirplane.FuelTankCount = root["FuelTankCount"];
  CurrentAirplane.HasAutopilot = root["HasAutopilot"];
  CurrentAirplane.AircraftName = root["Name"].as<const char*>();
  CurrentAirplane.SpoilerType = root["SpoilerType"];

}

void APIDeviceInfoParser(JsonObject& root) {

  CurrentAirplane.ApiVersion = root["ApiVersion"].as<const char*>();
  CurrentAirplane.AppState = root["AppState"].as<const char*>();
  CurrentAirplane.AppVersion = root["AppVersion"].as<const char*>();
  CurrentAirplane.DeviceName = root["DeviceName"].as<const char*>();
  CurrentAirplane.DisplayHeight = root["DisplayHeight"];
  CurrentAirplane.DisplayWidth = root["DisplayWidth"];
  //LoggedInUser
  //PlayMode

}

void ParseTCPRecivedData(uint8_t* data, size_t& len) {

  doc.clear();
  DeserializationError error = deserializeJson(doc, data, len);

  if (error) {
    Serial.print(F("###Json Parser Failed: "));
    Serial.println(error.c_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();

  String MsgType = root["Type"];
  if (MsgType == "Fds.IFAPI.APIAircraftState") {
    Serial.println("Fds.IFAPI.APIAircraftState");
    APIAircraftStateParser(root);
    serializeJsonPretty(root, Serial);
    return;

  } else if (MsgType == "Fds.IFAPI.APIAircraftInfo") {
    Serial.println("Fds.IFAPI.APIAircraftInfo");
    APIAircraftInfoParser(root);
    serializeJsonPretty(root, Serial);
    return;

  } else if (MsgType == "Fds.IFAPI.IFAPIStatus") {
    Serial.println("Fds.IFAPI.IFAPIStatus");
    APIDeviceInfoParser(root);
    serializeJsonPretty(root, Serial);
    return;

  } else {
    Serial.print("###Msg Type Not Support: ");
    Serial.println(MsgType.c_str());
    return;
  }



}

void ParseUDPRecivedData(uint8_t* data, size_t& len) {

  DeserializationError err = deserializeJson(doc, data, len);

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

}

void SaveClientAddr(IFClient addr) {  //保存上次客户端地址到EEPROM
  int address = 0;
  EEPROM.writeUInt(address, uint32_t(addr.IP));
  address += sizeof(uint32_t(addr.IP));
  EEPROM.writeUShort(address, addr.Port);
  EEPROM.commit();
}

bool LoadClientAddr(IFClient& cli) { //读取上次客户端地址到EEPROM
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

bool ConnectClient() {  //从UDP或者EEPROM连接客户端

  if (ClientAddr.updated) {
    client.connect(ClientAddr.IP, ClientAddr.Port);
    if (client.connected()) {
      ClientAddr.IP.printTo(Serial);
      Serial.println("Connected.From UDP");
      ClientAddr.updated = false;
      SaveClientAddr(ClientAddr);

    }
    return true;
  }

  IFClient lastClient;
  if (LoadClientAddr(lastClient)) {
    client.connect(lastClient.IP, lastClient.Port);
    if (client.connected()) {
      Serial.print("Connected.From EEPROM");
      lastClient.IP.printTo(Serial);
      Serial.println(lastClient.Port);
      return true;
    }

  }

  return false;

}

void SendCommandToClient(String Cmd/*, APICommand Cmd*/) {

  // We now create a URI for the request
  doc.clear();
  JsonObject root = doc.to<JsonObject>();

  root["Command"] = Cmd;
  root.createNestedArray("Parameters");

  String JsonCommand;
  serializeJson(root, JsonCommand);

  Serial.println(JsonCommand);
  uint32_t strsize = JsonCommand.length();

  client.write((char*)(&strsize), 4);          //size
  client.write(JsonCommand.c_str(), strsize);     //data

}

void SendJoystickToClient(uint8_t Joyname, int16_t Joyvalue) {

  // We now create a URI for the request
  doc.clear();
  JsonObject root = doc.to<JsonObject>();

  root["Command"] = "NetworkJoystick.SetAxisValue";
  JsonArray param = root.createNestedArray("Parameters");
  JsonObject paramvalue = param.createNestedObject();
  paramvalue["Name"] = (String)Joyname;
  paramvalue["Value"] = (String)Joyvalue;


  //Encode and Send:
  String JsonCommand;
  serializeJson(root, JsonCommand);

  //Serial.println(JsonCommand);
  uint32_t strsize = JsonCommand.length();

  client.write((char*)(&strsize), 4);          //size
  client.write(JsonCommand.c_str(), strsize);     //data

}

void SendPOVToClient(int8_t xValue, int8_t yValue) {

  // We now create a URI for the request
  doc.clear();
  JsonObject root = doc.to<JsonObject>();

  root["Command"] = "NetworkJoystick.SetPOVState";
  JsonArray param = root.createNestedArray("Parameters");
  JsonObject paramvalue = param.createNestedObject();
  paramvalue["Name"] = "X";
  paramvalue["Value"] = (String)xValue;
  paramvalue = param.createNestedObject();
  paramvalue["Name"] = "Y";
  paramvalue["Value"] = (String)yValue;

  //Encode and Send:
  String JsonCommand;
  serializeJson(root, JsonCommand);

  //Serial.println(JsonCommand);
  uint32_t strsize = JsonCommand.length();

  client.write((char*)(&strsize), 4);          //size
  client.write(JsonCommand.c_str(), strsize);     //data

}

void SendButtonToClient(uint8_t btnNum, bool isPress) {

  // We now create a URI for the request
  doc.clear();
  JsonObject root = doc.to<JsonObject>();

  root["Command"] = "NetworkJoystick.SetButtonState";
  JsonArray param = root.createNestedArray("Parameters");
  JsonObject paramvalue = param.createNestedObject();
  paramvalue["Name"] = (String)btnNum;
  paramvalue["Value"] = isPress ? "Down" : "Up";

  //Encode and Send:
  String JsonCommand;
  serializeJson(root, JsonCommand);

  Serial.println(JsonCommand);
  uint32_t strsize = JsonCommand.length();

  client.write((char*)(&strsize), 4);          //size
  client.write(JsonCommand.c_str(), strsize);     //data

}
