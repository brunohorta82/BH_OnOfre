/**
 * Desenvolvido por Bruno Horta
 * 
 * Todo o código é livre e pode ser utilizado ou alterado
 * 
 * Exemplo: https://www.youtube.com/watch?v=OyY4ymv6db0
 * */

#include <Bounce2.h>// https://github.com/thomasfredericks/Bounce2
#include <FS.h>  
//JSON
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson  
//MQTT
#include <PubSubClient.h>//https://www.youtube.com/watch?v=GMMH6qT8_f4  
//ESP
#include <ESP8266WiFi.h>'
//Wi-Fi Manger library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>//https://github.com/tzapu/WiFiManager

           
#define AP_TIMEOUT 180
#define SERIAL_BAUDRATE 115200

#define RELAY_ONE 5
#define RELAY_TWO 4

#define SWITCH_ONE 12
#define SWITCH_TWO 13

#define PAYLOAD_ON "ON"
#define PAYLOAD_OFF "OFF"

//CONSTANTS
const String HOSTNAME  = "OnOfreDual-"+String(ESP.getChipId());
const char * OTA_PASSWORD  = "otapower";
const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set/"+HOSTNAME;
const String MQTT_LIGHT_ONE_TOPIC = HOSTNAME+"/relay/one/set";
const String MQTT_LIGHT_TWO_TOPIC = HOSTNAME+"/relay/two/set";
const String MQTT_LIGHT_ONE_STATE_TOPIC = HOSTNAME+"/relay/one/state";
const String MQTT_LIGHT_TWO_STATE_TOPIC = HOSTNAME+"/relay/two/state";


//CONTROL FLAGS
bool lastButtonState = false;
//Configuração por defeito
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_username[34] = "";
char mqtt_password[34] = "";
bool lastButtonOneState = false;
bool lastButtonTwoState = false;
//flag para guardar configuração
bool shouldSaveConfig = false;
WiFiManager wifiManager;
//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void mountFileSystem(){
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_username, json["mqtt_username"]);
          strcpy(mqtt_password, json["mqtt_password"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}

void formatFileSystem(){
  SPIFFS.format(); 
  }
//Init webserver
int timesPress = 0;
long lastPressedMillis = 0;
bool checkManualReset(){
  if(timesPress == 0){
    lastPressedMillis = millis();
  }
  if(lastPressedMillis + 3000 > millis()){
    timesPress++;
    Serial.println("TIME ++");
  }else{
    timesPress = 0;
    Serial.println("TIME ZERO");
   }
    if(timesPress > 5){
      Serial.println("RESET");
      wifiManager.resetSettings();
      delay(1000);
     WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while(1)wdt_reset();
    }
  

}


void setupWifiManager(){
   
   //reset saved settings
 //wifiManager.resetSettings();
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_username("username", "mqtt username", mqtt_username, 32);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 32);
  
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);

  if (!wifiManager.autoConnect(HOSTNAME.c_str())) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //Reset para tentar novamente
    ESP.restart();
    delay(5000);
  }

  //Leitura dos valures guardados
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_username, custom_mqtt_username.getValue());
  strcpy(mqtt_password, custom_mqtt_password.getValue());
 //Guardar Configuração se necessario
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_password"] = mqtt_password;
    json["mqtt_username"] = mqtt_username;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}

Bounce debouncerSwOne = Bounce(); 
Bounce debouncerSwTwo = Bounce(); 
WiFiClient wclient;
PubSubClient client(mqtt_server,atoi(mqtt_port),wclient);  

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  //Limpar configuração
  //formatFileSystem();
  
  //Montar sistema de ficheiros
  mountFileSystem();
  //Configurar Wi-Fi Manager
  setupWifiManager();

  /*define o tempo limite até o portal de configuração ficar novamente inátivo,
   útil para quando alteramos a password do AP*/
   
  client.setCallback(callback);
  pinMode(RELAY_ONE,OUTPUT);
  pinMode(RELAY_TWO,OUTPUT);
  pinMode(SWITCH_ONE,INPUT_PULLUP);
  pinMode(SWITCH_TWO,INPUT_PULLUP);
  debouncerSwOne.attach(SWITCH_ONE);
  debouncerSwOne.interval(5); // interval in ms
  debouncerSwTwo.attach(SWITCH_TWO);
  debouncerSwTwo.interval(5); // interval in ms
  
   // Setup the second button with an internal pull-up :
  pinMode(SWITCH_TWO,INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncerSwTwo.attach(SWITCH_TWO);
 debouncerSwTwo.interval(5); // interval in ms
 prepareWebserverUpdate();
}

void turnOnOutOne(){
  digitalWrite(RELAY_ONE,HIGH);
  if (checkMqttConnection()){
    client.publish(MQTT_LIGHT_ONE_STATE_TOPIC.c_str(),PAYLOAD_ON);
  }

}

