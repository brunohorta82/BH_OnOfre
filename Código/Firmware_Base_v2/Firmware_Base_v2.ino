/**
 * Desenvolvido por Bruno Horta
 * 
 * Todo o código é livre e pode ser utilizado ou alterado
 * 
 * Exemplo: https://www.youtube.com/watch?v=OyY4ymv6db0
 * */
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
//OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h> 
           
#define AP_TIMEOUT 180
#define SERIAL_BAUDRATE 115200

#define RELAY_ONE 5
#define RELAY_TWO 4

#define PAYLOAD_ON "ON"
#define PAYLOAD_OFF "OFF"

//CONSTANTS
const String HOSTNAME  = "OnOfreDual-1";
const char * OTA_PASSWORD  = "otapower";
const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set/"+HOSTNAME;
const String MQTT_LIGHT_ONE_TOPIC = "relay/one/set";
const String MQTT_LIGHT_TWO_TOPIC = "relay/two/set";
const String MQTT_LIGHT_ONE_STATE_TOPIC = "relay/one";
const String MQTT_LIGHT_TWO_STATE_TOPIC = "relay/two";


//CONTROL FLAGS
bool OTA = false;
bool OTABegin = false;
bool lastButtonState = false;
//Configuração por defeito
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_username[34] = "";
char mqtt_password[34] = "";

//flag para guardar configuração
bool shouldSaveConfig = false;

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

void setupWifiManager(){
   WiFiManager wifiManager;
 
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_username("username", "mqtt username", mqtt_username, 32);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 32);
  
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);

  if (!wifiManager.autoConnect("Config")) {
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
  //reset saved settings
  //wifiManager.resetSettings();
  /*define o tempo limite até o portal de configuração ficar novamente inátivo,
   útil para quando alteramos a password do AP*/
   
  client.setCallback(callback);
  pinMode(RELAY_ONE,OUTPUT);
  pinMode(RELAY_TWO,OUTPUT);
}

void turnOnOut1(){
  digitalWrite(RELAY_ONE,HIGH);
  client.publish(MQTT_LIGHT_ONE_STATE_TOPIC.c_str(),PAYLOAD_ON);

}

void turnOffOut1(){
   digitalWrite(RELAY_ONE,LOW);  
   client.publish(MQTT_LIGHT_ONE_STATE_TOPIC.c_str(),PAYLOAD_OFF);
}

void turnOnOut2(){
  digitalWrite(RELAY_TWO,HIGH);
  client.publish(MQTT_LIGHT_TWO_STATE_TOPIC.c_str(),PAYLOAD_ON);
}



void turnOffOut2(){
   digitalWrite(RELAY_TWO,LOW);  
   client.publish(MQTT_LIGHT_TWO_STATE_TOPIC.c_str(),PAYLOAD_OFF);

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
    OTA = true;
    OTABegin = true;
  }else if (payloadStr.equals("OTA_OFF")){
    OTA = true;
    OTABegin = true;
  }else if (payloadStr.equals("REBOOT")){
    ESP.restart();
  }
 } else if(topicStr.equals(MQTT_LIGHT_ONE_TOPIC)){
  Serial.println(payloadStr);
  if(payloadStr.equals(PAYLOAD_ON)){
      turnOnOut1();
    }else if(payloadStr.equals(PAYLOAD_OFF)) {
      turnOffOut1();
    }
  }else if(topicStr.equals(MQTT_LIGHT_TWO_TOPIC)){
  Serial.println(payloadStr);
  if(payloadStr.equals(PAYLOAD_ON)){
      turnOnOut2();
    }else if(payloadStr.equals(PAYLOAD_OFF)) {
      turnOffOut2();
    }

  }    
} 


//Verifica se a ligação está ativa, caso não este liga-se e subscreve aos tópicos de interesse
bool checkMqttConnection(){
  if (!client.connected()) {
    if (client.connect(HOSTNAME.c_str(),mqtt_username, mqtt_password)) {
      //SUBSCRIÇÃO DE TOPICOS
      Serial.println("CONNECTED ON MQTT");
      client.subscribe(MQTT_SYSTEM_CONTROL_TOPIC.c_str());
      client.subscribe(MQTT_LIGHT_ONE_TOPIC.c_str());
      client.subscribe(MQTT_LIGHT_TWO_TOPIC.c_str());
      //Envia uma mensagem por MQTT para o tópico de log a informar que está ligado
      client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" CONNECTED").c_str());
    }
  }
  return client.connected();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (checkMqttConnection()){
      client.loop();
      if(OTA){
        if(OTABegin){
          setupOTA();
          OTABegin= false;
        }
        ArduinoOTA.handle();
      }
    }
  }
}
//Setup do OTA para permitir updates de Firmware via Wi-Fi
void setupOTA(){
  if (WiFi.status() == WL_CONNECTED && checkMqttConnection()) {
    client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" OTA IS SETUP").c_str());
    ArduinoOTA.setHostname(HOSTNAME.c_str());
    ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
    ArduinoOTA.begin();
    client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" OTA IS READY").c_str());
  }  
}




