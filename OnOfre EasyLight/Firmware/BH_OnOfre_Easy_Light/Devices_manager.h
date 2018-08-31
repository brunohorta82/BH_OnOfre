typedef enum {
  PUSH,
  NORMAL,
  PULSE_1S,
  PULSE_2S,
  PULSE_5S
  } switchType;
typedef enum{
  REED_NO,
  REED_NC,
  DHT_11,
  DHT_22,PIR
}sensorModel; 
typedef enum {
 MQTT_TOPIC,
 ONOFRE,
 RELAY,
 DISPLAY_I2C
} switchControlType;
   
typedef enum {
  T_INVERTED,
  T_NORMAL
}relayType;

typedef enum{
  TURN_OFF,
  TURN_ON,
  NONE_AUTO
}relayAutomationType;

typedef enum{
  ONE_WIRE
}sensorType;

typedef enum{
  ON_MQTT,ON_ROM
}saveState;


typedef enum{
  I2C
}displayType;

typedef enum{
  SSD1306
}displayModel;


String prepareDevices(JsonArray& devicesArrays){

}

void prepareRelay(JsonObject& relayJson){
 
 }

 
void prepareDisplay(JsonObject& displayJson){
 
 }

 void prepareSensor(JsonObject& sensorJson){
 
 }

 void prepareSwitch(JsonObject& switchJson){
 
 }

 
