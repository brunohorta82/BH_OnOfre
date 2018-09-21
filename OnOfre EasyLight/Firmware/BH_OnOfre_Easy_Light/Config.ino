
AsyncEventSource events("/events");

void logger(String payload){
  if(payload.equals(""))return;
  Serial.print("Free heap:"); Serial.println(ESP.getFreeHeap(),DEC);
   events.send(payload.c_str(), "log");
   Serial.printf((payload+"\n").c_str());
}

JsonObject& buildConfigToJson(String _nodeId, String _mqttIpDns, String _mqttUsername,String _mqttPassword ,String _wifiSSID, String _wifiSecret, String _hostname ,bool _homeAssistantAutoDiscovery,String _homeAssistantAutoDiscoveryPrefix){
      
      JsonObject& configJson = getJsonObject();
      configJson["nodeId"] = _nodeId;
      configJson["homeAssistantAutoDiscovery"] = _homeAssistantAutoDiscovery;
      configJson["homeAssistantAutoDiscoveryPrefix"] = _homeAssistantAutoDiscoveryPrefix;
      configJson["hostname"] = _hostname;
      configJson["mqttIpDns"] = _mqttIpDns;
      configJson["mqttUsername"] = _mqttUsername;
      configJson["mqttPassword"] = _mqttPassword;
      configJson["wifiSSID"] = _wifiSSID;
      configJson["wifiSecret"] = _wifiSecret;
      return configJson;
}

JsonObject& defaultConfigJson(){
   return buildConfigToJson(NODE_ID ,MQTT_BROKER_IP
   ,MQTT_USERNAME,MQTT_PASSWORD, WIFI_SSID,WIFI_SECRET,HOSTNAME,homeAssistantAutoDiscovery,homeAssistantAutoDiscoveryPrefix);
}
void requestToLoadDefaults(){
   SPIFFS.format();
   shouldReboot = true;
}
void applyJsonConfig(JsonObject& root) {
    nodeId = root["nodeId"] | NODE_ID;
    hostname = String(HARDWARE) +"-"+String(nodeId)+(nodeId == MODEL ? +"-"+String(ESP.getChipId()) : "");
    mqttIpDns=root["mqttIpDns"] | MQTT_BROKER_IP;
    mqttUsername = root["mqttUsername"] | MQTT_USERNAME;
    mqttPassword = root["mqttPassword"] | MQTT_PASSWORD;
    homeAssistantAutoDiscovery = root["homeAssistantAutoDiscovery"] | homeAssistantAutoDiscovery;
    homeAssistantAutoDiscoveryPrefix = root["homeAssistantAutoDiscoveryPrefix"] | HOME_ASSISTANT_AUTO_DISCOVERY_PREFIX;
    String lastSSID =  wifiSSID;
    String lastWifiSecrect =  wifiSecret;
    wifiSSID = root["wifiSSID"] | WIFI_SSID;
    wifiSecret = root["wifiSecret"] | WIFI_SECRET;
    if(wifiSSID != lastSSID ||  wifiSecret != lastWifiSecrect){
       jw.disconnect(); 
       jw.cleanNetworks();
       jw.addNetwork(wifiSSID.c_str(), wifiSecret.c_str());
       
    }
     updateMqttNodeId(nodeId);
   
}
 JsonObject& readStoredConfig(){
  
  if(SPIFFS.begin()){
    File cFile;   
    if(SPIFFS.exists(CONFIG_FILENAME)){
      cFile = SPIFFS.open(CONFIG_FILENAME,"r+"); 
      if(cFile){
        logger("[CONFIG] Read stored file config...");
        JsonObject &storedConfig = getJsonObject(cFile);
        storedConfig["firmwareVersion"] = FIRMWARE_VERSION;
        cFile.close();
        SPIFFS.end(); 
        if (storedConfig.success()) {
        return storedConfig;
        }else{
          logger("[CONFIF] Json file parse Error!");
        }
      }else{
        logger("[CONFIF] Create file config Error!");
        SPIFFS.end(); 
      }
    }else{
     logger("[CONFIF] File config not exists!"); 
     }
    }else{
     logger("[CONFIG] Open file system Error!");
   }
   return jsonBuffer.createObject(); 
}

void loadStoredConfiguration(){
  bool loadDefaults = false;
  
  if(SPIFFS.begin()){
    File cFile;   
    if(SPIFFS.exists(CONFIG_FILENAME)){
      cFile = SPIFFS.open(CONFIG_FILENAME,"r+"); 
      if(!cFile){
        logger("[CONFIF] Create file config Error!");
        return;
      }
        logger("[CONFIG] Read stored file config...");
        JsonObject &storedConfig = getJsonObject(cFile);
        if (!storedConfig.success()) {
         logger("[CONFIF] Json file parse Error!");
          loadDefaults = true;
        }else{
          logger("[CONFIG] Apply stored file config...");
          applyJsonConfig(storedConfig);
        }
        
     }else{
        loadDefaults = true;
      }
    cFile.close();
     if(loadDefaults){
      logger("[CONFIG] Apply default config...");
      cFile = SPIFFS.open(CONFIG_FILENAME,"w+"); 
      JsonObject &defaultConfig = defaultConfigJson();
      defaultConfig.printTo(cFile);
      applyJsonConfig(defaultConfig);
      cFile.close();
      }
     
  }else{
     logger("[CONFIG] Open file system Error!");
  }
   SPIFFS.end(); 
   
}


bool checkRebootRules(JsonObject& root){
  return nodeId != (root["nodeId"]  | NODE_ID);
}

void checkServices(JsonObject& root){
  restartMqtt =  mqttIpDns != (root["mqttIpDns"] | MQTT_BROKER_IP) || mqttUsername != (root["mqttUsername"] | MQTT_USERNAME) || mqttPassword != (root["mqttPassword"] | MQTT_PASSWORD);
}


void saveConfig(String _nodeId,  String _mqttIpDns, String _mqttUsername,String _mqttPassword ,String _wifiSSID, String _wifiSecret, String _hostname, bool _homeAssistantAutoDiscovery,String _homeAssistantAutoDiscoveryPrefix){
    JsonObject& newConfig = buildConfigToJson( _nodeId, _mqttIpDns,  _mqttUsername, _mqttPassword , _wifiSSID,  _wifiSecret,  _hostname,_homeAssistantAutoDiscovery, _homeAssistantAutoDiscoveryPrefix);
    hostname = String(HARDWARE) +"-"+String(nodeId)+(nodeId == MODEL ? +"-"+String(ESP.getChipId()) : "");
   if(SPIFFS.begin()){
      File rFile = SPIFFS.open(CONFIG_FILENAME,"w+");
      if(!rFile){
        logger("[CONFIG] Open config file Error!");
      } else {
       
      newConfig.printTo(rFile);
      }
      rFile.close();
   }else{
     logger("[CONFIG] Open file system Error!");
  }
  SPIFFS.end();
  configNeedsUpdate = false;
  logger("[CONFIG] New config loaded.");
  shouldReboot = checkRebootRules(newConfig);
  if(!shouldReboot){
   checkServices(newConfig);
   applyJsonConfig(newConfig);
  }
}
