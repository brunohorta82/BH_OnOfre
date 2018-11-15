#define MQTT_BROKER_PORT 1883
AsyncMqttClient mqttClient; 
Ticker mqttReconnectTimer;
/**
 * REASONS
  TCP_DISCONNECTED = 0,
  MQTT_UNACCEPTABLE_PROTOCOL_VERSION = 1,
  MQTT_IDENTIFIER_REJECTED = 2,
  MQTT_SERVER_UNAVAILABLE = 3,
  MQTT_MALFORMED_CREDENTIALS = 4,
  MQTT_NOT_AUTHORIZED = 5
 */
char * usernameMqtt = 0;
char * passwordMqtt = 0;
String getBaseTopic(){
 return  String(HARDWARE)+"/"+getConfigJson().get<String>("nodeId");
} 
String getAvailableTopic(){
  return String(HARDWARE)+"/"+getConfigJson().get<String>("nodeId")+"/available";
} 

typedef struct {
    String topic;
    String payload;
    bool retain;
} message_t;
std::vector<message_t> _messages;

String MQTT_COMMAND_TOPIC_BUILDER( String _id,String _class, String _name){
 return getBaseTopic()+"/"+_class+"/"+_id+"/set";
}

String MQTT_STATE_TOPIC_BUILDER( String _id,String _class, String _name){
 return getBaseTopic()+"/"+_class+"/"+_id+"/status";
}

void onMqttConnect(bool sessionPresent) {
    logger("[MQTT] Connected to MQTT.");
    mqttClient.publish(getAvailableTopic().c_str(),0,true,"1");
    registerMqttDevices();
}
void registerMqttDevices(){
  if(getConfigJson().get<bool>("homeAssistantAutoDiscovery")){
      realoadHaConfig();
    }else{
      JsonArray& _devices = getStoredSwitchs();
      for(int i  = 0 ; i < _devices.size() ; i++){ 
      JsonObject& d = _devices[i];      
      String _mqttCommand =d.get<String>("mqttCommandTopic");
      subscribeOnMqtt(_mqttCommand.c_str());
    }
  }
}

void connectToMqtt() {
  logger("[MQTT] Connecting to MQTT ["+getConfigJson().get<String>("mqttIpDns")+"]...");
  if(!getMqttState()){
    mqttClient.connect();
  }
}
bool getMqttState(){
  return mqttClient.connected();
  }
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  logger("[MQTT] Disconnected from MQTT. Reason: "+String(static_cast<uint8_t>(reason)));
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
  if(mqttClient.connected()){
    mqttClient.disconnect();
  }
  if(WiFi.status() != WL_CONNECTED || getConfigJson().get<String>("mqttIpDns").equals(""))return;
  mqttClient.onConnect(onMqttConnect);
  const static String clientId = getHostname();
  mqttClient.setClientId(clientId.c_str());
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  char * ipDnsMqtt = strdup(getConfigJson().get<String>("mqttIpDns").c_str());
  usernameMqtt = strdup(getConfigJson().get<String>("mqttUsername").c_str());
  passwordMqtt =strdup(getConfigJson().get<String>("mqttPassword").c_str());
  mqttClient.setCredentials(usernameMqtt,passwordMqtt);
  mqttClient.setWill(getAvailableTopic().c_str(),0,true,"0");
  mqttClient.setCleanSession(false);
  mqttClient.setServer(ipDnsMqtt, MQTT_BROKER_PORT);
  connectToMqtt();
  reloadMqttConfiguration = false;
}


void reloadMqttConfig(){
    reloadMqttConfiguration = true;
}
void publishOnMqttQueue(String topic,String payload, bool retain){
  if(mqttClient.connected() && !topic.equals("null")){
    _messages.push_back({topic,payload,retain});
  }
}
void publishOnMqtt(String topic,String payload, bool retain){
  if(mqttClient.connected() && !topic.equals("null")){
    mqttClient.publish(topic.c_str(), 0,retain,payload.c_str());
  }
}
void publishOnMqtt(String topic,JsonObject& payloadJson, bool retain){
  if(mqttClient.connected() && !topic.equals("null")){
    String payload = "";
    payloadJson.printTo(payload);
    mqttClient.publish(topic.c_str(), 0,retain,payload.c_str());
  }
}
long lastMessage = 0;
void mqttMsgDigest(){
    if(_messages.empty()){
      lastMessage = 0;
      return;
    }
    if(lastMessage + 500 < millis()){
      message_t m =_messages.back();
      mqttClient.publish(m.topic.c_str(), 0,true,m.payload.c_str());
      _messages.pop_back();
      lastMessage = millis();
    }
}
void subscribeOnMqtt(String topic){
  mqttClient.subscribe(topic.c_str(), 0);
 }
 void processMqttAction(String topic, String payload){
    mqttSwitchControl(topic, payload);
 }
