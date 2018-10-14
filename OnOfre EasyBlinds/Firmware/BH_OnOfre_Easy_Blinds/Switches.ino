#include <Bounce2.h> // https://github.com/thomasfredericks/Bounce2
#define RELAY_TYPE "relay"
#define CLOSE "CLOSE"
#define STOP "STOP"
#define OPEN "OPEN"
#define NONE "NONE"
#define SWITCH_DEVICE "switch"
#include <vector>
#define BUTTON_SWITCH 1
#define BUTTON_PUSH 2
#define BUTTON_TOUCH 2
#define BUTTON_SET_PULLUP true
#define INIT_STATE_OFF false
#define BUTTON_MASTER false
#define BUTTON_SLAVE true
bool storeState = false;

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
      //_switchs[i].switchJson.set("pullup",_switch.get<bool>("pullup"));
      //_switchs[i].switchJson.set("gpioControl",_switch.get<unsigned int>("gpioControl"));
      //_switchs[i].switchJson.set("gpioControlClose",_switch.get<unsigned int>("gpioControlOpen"));
      //_switchs[i].switchJson.set("gpioControlOpen",_switch.get<unsigned int>("gpioControlClose"));
      //_switchs[i].switchJson.set("typeControl",_switch.get<String>("typeControl"));
      //_switchs[i].switchJson.set("master",_switch.get<bool>("master"));
      _switchs[i].switchJson.set("mode",_switch.get<unsigned int>("mode"));
      String mqttCommand = MQTT_COMMAND_TOPIC_BUILDER(_id,SWITCH_DEVICE,_name);
      _switchs[i].switchJson.set("mqttCommandTopic",mqttCommand);
      _switchs[i].switchJson.set("mqttStateTopic",MQTT_STATE_TOPIC_BUILDER(_id,SWITCH_DEVICE,_name));
      subscribeOnMqtt(mqttCommand);
 
    }
     sws.add( _switchs[i].switchJson);
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
    configGpio(gpio, pullup ? INPUT_PULLUP  : INPUT);
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
void stateSwitch(String id, String state) {
  for (unsigned int i=0; i < _switchs.size(); i++) {
    if( _switchs[i].switchJson.get<String>("id").equals(id)){
    if( _switchs[i].switchJson.get<String>("typeControl").equals(RELAY_TYPE)){
      int gpioOpen = _switchs[i].switchJson.get<unsigned int>("gpioControlOpen");
      int gpioClose = _switchs[i].switchJson.get<unsigned int>("gpioControlClose");
      if(String("OPEN").equals(state)){
        openAction(gpioClose,gpioOpen);
        }else if(String("STOP").equals(state)){
          stopAction(gpioClose,gpioOpen);
        }if(String("CLOSE").equals(state)){
          closeAction(gpioClose,gpioOpen);
        }
        }
        }
    }
}
  
void openAction(int gpioClose, int gpioOpen){
  logger("[SWITCH] OPEN");
  turnOff( getRelay(gpioClose));
  delay(50);  
  turnOn( getRelay(gpioOpen));
}
void closeAction(int gpioClose, int gpioOpen){
  logger("[SWITCH] CLOSE");
  turnOff( getRelay(gpioOpen));
  delay(50);  
  turnOn( getRelay(gpioClose));
}
void stopAction(int gpioClose, int gpioOpen){
  logger("[SWITCH] STOP");
  turnOff( getRelay(gpioClose));  
  turnOff( getRelay(gpioOpen));
}

