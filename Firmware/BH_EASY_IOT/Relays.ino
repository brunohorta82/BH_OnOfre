#include <Ticker.h>
#define RELAY_DEVICE "relay"
#define NORMAL false
#define INVERTED true

JsonArray& rls = getJsonArray();

const String relaysFilename = "relays.json";

void removeRelay(String _id){
  int relayFound = false;
  int index = 0;
  for (unsigned int i=0; i < rls.size(); i++) {
    JsonObject& relayJson = rls.get<JsonVariant>(i);   
    if(relayJson.get<String>("id").equals(_id)){
      relayFound = true;
      index  = i;
    }
  }
  if(relayFound){
    rls.remove(index);
     
    }

  saveRelays();
  applyJsonRelays();
}
JsonArray& saveRelay(String _id,JsonObject& _relay){
  bool relayFound = false;
  for (unsigned int i=0; i < rls.size(); i++) {
    JsonObject& relayJson = rls.get<JsonVariant>(i);  
    if(relayJson.get<String>("id").equals(_id)){
      relayFound = true;
      relayJson.set("gpio",_relay.get<unsigned int>("gpio"));
      relayJson.set("inverted",_relay.get<bool>("inverted"));
    }
  }
  if(!relayFound){
      String _id = "R"+String(millis());
     relayJson(_id,_relay.get<unsigned int>("gpio"),_relay.get<bool>("inverted"),_relay.get<String>("name"),_relay.get<unsigned int>("maxAmp"),"fa-circle-o-notch");
    }
  saveRelays();
  applyJsonRelays();
  return rls;
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
    for (unsigned int i=0; i < rls.size(); i++) {
    JsonObject& relayJson = rls.get<JsonVariant>(i);
    if(relayJson.get<unsigned int>("gpio") == gpio){
      return relayJson ;
     }
    }
  return getJsonObject();
}

JsonArray& getStoredRelays(){
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
           for(int i = 0 ; i< storedRelays.size(); i++){
            rls.add(storedRelays.get<JsonVariant>(i));
            }
          applyJsonRelays();
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
      applyJsonRelays();
      cFile.close();
      }
     
  }else{
     logger("[RELAY] Open file system Error!");
  }
   SPIFFS.end(); 
   
}
void applyJsonRelays(){
  for(int i  = 0 ; i < rls.size() ; i++){ 
  JsonObject& relayJson = rls.get<JsonVariant>(i);     
    int gpio = relayJson.get<unsigned int>("gpio");
    configGpio(gpio, OUTPUT);
  }
}
void saveRelays(){
   if(SPIFFS.begin()){
      logger("[RELAY] Open "+relaysFilename);
      File rFile = SPIFFS.open(relaysFilename,"w+");
      if(!rFile){
        logger("[RELAY] Open relays file Error!");
      } else {
       
      rls.printTo(rFile);
      }
      rFile.close();
   }else{
     logger("[RELAY] Open file system Error!");
  }
  SPIFFS.end();
  logger("[RELAY] New relays config loaded.");
}
void relayJson(String _id,long _gpio, bool _inverted, String _name, int _maxAmp, String _icon){
      JsonObject& relayJson =getJsonObject();
      relayJson.set("id" , _id);
      relayJson.set("gpio" , _gpio);
      relayJson.set("inverted" , _inverted);
      relayJson.set("icon" , _icon);
      relayJson.set("name" , _name);
      relayJson.set("maxAmp", _maxAmp);
      relayJson.set("state", false);
      relayJson.set("class", RELAY_DEVICE);
      rls.add(relayJson);
}

JsonArray& createDefaultRelays(){
    #ifdef BHONOFRE
    relayJson("R1",RELAY_ONE,NORMAL,"Relé 1",2,"fa-circle-o-notch");
    relayJson("R2",RELAY_TWO,NORMAL,"Relé 2",2,"fa-circle-o-notch");
    #endif
    return rls;
}
