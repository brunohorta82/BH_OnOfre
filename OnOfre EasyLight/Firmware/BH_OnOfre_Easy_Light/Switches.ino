#include <Bounce2.h> // https://github.com/thomasfredericks/Bounce2
#include <vector>
#define RELAY_TYPE "relay"
#define MQTT_TYPE "mqtt"
#define SWITCH_DEVICE "switch"
#define BUTTON_SWITCH 1
#define BUTTON_PUSH 2
#define BUTTON_TOUCH 2
#define BUTTON_SET_PULLUP true
#define INIT_STATE_OFF false
#define BUTTON_MASTER false
#define BUTTON_SLAVE true



typedef struct {
    Bounce* debouncer; 
    JsonObject& switchJson;
} switch_t;
std::vector<switch_t> _switchs;

const String switchsFilename = "switchs.json";

JsonArray& saveSwitch(String _id,JsonObject& _switch){
  JsonArray& sws = getJsonArray();
  int switchFound = false;
  for (unsigned int i=0; i < _switchs.size(); i++) {
    if(_switchs[i].switchJson.get<String>("id").equals(_id)){
      switchFound = true;
      String _name = _switch.get<String>("name");
      _switchs[i].switchJson.set("gpio",_switch.get<unsigned int>("gpio"));
      _switchs[i].switchJson.set("name",_name);
      _switchs[i].switchJson.set("pullup",_switch.get<bool>("pullup"));
      _switchs[i].switchJson.set("gpioControl",_switch.get<unsigned int>("gpioControl"));
      _switchs[i].switchJson.set("typeControl",_switch.get<String>("typeControl"));
      _switchs[i].switchJson.set("master",_switch.get<bool>("master"));
      _switchs[i].switchJson.set("mode",_switch.get<unsigned int>("mode"));
      String mqttCommand = MQTT_COMMAND_TOPIC_BUILDER(_id,SWITCH_DEVICE,_name);
      _switchs[i].switchJson.set("mqttCommandTopic",mqttCommand);
      _switchs[i].switchJson.set("mqttStateTopic",MQTT_STATE_TOPIC_BUILDER(_id,SWITCH_DEVICE,_name));
      subscribeOnMqtt(mqttCommand);
 
    }
     sws.add( _switchs[i].switchJson);
  }
  if(!switchFound){
      String _name = _switch.get<String>("name");
      String _id = "B"+String(millis());
      switchJson(sws,_id,_switch.get<unsigned int>("gpio"),_switch.get<String>("typeControl"),_switch.get<unsigned int>("gpioControl"),INIT_STATE_OFF,  "fa-lightbulb-o",_name, _switch.get<bool>("pullup"),INIT_STATE_OFF,  _switch.get<unsigned int>("mode"), _switch.get<bool>("master"), MQTT_STATE_TOPIC_BUILDER(_id,SWITCH_DEVICE,_name), MQTT_COMMAND_TOPIC_BUILDER(_id,SWITCH_DEVICE,_name), "light");
    }

  saveSwitchs(sws);
  applyJsonSwitchs(sws);
 if(getConfigJson().get<bool>("homeAssistantAutoDiscovery")){
    createHALigthComponent();  
   }
  return sws;
 }

void applyJsonSwitchs(JsonArray& _switchsJson){
  _switchs.clear();
  for(int i  = 0 ; i < _switchsJson.size() ; i++){ 
    JsonObject& s = _switchsJson[i];      
    int gpio= s.get<unsigned int>("gpio");
    bool pullup =s.get<bool>("pullup");
    bool state =s.get<bool>("state");
    int gpioControl = s.get<unsigned int>("gpioControl");
    if ( gpio == 16) {
      configGpio(gpio, INPUT_PULLDOWN_16);
    } else {
      configGpio(gpio, pullup ? INPUT_PULLUP  : INPUT);
    }
    Bounce* debouncer = new Bounce(); 
    debouncer->attach(gpio);
    debouncer->interval(5); // interval in ms
    _switchs.push_back({debouncer,s});
    initNormal(s.get<bool>("stateControl"),gpioControl);
  }
}

void toogleSwitch(String id) {
  for (unsigned int i=0; i < _switchs.size(); i++) {
    if( _switchs[i].switchJson.get<String>("id").equals(id)){
    if( _switchs[i].switchJson.get<String>("typeControl").equals(RELAY_TYPE)){
      bool gpioState = toogleNormal( _switchs[i].switchJson.get<unsigned int>("gpioControl"));
       _switchs[i].switchJson.set("stateControl",gpioState);  
    }
   }
  }   
}

