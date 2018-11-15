

void createHASwitchsComponent(){
  JsonArray& _devices = getStoredSwitchs();
  for(int i  = 0 ; i < _devices.size() ; i++){ 
    JsonObject& switchJson = _devices[i];      
    String _id = switchJson.get<String>("id");
    String  _type = switchJson.get<String>("type");
    String _class =switchJson.get<String>("class");
    String _name =switchJson.get<String>("name");
    String _mqttCommand =switchJson.get<String>("mqttCommandTopic");
    String _mqttState =switchJson.get<String>("mqttStateTopic");
    bool _retain =switchJson.get<bool>("mqttRetain");
    String state = switchJson.get<bool>("stateControl") ? PAYLOAD_ON : PAYLOAD_OFF;
    publishOnMqttQueue((getConfigJson().get<String>("homeAssistantAutoDiscoveryPrefix")+"/"+_type+"/"+getConfigJson().get<String>("nodeId")+"/"+_id+"/config"),("{\"name\": \""+_name+"\", \""+(_type.equals("cover") ? "position_topic" : "state_topic")+"\": \""+_mqttState+"\",\"availability_topic\": \""+getAvailableTopic()+"\", \"command_topic\": \""+_mqttCommand+"\", \"retain\": false,\"state\":\""+state+"\",\"payload_available\":\"1\",\"payload_not_available\":\"0\"}"),true);
    subscribeOnMqtt(_mqttCommand.c_str());
    if(_type.equals("cover")){
       publishOnMqttQueue(switchJson.get<String>("mqttStateTopic").c_str(),String(switchJson.get<unsigned int>("positionControlCover")),true);
    }else{
      publishOnMqttQueue(switchJson.get<String>("mqttStateTopic").c_str(),switchJson.get<bool>("stateControl") ? PAYLOAD_ON : PAYLOAD_OFF,true);
      }
   }
}

void createHASensorComponent(){
  JsonArray& _devices = getStoredSensors();
  for(int i  = 0 ; i < _devices.size() ; i++){ 
    JsonObject& d = _devices[i];   
   if(d.get<bool>("disabled")){
    continue;
   }  
    String _id = d.get<String >("id");
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
        publishOnMqttQueue((getConfigJson().get<String>("homeAssistantAutoDiscoveryPrefix")+"/"+_class+"/"+getConfigJson().get<String>("nodeId")+"/"+_class+"_"+_fname+"_"+_id+"/config"),("{\"name\": \""+_fname+"\",\"unit_of_measurement\": \""+_unit+"\", \"state_topic\": \""+_mqttState+"\",\"availability_topic\": \""+getAvailableTopic()+"\",\"payload_available\":\"1\",\"payload_not_available\":\"0\"}"),true);
   } 
  }
}

void realoadHaConfig(){
  createHASwitchsComponent();
  createHASensorComponent();
}

void removeComponentHaConfig(String oldPrefix,String oldNodeId, String _type, String _class, String _id){
   publishOnMqtt((oldPrefix+"/"+_type+"/"+oldNodeId+"/"+_id+"/config"),"",true);
}

