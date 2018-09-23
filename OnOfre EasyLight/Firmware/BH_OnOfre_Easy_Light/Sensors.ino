#include <dht_nonblocking.h>
#include <vector>
#define TIME_READINGS_DELAY 30000ul
#define SENSOR_DEVICE  "sensor"
#define SENSOR_PIN 14
#define TEMPERATURE_TYPE 1
#define HUMIDITY_TYPE 2
#define SENSOR_DHT_11_TYPE 11
#define SENSOR_DHT_22_TYPE 22
#define DHT_SENSOR_TYPE DHT_TYPE_11

typedef struct {
    JsonObject& sensorJson;
    DHT_nonblocking* dht;
} sensor_t;
std::vector<sensor_t> _sensors;

const String sensorsFilename = "sensors.json";

JsonArray& getStoredSensors(){
  JsonArray& sws = getJsonArray(); 
  for (unsigned int i=0; i < _sensors.size(); i++) {
    sws.add( _sensors[i].sensorJson);
  }
  return sws;
}


void sensorJson(JsonArray& sensorsJson,long _id,int _gpio ,bool _disabled, String _icon, String _name,  int _type,JsonArray& functions){
      JsonObject& sensorJson = sensorsJson.createNestedObject();
      sensorJson["id"] = _id;
      sensorJson["gpio"] = _gpio;
      sensorJson["icon"] = _icon;
      sensorJson["name"] = _name;
      sensorJson["disabled"] = _disabled;
      sensorJson["type"] = _type;
      sensorJson["class"] = SENSOR_DEVICE;
      sensorJson["functions"] = functions;
   
}

JsonArray& createDefaultSensors(){
    JsonArray& sensorsJson = getJsonArray();
    long id1 = 1;
    JsonArray& functionsJson = getJsonArray();
    createFunctionArray(functionsJson,"Temperatura","fa-thermometer-half","ºC",MQTT_STATE_TOPIC_BUILDER(id1,SENSOR_DEVICE,"temperature"),false,TEMPERATURE_TYPE);
    createFunctionArray(functionsJson,"Humidade","fa-percent","%", MQTT_STATE_TOPIC_BUILDER(id1,SENSOR_DEVICE,"humidity"),false, HUMIDITY_TYPE);
    sensorJson(sensorsJson ,id1,SENSOR_PIN,DISABLE,  "fa-microchip","Temp e Hum", SENSOR_DHT_11_TYPE,functionsJson);
    return  sensorsJson ;
}

void createFunctionArray(JsonArray& functionsJson,String _name, String _icon, String _unit, String _mqttStateTopic, bool _retain, int _type ){
    JsonObject& functionJson = functionsJson.createNestedObject();
      functionJson["name"] = _name;
      functionJson["icon"] = _icon;
      functionJson["unit"] = _unit;
      functionJson["type"] = _type;
      functionJson["mqttStateTopic"] = _mqttStateTopic;
      functionJson["mqttRetain"] = _retain;
}
void loopSensors(){
   static unsigned long measurement_timestamp = millis( );
   if( millis( ) - measurement_timestamp > TIME_READINGS_DELAY ){
    for (unsigned int i=0; i < _sensors.size(); i++) {
      if(_sensors[i].sensorJson.get<bool>("disabled")){
        continue;
        }
    switch(_sensors[i].sensorJson.get<unsigned int>("type")){
      case SENSOR_DHT_11_TYPE:
      float temperature; 
      float humidity;
    if(_sensors[i].dht->measure( &temperature, &humidity ) == true ){
        measurement_timestamp = millis( );
        JsonArray& functions = _sensors[i].sensorJson.get<JsonVariant>("functions");
        for(int i  = 0 ; i < functions.size() ; i++){
          JsonObject& f = functions[i];    
          String _mqttState =f.get<String>("mqttStateTopic");
          int _type =f.get<unsigned int>("type");
          bool _retain =f.get<bool>("mqttRetain");   
          if(_type == TEMPERATURE_TYPE){
            publishOnMqttQueue(_mqttState ,String(  temperature,1),_retain);
          }else if(_type == HUMIDITY_TYPE){
            publishOnMqttQueue(_mqttState ,String( humidity,1),_retain);
          }

        /*String sntr = "";
        _sensors[0].sensorJson.printTo(sntr);
        publishOnEventSource("sensor",sntr);*/
        }
   }
      break;
      case SENSOR_DHT_22_TYPE:
      
      break;
      }
    }
    }
}

void loadStoredSensors(){
  bool loadDefaults = false;
  if(SPIFFS.begin()){
    File cFile;   
   #ifdef FORMAT
    SPIFFS.remove(sensorsFilename);
    #endif
    if(SPIFFS.exists(sensorsFilename)){
      cFile = SPIFFS.open(sensorsFilename,"r+"); 
      if(!cFile){
        logger("[SENSORS] Create file Sensor Error!");
        return;
      }
        logger("[SENSORS] Read stored file config...");
        JsonArray &storedSensors = getJsonArray(cFile);
        if (!storedSensors.success()) {
         logger("[SENSORS] Json file parse Error!");
          loadDefaults = true;
        }else{
          logger("[SENSORS] Apply stored file config...");
          applyJsonSensors(storedSensors);
        }
        
     }else{
        loadDefaults = true;
     }
    cFile.close();
     if(loadDefaults){
      logger("[SENSORS] Apply default config...");
      cFile = SPIFFS.open(sensorsFilename,"w+"); 
      JsonArray &defaultSensors = createDefaultSensors();
      defaultSensors.printTo(cFile);
      applyJsonSensors(defaultSensors);
       
      cFile.close();
      }
     
  }else{
     logger("[SWITCH] Open file system Error!");
  }
   SPIFFS.end(); 
   
}

void applyJsonSensors(JsonArray& _sensorsJson){
  _sensors.clear();
  for(int i  = 0 ; i < _sensorsJson.size() ; i++){ 
    JsonObject& s = _sensorsJson[i];
    int gpio= s.get<unsigned int>("gpio");
    int type= s.get<unsigned int>("type");
    switch(type){
      case SENSOR_DHT_11_TYPE:{
        DHT_nonblocking* dht_sensor = new DHT_nonblocking( gpio, DHT_TYPE_11 );
        _sensors.push_back({s, dht_sensor});
      }
      break;
      case SENSOR_DHT_22_TYPE:
        
      break;
     }
     
  }

}
