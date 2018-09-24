#include <dht_nonblocking.h>
#include <vector>
#define TIME_READINGS_DELAY 30000ul
#define SENSOR_DEVICE  "sensor"
#define SENSOR_PIN 14
#define TEMPERATURE_TYPE 1
#define HUMIDITY_TYPE 2

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

void sensorJson(JsonArray& sensorsJson,String _id,int _gpio ,bool _disabled, String _icon, String _name,  int _type,JsonArray& functions){
      JsonObject& sensorJson = sensorsJson.createNestedObject();
      sensorJson.set("id", _id);
      sensorJson.set("gpio", _gpio);
      sensorJson.set("icon", _icon);
      sensorJson.set("name", _name);
      sensorJson.set("disabled", _disabled);
      sensorJson.set("type", _type);
      sensorJson.set("class", SENSOR_DEVICE);
      sensorJson.set("functions", functions);
   
}

JsonArray& createDefaultSensors(){
    JsonArray& sensorsJson = getJsonArray();
    String id1 = "S1";
    JsonArray& functionsJson = getJsonArray();
    createFunctionArray(functionsJson,"Temperatura","fa-thermometer-half","ºC",MQTT_STATE_TOPIC_BUILDER(id1,SENSOR_DEVICE,"temperature"),false,TEMPERATURE_TYPE);
    createFunctionArray(functionsJson,"Humidade","fa-percent","%", MQTT_STATE_TOPIC_BUILDER(id1,SENSOR_DEVICE,"humidity"),false, HUMIDITY_TYPE);
    sensorJson(sensorsJson ,id1,SENSOR_PIN,DISABLE,  "fa-microchip","Temp e Hum", DHT_TYPE_11,functionsJson);
    return  sensorsJson ;
}

void createFunctionArray(JsonArray& functionsJson,String _name, String _icon, String _unit, String _mqttStateTopic, bool _retain, int _type ){
    JsonObject& functionJson = functionsJson.createNestedObject();
      functionJson.set("name", _name);
      functionJson.set("icon", _icon);
      functionJson.set("unit", _unit);
      functionJson.set("type", _type);
      functionJson.set("mqttStateTopic", _mqttStateTopic);
      functionJson.set("mqttRetain", _retain);
}
void loopSensors(){
   static unsigned long measurement_timestamp = millis( );
   if( millis( ) - measurement_timestamp > TIME_READINGS_DELAY ){
    for (unsigned int i=0; i < _sensors.size(); i++) {
      if(_sensors[i].sensorJson.get<bool>("disabled")){
        continue;
        }
    switch(_sensors[i].sensorJson.get<unsigned int>("type")){
      case DHT_TYPE_11:
      case DHT_TYPE_21:
      case DHT_TYPE_22:
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

        /* String sntr = "";
        _sensors[0].sensorJson.printTo(sntr);
        publishOnEventSource("sensor",sntr);*/
        }
   }
      break;
      }
    }
    }
}
JsonArray& saveSensor(String _id,JsonObject& _sensor){
  JsonArray& sns = getJsonArray();
  for (unsigned int i=0; i < _sensors.size(); i++) {
    if(_sensors[i].sensorJson.get<String>("id").equals(_id)){
      String _name = _sensor.get<String>("name");
      _sensors[i].sensorJson.set("gpio",_sensor.get<unsigned int>("gpio"));
      _sensors[i].sensorJson.set("name",_name);
      _sensors[i].sensorJson.set("disabled",_sensor.get<bool>("disabled"));
      _sensors[i].sensorJson.set("type",_sensor.get<unsigned int>("type"));
    }
     sns.add(  _sensors[i].sensorJson);
  }

  saveSensors(sns);
  applyJsonSensors(sns);
 if(getConfigJson().get<bool>("homeAssistantAutoDiscovery")){
    createHASensorComponent();  
   }
  return sns;
}
void saveSensors(JsonArray& _sensorsJson){
   if(SPIFFS.begin()){
      logger("[SENSORS] Open "+sensorsFilename);
      File rFile = SPIFFS.open(sensorsFilename,"w+");
      if(!rFile){
        logger("[SENSORS] Open sensors file Error!");
      } else {
      _sensorsJson.printTo(rFile);
      }
      rFile.close();
   }else{
     logger("[SENSORS] Open file system Error!");
  }
  SPIFFS.end();
  logger("[SENSORS] New sensors config loaded.");
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
      case DHT_TYPE_11:
      case DHT_TYPE_22:
      {
        DHT_nonblocking* dht_sensor = new DHT_nonblocking( gpio,type );
        _sensors.push_back({s, dht_sensor});
      }
      break;
     }
     
  }

}
