#include "hidjoystickrptparser.h"

JoystickReportParser::JoystickReportParser(JoystickEvents *evt) :joyEvents(evt),oldHat(0x08),oldButtons(0) {
        oldPad.X = 0x0000;
        oldPad.Y = 0x0000;
        oldPad.Z = 0x00;
        oldPad.T = 0x00;
}

void JoystickReportParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
        bool match = true;

        // Checking if there are changes in report since the method was last called
        GamePadEventData pad;
        pad.X = ((buf[1] << 8) | buf[0]) & 0x3ff;
        pad.Y = ((buf[1] >> 2) | buf[2] << 6) & 0x3ff;
        pad.Z = buf[3];
        pad.T = buf[5];
        if(memcmp(&pad,&oldPad,sizeof(oldPad)))
          match = false;

        // Calling Game Pad event handler
        if (!match && joyEvents) {
                joyEvents->OnGamePadChanged(&pad);

                oldPad = pad;
        }

        uint8_t hat = buf[2] >> 4;

        // Calling Hat Switch event handler
        if (hat != oldHat && joyEvents) {
                joyEvents->OnHatSwitch(hat);
                oldHat = hat;
        }

        uint16_t buttons = buf[6] << 8 | buf[4];
        uint16_t changes = (buttons ^ oldButtons);

        // Calling Button Event Handler for every button changed
        if (changes) {
                for (uint8_t i = 0; i < KEYNUM; i++) {
                        uint16_t mask = (0x0001 << i);

                        if (((mask & changes) > 0) && joyEvents) {
                                if ((buttons & mask) > 0)
                                        joyEvents->OnButtonDn(i + 1);
                                else
                                        joyEvents->OnButtonUp(i + 1);
                        }
                }
                oldButtons = buttons;
        }
}

void SendCommandToClient(String Cmd/*, APICommand Cmd*/) {

  // We now create a URI for the request
  doc.clear();
  JsonObject root = doc.to<JsonObject>();

  root["Command"] = Cmd;
  root.createNestedArray("Parameters");

  String JsonCommand;
  serializeJson(root, JsonCommand);

  //Serial.println(JsonCommand);
  uint32_t strsize = JsonCommand.length();

  client.write((uint8_t*)(&strsize), 4);          //size
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

  client.write((uint8_t*)(&strsize), 4);          //size
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

  client.write((uint8_t*)(&strsize), 4);          //size
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

  //Serial.println(JsonCommand);
  uint32_t strsize = JsonCommand.length();

  client.write((uint8_t*)(&strsize), 4);          //size
  client.write(JsonCommand.c_str(), strsize);     //data

}

void JoystickEvents::OnGamePadChanged(const GamePadEventData *evt) {
  //Serial.print("X: ");
  //Serial.print(evt->X, DEC);
  SendJoystickToClient(1, map(evt->X, 0, 1024, -1024, 1024));

  //Serial.print("\tY: ");
  //Serial.print(evt->Y, DEC);
  SendJoystickToClient(0, map(evt->Y, 0, 1024, -1024, 1024));

  //Serial.print("\tZ: ");
  //Serial.print(evt->Z, DEC);
  SendJoystickToClient(2, map(evt->Z, 0, 255, -1024, 1024));

  //Serial.print("\tT: ");
  //Serial.println(evt->T, DEC);
  SendJoystickToClient(3, map(evt->T, 0, 255, -1024, 1024));

}

void JoystickEvents::OnHatSwitch(uint8_t hat) {
  //Serial.print("Hat Switch: ");
  //Serial.print(hat, HEX);
  //Serial.println("");
  switch (hat) {
    case 0:
      SendPOVToClient(0, 1);
      break;
    case 2:
      SendPOVToClient(1, 0);
      break;
    case 4:
      SendPOVToClient(0, -1);
      break;
    case 6:
      SendPOVToClient(-1, 0);
      break;
    case 8:
      SendPOVToClient(0, 0);
      break;
  }
}

void JoystickEvents::OnButtonUp(uint8_t but_id) {

  SendButtonToClient(but_id, false);
}

void JoystickEvents::OnButtonDn(uint8_t but_id) {

  SendButtonToClient(but_id, true);
}
