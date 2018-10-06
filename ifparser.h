#include <ArduinoJson.h>
#include <IPAddress.h>

struct AirplaneFlapsConfiguration{

    int FlapsAngle;
    String FlapsName;
    String ShortName;
    int SlatsAngle;

};

struct AirplaneState{

    uint8_t Result;
    //String Type;
    double AccelerationX;
    double AccelerationY;
    double AccelerationZ;
    double AltitudeAGL;
    double AltitudeMSL;
    String ApproachAirportICAO;
    double ApproachDistance;
    double ApproachHorizontalAngle;
    String ApproachRunway;
    double ApproachVerticalAngle;
    double Bank;
    double CourseTrue;
    uint8_t FlapsIndex;
    double GForce;
    uint8_t GearState;
    double GroundSpeed;
    double GroundSpeedKts;
    double HeadingMagnetic;
    double HeadingTrue;
    double IndicatedAirspeed;
    double IndicatedAirspeedKts;
    bool IsAutopilotOn;
    bool IsBraking;
    bool IsCrashed;
    bool IsLanded;
    bool IsOnGround;
    bool IsOnRunway;
    bool IsOverLandingWeight;
    bool IsOverTakeoffWeight;
    bool IsPushbackActive;
    //Location
    double AltitudeLight;
    double Latitude;
    double Longitude;

    double MachNumber;
    double MagneticDeviation;
    double Pitch;
    bool ReverseThrustState;
    double SideForce;
    uint8_t SpoilersPosition;
    uint8_t StallProximity;
    bool StallWarning;
    bool Stalling;
    double TrueAirspeed;
    double Velocity;
    double VerticalSpeed;
    double Weight;
    double WeightPercentage;

    //Aircraftinfo
    uint8_t EngineCount;
    uint8_t FlapsCount;
    AirplaneFlapsConfiguration FlapsConfiguration[10];
    uint8_t FuelTankCount;
    bool HasAutopilot;
    String AircraftName;
    uint8_t SpoilerType;

    //DeviceInfo
    IPAddress ClientAddress;
    int ClientPort;
    String ApiVersion;
    String AppState;
    String AppVersion;
    String DeviceName;
    int DisplayHeight;
    int DisplayWidth;
    //String LoggedInUser;
    //String PlayMode;

};

void APIAircraftStateParser(JsonObject& root);
void APIAircraftInfoParser(JsonObject& root);
void APIDeviceInfoParser(JsonObject& root);
