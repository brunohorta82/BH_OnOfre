#include <Ticker.h>
#define RELAY_DEVICE "relay"
#define NORMAL false
#define INVERTED true
typedef struct {
    JsonObject& relayJson;
} relay_t;
std::vector<relay_t> _relays;

const String relaysFilename = "relays.json";

JsonArray& saveRelay(String _id,JsonObject& _relay){
  JsonArray& rs = getJsonArray();
  for (unsigned int i=0; i < _relays.size(); i++) {
    if(_relays[i].relayJson.get<String>("id").equals(_id)){
      _relays[i].relayJson.set("gpio",_relay.get<unsigned int>("gpio"));
      _relays[i].relayJson.set("inverted",_relay.get<bool>("inverted"));
    }
     rs.add( _relays[i].relayJson);
  }
  saveRelay(rs);
  applyJsonRelays(rs);
  return rs;
 }
void turnOn(JsonObject& relay) {
  int gpio = relay.get<unsigned int>("gpio");
  bool inverted = relay.get<bool>("inverted");
  String name = relay.get<String>("name");
  digitalWrite( gpio,inverted ? LOW : HIGH);
  switchNotify(gpio, inverted ? !digitalRead(gpio) : digitalRead(gpio) );
  logger("[RELAY "+name+" GPIO: "+String(gpio)+"] ON");
}

void turnOff(JsonObject& relay) {
  int gpio = relay.get<unsigned int>("gpio");
  bool inverted = relay.get<bool>("inverted");
  String name = relay.get<String>("name");
  digitalWrite( gpio,inverted ? HIGH : LOW);
  switchNotify(gpio, inverted ? !digitalRead(gpio) : digitalRead(gpio) );
  logger("[RELAY "+name+" GPIO: "+String(gpio)+"] OFF");
}

void initNormal(bool state,int gpio){
  if(state){
    turnOn(getRelay(gpio));
   }else{
    turnOff(getRelay(gpio));
   }
}
 
bool toogleNormal(int gpio){
  if(digitalRead(gpio)){
    turnOff(getRelay(gpio));
   }else{
    turnOn(getRelay(gpio));
   }
   return digitalRead(gpio);
}

JsonObject& getRelay(int gpio){
    for (unsigned int i=0; i < _relays.size(); i++) {
    JsonObject& r = _relays[i].relayJson;      
    if(r.get<unsigned int>("gpio") == gpio){
      return r;
     }
    }
  return getJsonObject();
}

JsonArray& getStoredRelays(){
  JsonArray& rls = getJsonArray(); 
  for (unsigned int i=0; i < _relays.size(); i++) {
    rls.add( _relays[i].relayJson);
  }
  return rls;
}


void loadStoredRelays(){
  bool loadDefaults = false;
  if(SPIFFS.begin()){
    File cFile;
    #ifdef FORMAT
    SPIFFS.remove(relaysFilename);
    #endif
    if(SPIFFS.exists(relaysFilename)){
      cFile = SPIFFS.open(relaysFilename,"r+"); 
      if(!cFile){
        logger("[RELAY] Create file relays Error!");
        return;
      }
        logger("[RELAY] Read stored file config...");
        JsonArray &storedRelays = getJsonArray(cFile);
        if (!storedRelays.success()) {
         logger("[RELAY] Json file parse Error!");
          loadDefaults = true;
        }else{
          logger("[RELAY] Apply stored file config...");
          applyJsonRelays(storedRelays);
        }
        
     }else{
        loadDefaults = true;
     }
    cFile.close();
     if(loadDefaults){
      logger("[RELAY] Apply default config...");
      cFile = SPIFFS.open(relaysFilename,"w+"); 
      JsonArray &defaultRelays = createDefaultRelays();
      defaultRelays.printTo(cFile);
      applyJsonRelays(defaultRelays);
      cFile.close();
      }
     
  }else{
     logger("[RELAY] Open file system Error!");
  }
   SPIFFS.end(); 
   
}
void applyJsonRelays(JsonArray& _relaysJson){
  _relays.clear();
  for(int i  = 0 ; i < _relaysJson.size() ; i++){ 
    JsonObject& r = _relaysJson[i];      
    int gpio = r.get<unsigned int>("gpio");
    _relays.push_back({r});
    pinMode(gpio, OUTPUT);
  }
}
void saveRelay(JsonArray& _relaysJson){
   if(SPIFFS.begin()){
      logger("[RELAY] Open "+relaysFilename);
      File rFile = SPIFFS.open(relaysFilename,"w+");
      if(!rFile){
        logger("[RELAY] Open relays file Error!");
      } else {
       
      _relaysJson.printTo(rFile);
      }
      rFile.close();
   }else{
     logger("[RELAY] Open file system Error!");
  }
  SPIFFS.end();
  logger("[RELAY] New relays config loaded.");
}
void relayJson(JsonArray& relaysJson,String _id,long _gpio, bool _inverted, String _name, int _maxAmp, String _icon){
      JsonObject& relayJson = relaysJson.createNestedObject();
      relayJson["id"] = _id;
      relayJson["gpio"] = _gpio;
      relayJson["inverted"] = _inverted;
      relayJson["icon"] = _icon;
      relayJson["name"] = _name;
      relayJson["maxAmp"] = _maxAmp;
      relayJson["state"] = false;
      relayJson["class"] = RELAY_DEVICE;
}

JsonArray& createDefaultRelays(){
    JsonArray& relaysJson = getJsonArray();
    relayJson(relaysJson,"R1",RELAY_ONE,NORMAL,"Relé 1",2,"fa-circle-o-notch");
    relayJson(relaysJson,"R2",RELAY_TWO,NORMAL,"Relé 2",2,"fa-circle-o-notch");
    return relaysJson;
}