void toogleSwitch(String topic, String payload) {
  for (unsigned int i=0; i < _switchs.size(); i++) {
    if( _switchs[i].switchJson.get<String>("mqttCommandTopic").equals(topic)){
    if( _switchs[i].switchJson.get<String>("typeControl").equals(RELAY_TYPE)){
       int gpio = _switchs[i].switchJson.get<unsigned int>("gpioControl");
      if(String(PAYLOAD_ON).equals(payload)){
        turnOn(getRelay(gpio));
        }else if (String(PAYLOAD_OFF).equals(payload)){
        turnOff( getRelay(gpio));  
       }   
    }
   }
  }   
}
void triggerSwitch(bool _state,  JsonObject& switchJson) {
  _state =  switchJson.get<bool>("pullup") ? !_state : _state;
    if(switchJson.get<String>("typeControl").equals(RELAY_TYPE)){
      bool gpioState = toogleNormal(switchJson.get<unsigned int>("gpioControl"));
      switchJson.set("stateControl",gpioState);  
    }else if(switchJson.get<String>("typeControl").equals(MQTT_TYPE)){
      publishState( switchJson);
    }   
}

void publishState(JsonObject& switchJson){
    String swtr = "";
    switchJson.printTo(swtr);
    publishOnEventSource("switch",swtr);
    if(switchJson.get<String>("typeControl").equals(RELAY_TYPE)){
      publishOnMqtt(switchJson.get<String>("mqttStateTopic").c_str(),switchJson.get<bool>("stateControl") ? PAYLOAD_ON : PAYLOAD_OFF,true);
    }else if( switchJson.get<String>("typeControl").equals(MQTT_TYPE)){
      publishOnMqtt(switchJson.get<String>("mqttStateTopic").c_str(),switchJson.get<bool>("state") ? PAYLOAD_ON : PAYLOAD_OFF,true);
    }  
    
}

void switchNotify(int gpio, bool _gpioState){
  JsonArray& sws = getJsonArray();
  for (unsigned int i=0; i < _switchs.size(); i++) {
    JsonObject& sw =  _switchs[i].switchJson;
    if(sw.get<unsigned int>("gpioControl") == gpio){
      sw.set("stateControl",_gpioState);
      publishState( sw);
    }
     sws.add(sw);
  }
  saveSwitchs(sws);
}

JsonArray& getStoredSwitchs(){
  JsonArray& sws = getJsonArray(); 
  for (unsigned int i=0; i < _switchs.size(); i++) {
    sws.add( _switchs[i].switchJson);
  }
  return sws;
}

void loadStoredSwitchs(){
  bool loadDefaults = false;
  if(SPIFFS.begin()){
    File cFile;   
    #ifdef FORMAT
    SPIFFS.remove(switchsFilename);
    #endif
    if(SPIFFS.exists(switchsFilename)){
      cFile = SPIFFS.open(switchsFilename,"r+"); 
      if(!cFile){
        logger("[SWITCH] Create file switchs Error!");
        return;
      }
        logger("[SWITCH] Read stored file config...");
        JsonArray &storedSwitchs = getJsonArray(cFile);       
     
        if (!storedSwitchs.success()) {
         logger("[SWITCH] Json file parse Error!");
          loadDefaults = true;
        }else{
          logger("[SWITCH] Apply stored file config...");
          applyJsonSwitchs(storedSwitchs);
        }
        
     }else{
        loadDefaults = true;
     }
    cFile.close();
     if(loadDefaults){
      logger("[SWITCH] Apply default config...");
      cFile = SPIFFS.open(switchsFilename,"w+"); 
      JsonArray &defaultSwitchs = createDefaultSwitchs();
      defaultSwitchs.printTo(cFile);
      applyJsonSwitchs(defaultSwitchs);
      cFile.close();
      }
     
  }else{
     logger("[SWITCH] Open file system Error!");
  }
   SPIFFS.end(); 
   
}

