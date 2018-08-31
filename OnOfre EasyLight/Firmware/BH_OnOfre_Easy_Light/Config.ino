
AsyncEventSource events("/events");
void logger(String payload){
  if(payload.equals(""))return;
  Serial.print("Free heap:"); Serial.println(ESP.getFreeHeap(),DEC);
   events.send(payload.c_str(), "log");
   Serial.printf((payload+"\n").c_str());
}

JsonObject& buildConfigToJson(String _nodeId, String _mqttIpDns, String _mqttUsername,String _mqttPassword ,String _wifiSSID, String _wifiSecret, String _hostname ,bool _homeAssistantAutoDiscovery,String _homeAssistantAutoDiscoveryPrefix, String _switchIO12Name, String _switchIO13Name){
      DynamicJsonBuffer jsonBuffer(CONFIG_BUFFER_SIZE);
      JsonObject& configJson = jsonBuffer.createObject();
      configJson["nodeId"] = _nodeId;
      configJson["homeAssistantAutoDiscovery"] = _homeAssistantAutoDiscovery;
      configJson["homeAssistantAutoDiscoveryPrefix"] = _homeAssistantAutoDiscoveryPrefix;
      configJson["switchIO12Name"] = _switchIO12Name;
      configJson["switchIO13Name"] = _switchIO13Name;
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
   ,MQTT_USERNAME,MQTT_PASSWORD, WIFI_SSID,WIFI_SECRET,HOSTNAME,homeAssistantAutoDiscovery,homeAssistantAutoDiscoveryPrefix,SWITCH_IO12_NAME,SWITCH_IO13_NAME);
}
void requestToLoadDefaults(){
   SPIFFS.format();
   shouldReboot = true;
}
void applyJsonConfig(JsonObject& root) {
    nodeId = root["nodeId"] | NODE_ID;
    hostname = String(HARDWARE) +"-"+String(nodeId);
    baseTopic = String(HARDWARE)+"/"+nodeId;
    availableTopic = String(HARDWARE)+"_"+nodeId+"/status";
    mqttIpDns=root["mqttIpDns"] | MQTT_BROKER_IP;
    mqttUsername = root["mqttUsername"] | MQTT_USERNAME;
    mqttPassword = root["mqttPassword"] | MQTT_PASSWORD;
    switchIO12Name = root["switchIO12Name"] | SWITCH_IO12_NAME;
    switchIO13Name = root["switchIO13Name"] |SWITCH_IO13_NAME;
    homeAssistantAutoDiscovery = root["homeAssistantAutoDiscovery"] | homeAssistantAutoDiscovery;
    homeAssistantAutoDiscoveryPrefix = root["homeAssistantAutoDiscoveryPrefix"] | homeAssistantAutoDiscovery;
    String lastSSID =  wifiSSID;
    String lastWifiSecrect =  wifiSecret;
    wifiSSID = root["wifiSSID"] | WIFI_SSID;
    wifiSecret = root["wifiSecret"] | WIFI_SECRET;
    if(wifiSSID != lastSSID ||  wifiSecret != lastWifiSecrect){
       jw.disconnect(); 
       jw.cleanNetworks();
       jw.addNetwork(wifiSSID.c_str(), wifiSecret.c_str());
       
    }
   
}
 JsonObject& readStoredConfig(){
  DynamicJsonBuffer jsonBuffer(CONFIG_BUFFER_SIZE);
  if(SPIFFS.begin()){
    File cFile;   
    if(SPIFFS.exists(CONFIG_FILENAME)){
      cFile = SPIFFS.open(CONFIG_FILENAME,"r+"); 
      if(cFile){
        logger("[CONFIG] Read stored file config...");
        JsonObject &storedConfig = jsonBuffer.parseObject(cFile);
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
  DynamicJsonBuffer jsonBuffer(CONFIG_BUFFER_SIZE);
  if(SPIFFS.begin()){
    File cFile;   
    if(SPIFFS.exists(CONFIG_FILENAME)){
      cFile = SPIFFS.open(CONFIG_FILENAME,"r+"); 
      if(!cFile){
        logger("[CONFIF] Create file config Error!");
        return;
      }
        logger("[CONFIG] Read stored file config...");
        JsonObject &storedConfig = jsonBuffer.parseObject(cFile);
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


void saveConfig(String _nodeId,  String _mqttIpDns, String _mqttUsername,String _mqttPassword ,String _wifiSSID, String _wifiSecret, String _hostname, bool _homeAssistantAutoDiscovery,String _homeAssistantAutoDiscoveryPrefix, String _switchIO12Name, String _switchIO13Name){
    JsonObject& newConfig = buildConfigToJson( _nodeId, _mqttIpDns,  _mqttUsername, _mqttPassword , _wifiSSID,  _wifiSecret,  _hostname,_homeAssistantAutoDiscovery, _homeAssistantAutoDiscoveryPrefix, _switchIO12Name,  _switchIO13Name);
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
