

void createHALigthComponent( JsonArray& _devices){
  for(int i  = 0 ; i < _devices.size() ; i++){ 
    JsonObject& d = _devices[i];      
    int _id = d.get<unsigned int>("id");
    String  _type = d.get<String>("type");
    String _class =d.get<String>("class");
    String _name =d.get<String>("name");
    String _mqttCommand =d.get<String>("mqttCommandTopic");
    String _mqttState =d.get<String>("mqttStateTopic");
    bool _retain =d.get<bool>("mqttRetain");
    publishOnMqtt((homeAssistantAutoDiscoveryPrefix+"/"+_type+"/"+nodeId+"/"+_class+"_"+String(_id)+"/config"),("{\"name\": \""+_name+"\", \"state_topic\": \""+_mqttState+"\",\"availability_topic\": \""+getAvailableTopic()+"\", \"command_topic\": \""+_mqttCommand+"\", \"retain\": "+String(_retain)+",\"payload_available\":\"1\",\"payload_not_available\":\"0\"}"),true);
    subscribeOnMqtt(_mqttCommand.c_str());
   }
}
