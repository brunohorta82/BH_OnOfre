

void createHALigthComponent(){
  JsonArray& _devices = getStoredSwitchs();
  for(int i  = 0 ; i < _devices.size() ; i++){ 
    JsonObject& d = _devices[i];      
    int _id = d.get<unsigned int>("id");
    String  _type = d.get<String>("type");
    String _class =d.get<String>("class");
    String _name =d.get<String>("name");
    String _mqttCommand =d.get<String>("mqttCommandTopic");
    String _mqttState =d.get<String>("mqttStateTopic");
    bool _retain =d.get<bool>("mqttRetain");
    publishOnMqttQueue((getConfigJson().get<String>("homeAssistantAutoDiscoveryPrefix")+"/"+_type+"/"+getConfigJson().get<String>("nodeId")+"/"+_class+"_"+String(_id)+"/config"),("{\"name\": \""+_name+"\", \"state_topic\": \""+_mqttState+"\",\"availability_topic\": \""+getAvailableTopic()+"\", \"command_topic\": \""+_mqttCommand+"\", \"retain\": "+String(_retain)+",\"payload_available\":\"1\",\"payload_not_available\":\"0\"}"),true);
    subscribeOnMqtt(_mqttCommand.c_str());
   }
}

void createHASensorComponent(){
  JsonArray& _devices = getStoredSensors();
  for(int i  = 0 ; i < _devices.size() ; i++){ 
    JsonObject& d = _devices[i];      
    int _id = d.get<unsigned int>("id");
    String  _type = d.get<String>("type");
    String _class =d.get<String>("class");
    String _name =d.get<String>("name");
    JsonArray& functions = d.get<JsonVariant>("functions");
     for(int i  = 0 ; i < functions.size() ; i++){
        JsonObject& f = functions[i]; 
        String _fname =f.get<String>("name");
        String _unit =f.get<String>("unit");
        String _mqttState =f.get<String>("mqttStateTopic");
        bool _retain =f.get<bool>("mqttRetain");   
        publishOnMqttQueue((getConfigJson().get<String>("homeAssistantAutoDiscoveryPrefix")+"/"+_class+"/"+getConfigJson().get<String>("nodeId")+"/"+_class+"_"+_fname+"_"+String(_id)+"/config"),("{\"name\": \""+_fname+"\",\"unit_of_measurement\": \""+_unit+"\", \"state_topic\": \""+_mqttState+"\",\"availability_topic\": \""+getAvailableTopic()+"\",\"payload_available\":\"1\",\"payload_not_available\":\"0\"}"),true);
   } 
  }
}

void realoadHaConfig(){
  //TODO SEND EMPTY MESSAGE TO CLEAN OLD DEVICES
  createHALigthComponent();
  createHASensorComponent();
}
