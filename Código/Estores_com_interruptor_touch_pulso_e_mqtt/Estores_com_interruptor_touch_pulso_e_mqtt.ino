#include <FS.h>  
//JSON
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson  
#include <Timing.h> //https://github.com/scargill/Timing
//MQTT
#include <PubSubClient.h>
//ESP
#include <ESP8266WiFi.h>
//Wi-Fi Manger library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>//https://github.com/tzapu/WiFiManager

#include <Bounce2.h> //https://github.com/thomasfredericks/Bounce-Arduino-Wiring
#define AP_TIMEOUT 60
#define SERIAL_BAUDRATE 115200

#define BLIND_OPEN_RELAY 04
#define BLIND_CLOSE_RELAY 05

#define BLIND_TOUCH 13

//CONSTANTS
const String HOSTNAME  = "OnOfreDual-"+String(ESP.getChipId());
const char * OTA_PASSWORD  = "otapower";
const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set";

const String MQTT_CONTROL_TOPIC = HOSTNAME+"/cover/set";
const String MQTT_STATE_TOPIC = HOSTNAME+"/cover/state";

//Configuração por defeito
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_username[34] = "";
char mqtt_password[34] = "";
int mqttRetry = 0;
WiFiManager wifiManager;
WiFiClient wclient;
PubSubClient client(mqtt_server,atoi(mqtt_port),wclient);  
Bounce debouncerPulse = Bounce();

//CONTROL FLAGS

String lastState = "CLOSE";
int buttonState;             
int lastButtonState = HIGH;   
long lastDebounceTime = 0;
//flag para guardar configuração
bool shouldSaveConfig = false;
int timePressed = 0;


Timing reconnectTimer;

void saveConfigCallback () {
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
}
void setup() {
  pinMode(BLIND_OPEN_RELAY,OUTPUT);
  pinMode(BLIND_CLOSE_RELAY,OUTPUT);
  digitalWrite(BLIND_OPEN_RELAY,LOW);
  digitalWrite(BLIND_CLOSE_RELAY,LOW);
  Serial.begin(SERIAL_BAUDRATE);
  reconnectTimer.begin(0);
  //Limpar configuração
  //formatFileSystem();
  //Montar sistema de ficheiros
  mountFileSystem();
  //Configurar Wi-Fi Manager
  setupWifiManager();
  client.setCallback(callback);
  pinMode(BLIND_TOUCH,INPUT);
  debouncerPulse.attach(BLIND_TOUCH);
  debouncerPulse.interval(5);//DELAY
  
  prepareWebserverUpdate();
 
}
void openBlinds(){
  digitalWrite(BLIND_CLOSE_RELAY,LOW);
  digitalWrite(BLIND_OPEN_RELAY,HIGH);
  lastState = "OPEN";
  Serial.println("OPEN METHOD");
   if (checkMqttConnection()){
    client.publish(MQTT_STATE_TOPIC.c_str(),"open");
  }
}
void closeBlinds(){
  digitalWrite(BLIND_OPEN_RELAY,LOW);
  digitalWrite(BLIND_CLOSE_RELAY,HIGH);
  lastState = "CLOSE";
  Serial.println("CLOSE METHOD");
   if (checkMqttConnection()){
    client.publish(MQTT_STATE_TOPIC.c_str(),"closed");
  }
}

void stopBlinds(){
  digitalWrite(BLIND_OPEN_RELAY,LOW);
  digitalWrite(BLIND_CLOSE_RELAY,LOW);
  Serial.println("STOP");
  if (checkMqttConnection()){
    client.publish(MQTT_STATE_TOPIC.c_str(),"50");
  }
}

//Chamada de recepção de mensagem 
void callback(char* topic, byte* payload, unsigned int length) {
  String payloadStr = "";
  for (int i=0; i<length; i++) {
    payloadStr += (char)payload[i];
  }
  Serial.print("MQTT: ");
  Serial.println(payloadStr);
  String topicStr = String(topic);
  if(topicStr.equals(MQTT_SYSTEM_CONTROL_TOPIC)){
    if(payloadStr.equals("OTA_ON_"+String(HOSTNAME))){
     turnOnArduinoOta();
    }else if (payloadStr.equals("OTA_OFF_"+String(HOSTNAME))){
     turnOffArduinoOta();
    }else if (payloadStr.equals("REBOOT_"+String(HOSTNAME))){
      ESP.restart();
    }
  }else if ( topicStr.equals(MQTT_CONTROL_TOPIC)){
  if(payloadStr.equals("OPEN")){
    openBlinds();
  }else if (payloadStr.equals("CLOSE")){
    closeBlinds();
  }else if (payloadStr.equals("STOP")){
    stopBlinds();
  }
  }
} 
  
bool checkMqttConnection(){
  if(mqttRetry < 3){
    mqttRetry ++;
  if (!client.connected()) {
     Serial.println("Try new MQTT connection");
    if (client.connect(HOSTNAME.c_str(),mqtt_username, mqtt_password)) {
      mqttRetry = 0;
      //SUBSCRIÇÃO DE TOPICOS
      Serial.println("CONNECTED ON MQTT");
      client.subscribe(MQTT_SYSTEM_CONTROL_TOPIC.c_str());
      client.subscribe(MQTT_CONTROL_TOPIC.c_str());
      //Envia uma mensagem por MQTT para o tópico de log a informar que está ligado
      client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" CONNECTED").c_str());
      client.publish(("homeassistant/cover/"+String(HOSTNAME)+"/config").c_str(),("{\"name\": \""+String(HOSTNAME)+"_COVER\", \"state_topic\": \""+MQTT_STATE_TOPIC+"\", \"command_topic\": \""+MQTT_CONTROL_TOPIC+"\"}").c_str());
    }
  }
  return client.connected();
  }
  return false;
}


void loop() {
    if (reconnectTimer.onTimeout(60000) && mqttRetry >= 3) {
      Serial.println("Reset retry");
      mqttRetry--;
    }
  debouncerPulse.update();
  bool realState = debouncerPulse.read();
  if (realState){
    //BOTAO FOI PRESSIONADO
    if(lastButtonState != realState){
       checkManualReset();
      lastButtonState = realState;
      if (lastState.equals("CLOSE")){
        openBlinds();
      }else if(lastState.equals("OPEN")){
        closeBlinds();
      }
    }
     
    timePressed++;
    delay(100);
    if(timePressed == 15){
      stopBlinds();
   
      
    }
  }else{
    lastButtonState = realState;
    timePressed = 0;
  }

if (WiFi.status() == WL_CONNECTED) {
    otaLoop();
    if (checkMqttConnection()){
      client.loop();    
    }
  }
}



