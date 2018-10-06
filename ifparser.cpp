#include "ifparser.h"

AirplaneState CurrentAirplane;

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
