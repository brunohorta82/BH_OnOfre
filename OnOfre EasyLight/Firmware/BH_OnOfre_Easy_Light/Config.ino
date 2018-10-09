AsyncEventSource events("/events");

JsonObject& configJson = getJsonObject();

void logger(String payload){
  if(payload.equals(""))return;
  Serial.print("Free heap:"); Serial.println(ESP.getFreeHeap(),DEC);
   events.send(payload.c_str(), "log");
   Serial.printf((payload+"\n").c_str());
}
 

void resetToFactoryConfig(){
   SPIFFS.format();
   shouldReboot = true;
}

JsonObject& getConfigJson(){
 return configJson;
}

String getHostname(){
  String nodeId = configJson.get<String>("nodeId");
  return   String(HARDWARE) +"-"+nodeId+(nodeId == MODEL ? +"-"+String(ESP.getChipId()) : "");
}
void applyUpdateConfig(double outdatedVersion){
  if(outdatedVersion < 1.4){
      JsonArray& _devices =  getStoredSensors();
      for(int i  = 0 ; i < _devices.size() ; i++){ 
        JsonObject& d = _devices[i]; 
        JsonArray& functions = d.get<JsonVariant>("functions");
        for(int i  = 0 ; i < functions.size() ; i++){
          JsonObject& f = functions[i];    
          String _name = f.get<String>("name");
          if(_name.equals("Temperatura")){
            f.set("uniqueName","temperature");
          }else if(_name.equals("Humidade")){
            f.set("uniqueName","humidity");
          }
          
        }
        rebuildSensorsMqttTopics();     
    }
  
  }
}

void loadStoredConfiguration(){
  bool configFail = true;
  if(SPIFFS.begin()){
    File cFile;   
    if(SPIFFS.exists(CONFIG_FILENAME)){
      cFile = SPIFFS.open(CONFIG_FILENAME,"r+"); 
      if(cFile){
        logger("[CONFIG] Read stored file config...");
        JsonObject& storedConfig = getJsonObject(cFile);
        if (storedConfig.success()) {
          configJson.set("nodeId",storedConfig.get<String>("nodeId"));
          configJson.set("homeAssistantAutoDiscovery",storedConfig.get<bool>("homeAssistantAutoDiscovery"));
          configJson.set("homeAssistantAutoDiscoveryPrefix",storedConfig.get<String>("homeAssistantAutoDiscoveryPrefix"));
          configJson.set("hostname",storedConfig.get<String>("hostname"));
          configJson.set("mqttIpDns",storedConfig.get<String>("mqttIpDns"));
          configJson.set("mqttUsername",storedConfig.get<String>("mqttUsername"));
          configJson.set("mqttPassword",storedConfig.get<String>("mqttPassword"));
          configJson.set("wifiSSID",storedConfig.get<String>("wifiSSID"));
          configJson.set("wifiSecret", storedConfig.get<String>("wifiSecret"));
          configJson.set("wifiIp", storedConfig.get<String>("wifiIp"));
          configJson.set("wifiMask", storedConfig.get<String>("wifiMask"));
          configJson.set("wifiGw", storedConfig.get<String>("wifiGwt"));
          configJson.set("staticIp", storedConfig.get<bool>("staticIp"));
          configJson.set("apSecret", storedConfig.get<String>("apSecret"));
          configJson.set("firmwareVersion", FIRMWARE_VERSION);
          double configVersion = storedConfig.get<double>("configVersion");
          if(configVersion < FIRMWARE_VERSION){
               logger("[CONFIG] CONFIG VERSION STARTED");
               cFile.close();
               SPIFFS.end(); 
               applyUpdateConfig(configVersion);
               logger("[CONFIG] UPDATE REQUIRED");
               applyUpdateConfig(configVersion);
               configJson.set("configVersion", FIRMWARE_VERSION);
               saveConfig();
               loadStoredConfiguration();
               return;
          }else{
                logger("[CONFIG] CONFIG UPDATED Version "+String(configVersion));
            }
           
          logger("[CONFIG] Apply stored file config with success...");
          cFile.close();
          configFail = false;
        }
      }
    }
    
  if(configFail){
    logger("[CONFIG] Apply default config...");
    cFile = SPIFFS.open(CONFIG_FILENAME,"w+"); 
    configJson.set("nodeId",NODE_ID);
    configJson.set("homeAssistantAutoDiscovery", false);
    configJson.set("homeAssistantAutoDiscoveryPrefix", HOME_ASSISTANT_AUTO_DISCOVERY_PREFIX);
    configJson.set("hostname",getHostname());
    configJson.set("mqttIpDns",MQTT_BROKER_IP);
    configJson.set("mqttUsername", MQTT_USERNAME);
    configJson.set("mqttPassword",MQTT_PASSWORD);
    configJson.set("wifiSSID", WIFI_SSID);
    configJson.set("wifiSecret", WIFI_SECRET);
    configJson.set("configVersion", FIRMWARE_VERSION);
    configJson.printTo(cFile);
  }
  SPIFFS.end(); 
  }else{
    logger("[CONFIG] File system error...");
   }
}


JsonObject& saveNode(JsonObject& nodeConfig){
  //bool rebootChecker = !nodeConfig.get<String>("nodeId").equals(configJson.get<String>("nodeId")); 
  String  nodeId = nodeConfig.get<String>("nodeId");
  if(nodeId != nullptr && !configJson.get<String>("nodeId").equals(nodeId)){
    configJson.set("nodeId",nodeId);
    saveConfig();
    reloadWiFiConfig();
    reloadMqttConfig();
    rebuildSwitchMqttTopics();
    rebuildSensorsMqttTopics();
    //shouldReboot = rebootChecker;
  }
  return configJson;
} 

JsonObject& saveWifi(JsonObject& _config){
  configJson.set("wifiSSID",_config.get<String>("wifiSSID"));
  configJson.set("wifiSecret", _config.get<String>("wifiSecret"));
  configJson.set("wifiIp", _config.get<String>("wifiIp"));
  configJson.set("wifiMask", _config.get<String>("wifiMask"));
  configJson.set("wifiGw", _config.get<String>("wifiGwt"));
  configJson.set("staticIp", _config.get<bool>("staticIp"));
  configJson.set("apSecret", _config.get<String>("apSecret"));
  saveConfig();
  reloadWiFiConfig();
  return configJson;
} 

JsonObject& saveMqtt(JsonObject& _config){
  configJson.set("mqttIpDns",_config.get<String>("mqttIpDns"));
  configJson.set("mqttUsername",_config.get<String>("mqttUsername"));
  configJson.set("mqttPassword",_config.get<String>("mqttPassword"));
  saveConfig();
  reloadMqttConfig();
  return configJson;
} 

JsonObject& saveHa(JsonObject& _config){
  configJson.set("homeAssistantAutoDiscovery",_config.get<bool>("homeAssistantAutoDiscovery"));
  configJson.set("homeAssistantAutoDiscoveryPrefix",_config.get<String>("homeAssistantAutoDiscoveryPrefix"));
  saveConfig();
  realoadHaConfig();
  return configJson;
} 

void saveConfig(){
   if(SPIFFS.begin()){
      File rFile = SPIFFS.open(CONFIG_FILENAME,"w+");
      if(!rFile){
        logger("[CONFIG] Open config file Error!");
      } else {
       
      configJson.printTo(rFile);
      }
      rFile.close();
   }else{
     logger("[CONFIG] Open file system Error!");
  }
  SPIFFS.end();
  logger("[CONFIG] New config stored.");
  
}
