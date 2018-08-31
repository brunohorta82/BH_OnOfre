

void createHALigthComponent(AsyncMqttClient* _mqttClient){
  for(int i = 0 ; i < NUMBER_OF_MQTT_RELAYS; i++){
    _mqttClient->publish(("homeassistant/light/"+nodeId+"/relay_"+String(i+1)+"/config").c_str(),0,true,("{\"name\": \""+String(HARDWARE)+"_"+nodeId+"_relay_"+String(i+1)+"\", \"state_topic\": \""+MQTT_RELAY_TOPIC(i,false)+"\",\"availability_topic\": \""+availableTopic+"\", \"command_topic\": \""+MQTT_RELAY_TOPIC(i,true)+"\", \"retain\": true,\"payload_available\":\"1\",\"payload_not_available\":\"0\"}").c_str());
    _mqttClient->subscribe(MQTT_RELAY_TOPIC(i,true).c_str(),1);
   }
}
