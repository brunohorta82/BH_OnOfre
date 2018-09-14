
AsyncMqttClient mqttClient; 
Ticker mqttReconnectTimer;
String baseTopic = String(HARDWARE)+"/"+nodeId;
String availableTopic = String(HARDWARE)+"_"+nodeId+"/status";

String MQTT_TOPIC_BUILDER(String device,int number, bool command){
 return baseTopic+"/"+device+"_"+String(number)+"/"+(command ? "set" : "status");
}
void updateMqttNodeId(String _nodeId){
  baseTopic = String(HARDWARE)+"/"+_nodeId;
  availableTopic = String(HARDWARE)+"_"+_nodeId+"/status";
  }
String getAvailableTopic(){
  return availableTopic;
  }
void onMqttConnect(bool sessionPresent) {
    logger("[MQTT] Connected to MQTT.");
    mqttClient.publish(availableTopic.c_str(),0,true,"1");
    if(homeAssistantAutoDiscovery){
      createHALigthComponent(&mqttClient);  
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
  if(!topic.startsWith("/")){
    topic = "/"+topic;
    }
    Serial.println(nodeId);
    Serial.println(baseTopic+topic);
  mqttClient.publish((baseTopic+topic).c_str(), 0,retain,payload.c_str());
 }

 void processMqttAction(String topic, String payload){
  mqttqttRelayControl(topic,payload);
 }