void saveSwitchs(JsonArray& _switchsJson){
   if(SPIFFS.begin()){
      logger("[SWITCH] Open "+switchsFilename);
      File rFile = SPIFFS.open(switchsFilename,"w+");
      if(!rFile){
        logger("[SWITCH] Open switch file Error!");
      } else {
       
      _switchsJson.printTo(rFile);
      }
      rFile.close();
   }else{
     logger("[SWITCH] Open file system Error!");
  }
  SPIFFS.end();
  logger("[SWITCH] New switch config loaded.");
}

void switchJson(JsonArray& switchsJson,String _id,int _gpio ,String _typeControl, int _gpioControl, bool _stateControl, String _icon, String _name, bool _pullup, bool _state, int _mode, bool _master, String _mqttStateTopic, String _mqttCommandTopic, String _type){
      JsonObject& switchJson = switchsJson.createNestedObject();
      switchJson["id"] = _id;
      switchJson["gpio"] = _gpio;
      switchJson["pullup"] = _pullup;
      switchJson["gpioControl"] = _gpioControl;
      switchJson["typeControl"] = _typeControl;
      switchJson["stateControl"] = _stateControl;
      switchJson["mqttStateTopic"] = _mqttStateTopic;
      switchJson["mqttCommandTopic"] = _mqttCommandTopic;
      switchJson["mqttRetain"] = true;
      switchJson["master"] = _master;
      switchJson["icon"] = _icon;
      switchJson["name"] = _name;
      switchJson["mode"] = _mode;
      switchJson["state"] = _state;
      switchJson["locked"] = false;
      switchJson["type"] = _type;
      switchJson["class"] = SWITCH_DEVICE;
}
void rebuildSwitchMqttTopics(){
      bool store = false;
      JsonArray& _devices = getStoredSwitchs();
      for(int i  = 0 ; i < _devices.size() ; i++){ 
        store = true;
      JsonObject& d = _devices[i];      
      String id = d.get<String>("id");
      String name = d.get<String>("name");
      d.set("mqttCommandTopic",MQTT_COMMAND_TOPIC_BUILDER(id,SWITCH_DEVICE,name));
      d.set("mqttStateTopic",MQTT_STATE_TOPIC_BUILDER(id,SWITCH_DEVICE,name));
      subscribeOnMqtt(d.get<String>("mqttCommandTopic"));
    }
    if(store){
      saveSwitchs(_devices);
      if(getConfigJson().get<bool>("homeAssistantAutoDiscovery")){
        createHALigthComponent();  
      }
    }
  }
JsonArray& createDefaultSwitchs(){
    JsonArray& switchsJson = getJsonArray();
    String id1 = "B1";
    String id2 = "B2";
    switchJson(switchsJson,id1,SWITCH_ONE,RELAY_TYPE,RELAY_ONE,INIT_STATE_OFF,  "fa-lightbulb-o","Interrutor1", BUTTON_SET_PULLUP,INIT_STATE_OFF,  BUTTON_SWITCH, BUTTON_MASTER, MQTT_STATE_TOPIC_BUILDER(id1,SWITCH_DEVICE,"Interrutor1"), MQTT_COMMAND_TOPIC_BUILDER(id1,SWITCH_DEVICE,"Interrutor1"), "light");
    switchJson(switchsJson,id2,SWITCH_TWO,RELAY_TYPE,RELAY_TWO, INIT_STATE_OFF, "fa-lightbulb-o","Interrutor2", BUTTON_SET_PULLUP,INIT_STATE_OFF,  BUTTON_SWITCH, BUTTON_MASTER, MQTT_STATE_TOPIC_BUILDER(id2,SWITCH_DEVICE,"Interrutor2"),MQTT_COMMAND_TOPIC_BUILDER(id2,SWITCH_DEVICE,"Interrutor2"), "light");
    return switchsJson;
}

void loopSwitchs(){
    for (unsigned int i=0; i < _switchs.size(); i++) {
      Bounce* b =   _switchs[i].debouncer;
      b->update();
      bool value =  b->read();
      value = _switchs[i].switchJson.get<bool>("pullup") ? !value : value;
      int swmode = _switchs[i].switchJson.get<unsigned int>("mode");
          if(_switchs[i].switchJson.get<bool>("state") != value){
            _switchs[i].switchJson.set("state",value);
            if( swmode == BUTTON_SWITCH || (swmode == BUTTON_PUSH && !value) ){
              triggerSwitch( value, _switchs[i].switchJson);
            }
      }     
   }                                                                                                 
}