void turnOffOutOne(){
   digitalWrite(RELAY_ONE,LOW);
   if (checkMqttConnection()){  
    client.publish(MQTT_LIGHT_ONE_STATE_TOPIC.c_str(),PAYLOAD_OFF);
   }
}

void turnOnOutTwo(){
  digitalWrite(RELAY_TWO,HIGH);
  if (checkMqttConnection()){
    client.publish(MQTT_LIGHT_TWO_STATE_TOPIC.c_str(),PAYLOAD_ON);
  }
}



void turnOffOutTwo(){
   digitalWrite(RELAY_TWO,LOW);  
   if (checkMqttConnection()){
    client.publish(MQTT_LIGHT_TWO_STATE_TOPIC.c_str(),PAYLOAD_OFF);
   }

}
//Chamada de recepção de mensagem 
void callback(char* topic, byte* payload, unsigned int length) {
  String payloadStr = "";
  for (int i=0; i<length; i++) {
    payloadStr += (char)payload[i];
  }
  Serial.println(payloadStr);
  String topicStr = String(topic);
 if(topicStr.equals(MQTT_SYSTEM_CONTROL_TOPIC)){
  if(payloadStr.equals("OTA_ON")){
   turnOnArduinoOta();
  }else if (payloadStr.equals("OTA_OFF")){
    turnOffArduinoOta();
  }else if (payloadStr.equals("REBOOT")){
    ESP.restart();
  }
 } else if(topicStr.equals(MQTT_LIGHT_ONE_TOPIC)){
  Serial.println(payloadStr);
  if(payloadStr.equals(PAYLOAD_ON)){
      pulseSwitchOne();
    }
  }else if(topicStr.equals(MQTT_LIGHT_TWO_TOPIC)){
  Serial.println(payloadStr);
  if(payloadStr.equals(PAYLOAD_ON)){
      pulseSwitchTwo();
    }
  }    
} 


//Verifica se a ligação está ativa, caso não este liga-se e subscreve aos tópicos de interesse
bool checkMqttConnection(){
  if (!client.connected()) {
     Serial.print("TRY CONNECT TO MQTT ");
       Serial.println(mqtt_server);
    if (client.connect(HOSTNAME.c_str(),mqtt_username, mqtt_password)) {
      //SUBSCRIÇÃO DE TOPICOS
      Serial.print("CONNECTED ON MQTT ");
       Serial.println(mqtt_server);
      client.subscribe(MQTT_SYSTEM_CONTROL_TOPIC.c_str());
      client.subscribe(MQTT_LIGHT_ONE_TOPIC.c_str());
      client.subscribe(MQTT_LIGHT_TWO_TOPIC.c_str());
      //Envia uma mensagem por MQTT para o tópico de log a informar que está ligado
      client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" CONNECTED").c_str());
      client.publish(("homeassistant/pulse/"+String(HOSTNAME)+"_1/config").c_str(),("{\"name\": \""+String(HOSTNAME)+"_ONE\", \"state_topic\": \""+MQTT_LIGHT_ONE_STATE_TOPIC+"\", \"command_topic\": \""+MQTT_LIGHT_ONE_TOPIC+"\"}").c_str());
      client.publish(("homeassistant/pulse/"+String(HOSTNAME)+"_2/config").c_str(),("{\"name\": \""+String(HOSTNAME)+"_TWO\", \"state_topic\": \""+MQTT_LIGHT_TWO_STATE_TOPIC+"\", \"command_topic\": \""+MQTT_LIGHT_TWO_TOPIC+"\"}").c_str());
    }
  }
  return client.connected();
}
//Inverte o Estado do RELAY ON (ex: se ele estiver ligado então desliga e vice versa)
void pulseSwitchOne() {
   turnOnOutOne();
   delay(500);// 0.5 segundos
   turnOffOutOne();
}

//Inverte o Estado do RELAY TWO (ex: se ele estiver ligado então desliga e vice versa)
void pulseSwitchTwo() {
   turnOnOutTwo();
   delay(500);// 0.5 segundos
   turnOffOutTwo();
}
void loop() {
  debouncerSwOne.update();
  debouncerSwTwo.update();
  bool realOneState = debouncerSwOne.read();
  if(lastButtonOneState != realOneState ){
    checkManualReset();
    lastButtonOneState = realOneState;
      pulseSwitchOne();
  }
  bool realTwoState = debouncerSwTwo.read();
  if(lastButtonTwoState != realTwoState  ){
    checkManualReset();
    lastButtonTwoState = realTwoState;
        pulseSwitchTwo();
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    otaLoop();
    if (checkMqttConnection()){
      client.loop();
     
    }
  }
}




