// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>
#define RELAY_TYPE "relay"
#include <vector>
#define BUTTON_SWITCH 1
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

void applyJsonSwitchs(JsonArray& _switchsJson){
  for(int i  = 0 ; i < _switchsJson.size() ; i++){ 
    JsonObject& s = _switchsJson[i];      
    int id = s.get<unsigned int>("id");
    bool pullup =s.get<bool>("pullup");
    bool state =s.get<bool>("state");
    int gpioControl = s.get<unsigned int>("gpioControl");
    pinMode(id, pullup ? INPUT_PULLUP  : INPUT);
    Bounce* debouncer = new Bounce(); 
    debouncer->attach(id);
    debouncer->interval(5); // interval in ms
    _switchs.push_back({debouncer,s});
    initNormal(s.get<bool>("stateControl"),s.get<unsigned int>("gpioControl"));
  }
}
void toogleSwitch(int gpio) {
  for (unsigned int i=0; i < _switchs.size(); i++) {
    if( _switchs[i].switchJson.get<unsigned int>("id") == gpio){
    if( _switchs[i].switchJson.get<String>("typeControl").equals(RELAY_TYPE)){
      bool gpioState = toogleNormal( _switchs[i].switchJson.get<unsigned int>("gpioControl"));
       _switchs[i].switchJson.set("stateControl",gpioState);  
    }
   }
  }   
}
void triggerSwitch(bool _state,  JsonObject& switchJson) {
  _state =  switchJson.get<bool>("pullup") ? !_state : _state;
    if(switchJson.get<String>("typeControl").equals(RELAY_TYPE)){
      bool gpioState = toogleNormal(switchJson.get<unsigned int>("gpioControl"));
      switchJson.set("stateControl",gpioState);  
    }   
}

void switchNotify(int gpio, bool _gpioState){
  for (unsigned int i=0; i < _switchs.size(); i++) {
    if(_switchs[i].switchJson.get<unsigned int>("gpioControl") == gpio){
      _switchs[i].switchJson.set("stateControl",_gpioState);
    String swtr = "";
    _switchs[i].switchJson.printTo(swtr);
    publishOnEventSource("switch",swtr);
    return;
    }
  }
}

JsonArray& readStoredSwitchs(){
  
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
    SPIFFS.remove(switchsFilename);
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

void saveSwitch(JsonArray& _switchsJson){
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

void switchJson(JsonArray& switchsJson,int _id, String _typeControl, int _gpioControl, bool _stateControl, String _icon, String _name, bool _pullup, bool _state, int _mode, bool _master, String _mqttStateTopic, String _mqttCommandTopic){
      JsonObject& switchJson = switchsJson.createNestedObject();
      switchJson["id"] = _id;
      switchJson["pullup"] = _pullup;
      switchJson["gpioControl"] = _gpioControl;
      switchJson["typeControl"] = _typeControl;
      switchJson["stateControl"] = _stateControl;
      switchJson["mqttStateTopic"] = _mqttStateTopic;
      switchJson["mqttCommandTopic"] = _mqttCommandTopic;
      switchJson["master"] = _master;
      switchJson["icon"] = _icon;
      switchJson["name"] = _name;
      switchJson["mode"] = _mode;
      switchJson["state"] = _state;
}

JsonArray& createDefaultSwitchs(){
    JsonArray& switchsJson = getJsonArray();
    switchJson(switchsJson,SWITCH_ONE,RELAY_TYPE,RELAY_ONE,INIT_STATE_OFF,  "fa-lightbulb-o","Interrutor 1", BUTTON_SET_PULLUP,INIT_STATE_OFF,  BUTTON_SWITCH, BUTTON_MASTER, "", "");
    switchJson(switchsJson,SWITCH_TWO,RELAY_TYPE,RELAY_TWO, INIT_STATE_OFF, "fa-lightbulb-o","Interrutor 2", BUTTON_SET_PULLUP,INIT_STATE_OFF,  BUTTON_SWITCH, BUTTON_MASTER, "", "");
    return switchsJson;
}

void loopSwitchs(){
    for (unsigned int i=0; i < _switchs.size(); i++) {
      Bounce* b =   _switchs[i].debouncer;
      b->update();
      bool value =  b->read();
      if(_switchs[i].switchJson.get<bool>("state") != value){
        _switchs[i].switchJson.set("state",value);
        triggerSwitch( value, _switchs[i].switchJson);
      }
    }
}
