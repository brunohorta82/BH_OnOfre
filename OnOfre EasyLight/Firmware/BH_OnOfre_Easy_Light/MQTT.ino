
AsyncMqttClient mqttClient; 
Ticker mqttReconnectTimer;
String baseTopic = String(HARDWARE)+"/"+nodeId;
String availableTopic = String(HARDWARE)+"/"+nodeId+"/status";

String MQTT_COMMAND_TOPIC_BUILDER( int _id,String _class, String _name){
 return baseTopic+"/"+_class+"/"+"/"+_name+"/"+String(_id)+"/set";
}
String MQTT_STATE_TOPIC_BUILDER( int _id,String _class, String _name){
 return baseTopic+"/"+_class+"/"+_name+"/"+String(_id)+"/status";
}
void updateMqttNodeId(String _nodeId){
  baseTopic = String(HARDWARE)+"/"+_nodeId;
  availableTopic = String(HARDWARE)+"_"+_nodeId+"/status";
  rebuildSwitchMqttTopics();
}
String getAvailableTopic(){
  return availableTopic;
  }
void onMqttConnect(bool sessionPresent) {
    logger("[MQTT] Connected to MQTT.");
    mqttClient.publish(availableTopic.c_str(),0,true,"1");
    registerMqttDevices();
}
void registerMqttDevices(){
  if(homeAssistantAutoDiscovery){
      createHALigthComponent(readStoredSwitchs());  
    }else{
      JsonArray& _devices = readStoredSwitchs();
      for(int i  = 0 ; i < _devices.size() ; i++){ 
      JsonObject& d = _devices[i];      
      String _mqttCommand =d.get<String>("mqttCommandTopic");
      subscribeOnMqtt(_mqttCommand.c_str());
    }
  }
}
void mqttSend(String topic, bool retain, String payload){
  mqttClient.publish(topic.c_str(),0,retain,payload.c_str());
 }
void connectToMqtt() {
  logger("[MQTT] Connecting to MQTT ["+mqttIpDns+"]...");
  mqttClient.connect();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  logger("[MQTT] Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  String topicStr = String(topic);
  logger("[MQTT] TOPIC: "+topicStr);
       String payloadStr = "";
  for (int i=0; i<len; i++) {
    payloadStr += payload[i];
  }
  logger("[MQTT] PAYLOAD: "+payloadStr);
  processMqttAction(topicStr,payloadStr);
}

void setupMQTT() {
  if(WiFi.status() != WL_CONNECTED || mqttIpDns.equals(""))return;
  mqttClient.disconnect();
  mqttClient.onConnect(onMqttConnect);
  mqttClient.setClientId((String(HARDWARE)+"_"+nodeId).c_str());
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setCredentials(mqttUsername.c_str(),mqttPassword.c_str());
  mqttClient.setCleanSession(false);
  mqttClient.setWill(availableTopic.c_str(),0,true,"0");
  mqttClient.setServer( mqttIpDns.c_str(), MQTT_BROKER_PORT);
  connectToMqtt();
}

void publishOnMqtt(String topic,String payload, bool retain){
  mqttClient.publish(topic.c_str(), 0,retain,payload.c_str());
 }
void subscribeOnMqtt(String topic){
  mqttClient.subscribe(topic.c_str(), 0);
 }
 void processMqttAction(String topic, String payload){
    toogleSwitch(topic, payload);
 }