void toogleSwitch(String topic, String payload) {
  for (unsigned int i=0; i < _switchs.size(); i++) {
    if( _switchs[i].switchJson.get<String>("mqttCommandTopic").equals(topic)){
    if( _switchs[i].switchJson.get<String>("typeControl").equals(RELAY_TYPE)){
      int gpio = _switchs[i].switchJson.get<unsigned int>("gpioControl");
       int gpioOpen = _switchs[i].switchJson.get<unsigned int>("gpioControlOpen");
       int gpioClose = _switchs[i].switchJson.get<unsigned int>("gpioControlClose");
      if(String(PAYLOAD_ON).equals(payload)){
        turnOn(getRelay(gpio));
        }else if (String(PAYLOAD_OFF).equals(payload)){
        turnOff( getRelay(gpio));  
       }else if (String(PAYLOAD_OPEN).equals(payload)){
          openAction(gpioClose,gpioOpen);
       } else if (String(PAYLOAD_CLOSE).equals(payload)){
        closeAction(gpioClose,gpioOpen);
       } else if (String(PAYLOAD_STOP).equals(payload)){
        stopAction(gpioClose,gpioOpen);  
       }    
    }
   }
  }   
}


void triggerSwitch(bool _state,  JsonObject& switchJson) {
  _state =  switchJson.get<bool>("pullup") ? !_state : _state;
    if(switchJson.get<String>("typeControl").equals(RELAY_TYPE)){
        if(switchJson.get<String>("stateAction").equals(CLOSE)){
          stateSwitch(switchJson.get<String>("id"), OPEN);
          switchJson.set("stateAction",OPEN);
        }else if(switchJson.get<String>("stateAction").equals(OPEN)){
           stateSwitch(switchJson.get<String>("id"),CLOSE);
           switchJson.set("stateAction",CLOSE);  
        }else{
           stateSwitch(switchJson.get<String>("id"),OPEN);
           switchJson.set("stateAction",OPEN);
          }     
    }
}

void switchNotify(int gpio, bool _gpioState){
  JsonArray& sws = getJsonArray();
  for (unsigned int i=0; i < _switchs.size(); i++) {
    if(_switchs[i].switchJson.get<unsigned int>("gpioControl") == gpio){
      _switchs[i].switchJson.set("stateControl",_gpioState);
    String swtr = "";
    _switchs[i].switchJson.printTo(swtr);
    publishOnEventSource("switch",swtr);
    publishOnMqtt(_switchs[i].switchJson.get<String>("mqttStateTopic").c_str(),_gpioState ? PAYLOAD_ON : PAYLOAD_OFF,true);
    }else  if(_switchs[i].switchJson.get<unsigned int>("gpioControlOpen") == gpio){
      
      }else  if(_switchs[i].switchJson.get<unsigned int>("gpioControlClose") == gpio){
      
      }
     sws.add( _switchs[i].switchJson);
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

void switchJson(JsonArray& switchsJson,String _id,int _gpio ,String _typeControl, int _gpioControl,int _gpioControlOpen,int _gpioControlClose, bool _stateControl, String _icon, String _name, bool _pullup, bool _state, int _mode, bool _master, String _mqttStateTopic, String _mqttCommandTopic, String _type){
      JsonObject& switchJson = switchsJson.createNestedObject();
      switchJson["id"] = _id;
      switchJson["gpio"] = _gpio;
      switchJson["pullup"] = _pullup;
      switchJson["gpioControl"] = _gpioControl;
      switchJson["gpioControlOpen"] = _gpioControlOpen;
      switchJson["gpioControlClose"] = _gpioControlClose;
      switchJson["stateAction"] = STOP;
      switchJson["pauseAction"] = NONE;
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
    switchJson(switchsJson,id1,SWITCH_ONE,RELAY_TYPE,-1,RELAY_ONE,RELAY_TWO,INIT_STATE_OFF,  "fa-window-maximize","Persiana1", BUTTON_SET_PULLUP,INIT_STATE_OFF,  BUTTON_SWITCH, BUTTON_MASTER, MQTT_STATE_TOPIC_BUILDER(id1,SWITCH_DEVICE,"Persiana"), MQTT_COMMAND_TOPIC_BUILDER(id1,SWITCH_DEVICE,"Interrutor1"), "cover");    
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
