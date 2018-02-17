  /**
 * Desenvolvido por Bruno Horta
 * 
 * Todo o código é livre e pode ser utilizado ou alterado
 * 
 * Exemplo: https://www.youtube.com/watch?v=OyY4ymv6db0
 * */
//MQTT
#include <PubSubClient.h>
//ESP
#include <ESP8266WiFi.h>'
#include <Bounce2.h>
//OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>            


#define RECONNECT_TIMEOUT 30000
#define SERIAL_BAUDRATE 115200

#define MQTT_AUTH true
#define MQTT_USERNAME "homeassistant"
#define MQTT_PASSWORD "moscasMoscas82"
#define PAYLOAD_ON "ON"
#define PAYLOAD_OFF "OFF"
//GPIOS
#define RELAY_ONE 4
#define RELAY_TWO 5
#define SWITCH_ONE 12
#define SWITCH_TWO 13
#define SWITCH_THREE 14

//CONSTANTS
const String HOSTNAME  = "KitchenLights";
const char * OTA_PASSWORD  = "otapower";

const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set";

const String MQTT_LIGHT_ONE_TOPIC = "bhhome/Kitchen/table/light/set";
const String MQTT_LIGHT_TWO_TOPIC = "bhhome/Kitchen/bench/light/set";
const String MQTT_LIGHT_ONE_STATE_TOPIC = "bhhome/Kitchen/table/light/status";
const String MQTT_LIGHT_TWO_STATE_TOPIC = "bhhome/Kitchen/bench/light/status";
const String MQTT_LIGHT_THREE_STATE_TOPIC = "bhhome/livingroom/center/light/status";

//MQTT BROKERS GRATUITOS PARA TESTES https://github.com/mqtt/mqtt.github.io/wiki/public_brokers
const char* MQTT_SERVER = "192.168.187.203";

WiFiClient wclient;
PubSubClient client(MQTT_SERVER,1883,wclient);

Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce(); 
Bounce debouncer3 = Bounce(); 
//CONTROL FLAGS
bool OTA = false;
bool OTABegin = false;
bool lastButtonOneState = false;
bool lastButtonTwoState = false;
bool lastButtonThreeState = false;
bool timeout = true;
void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  //WiFi.disconnect(true);
  long init = millis();
  // Aguarda até estar ligado
  WiFi.mode(WIFI_STA);
  Serial.println("A Aguardar Configuração");
  if(WiFi.SSID() != NULL){
    WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str());
    long start = millis();
    while(millis() - start < RECONNECT_TIMEOUT){
       Serial.println("#");
      if(WiFi.status() == WL_CONNECTED){
        timeout = false;
        break;
      }
       delay(500);
    }   
  }
  if(timeout){
    Serial.println("Smart Config");
    timeout  = true;
    WiFi.beginSmartConfig();
    long start = millis();
    while(millis() - start < RECONNECT_TIMEOUT){
      Serial.println(".");
      if(WiFi.status() == WL_CONNECTED){
        Serial.println("DONE");
        timeout = false;
        break;
      }
        delay(500);
    }
  }
  if(timeout){
    Serial.println("RESET");
    while(1)ESP.restart();
    delay(500);
  }
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Tempo a ligar ");
  Serial.print(millis() - init  );
  Serial.println(" milisegundos");

  //Registo do callback MQTT 
  client.setCallback(callback);

  //Configuração dos GPIOS
  pinMode(RELAY_ONE,OUTPUT);
  pinMode(RELAY_TWO,OUTPUT);
  pinMode(SWITCH_ONE,INPUT_PULLUP);
  pinMode(SWITCH_TWO,INPUT_PULLUP);
  pinMode(SWITCH_THREE,INPUT_PULLUP);
  
  debouncer1.attach(SWITCH_ONE);
  debouncer1.interval(5); // intervalo em ms
  
  debouncer2.attach(SWITCH_TWO);
  debouncer2.interval(5); // intervalo em ms
  
  debouncer3.attach(SWITCH_THREE);
  debouncer3.interval(5); // intervalo em ms 
}


//Chamada de recepção de mensagens MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String payloadStr = "";
  for (int i=0; i<length; i++) {
    payloadStr += (char)payload[i];
  }
  Serial.println(payloadStr);
  String topicStr = String(topic);
  if(topicStr.equals(MQTT_SYSTEM_CONTROL_TOPIC)){
    if(payloadStr.equals("OTA_ON_"+String(HOSTNAME))){
      OTA = true;
      OTABegin = true;
    }else if (payloadStr.equals("OTA_OFF_"+String(HOSTNAME))){
      OTA = true;
      OTABegin = true;
    }else if (payloadStr.equals("REBOOT_"+String(HOSTNAME))){
      ESP.restart();
    }
  } else if(topicStr.equals(MQTT_LIGHT_ONE_TOPIC)){
    if(payloadStr.equals(PAYLOAD_ON)){
      turnOnOne();
    }else if(payloadStr.equals(PAYLOAD_OFF)) {
      turnOffOne();
    }
  }else if(topicStr.equals(MQTT_LIGHT_TWO_TOPIC)){
    if(payloadStr.equals(PAYLOAD_ON)){
      turnOnTwo();
    }else if(payloadStr.equals(PAYLOAD_OFF)) {
      turnOffTwo();
    }
  }
}

//LIGA O RELAY ONE e envia o estado ON por MQTT
void turnOnOne(){
  digitalWrite(RELAY_ONE,LOW);
  if (checkMqttConnection()){
    client.publish(MQTT_LIGHT_ONE_STATE_TOPIC.c_str(),PAYLOAD_ON,true);
  }
}

//DESLIGA O RELAY ONE e envia o estado OFF por MQTT
void turnOffOne(){
  digitalWrite(RELAY_ONE,HIGH);
  if (checkMqttConnection()){  
    client.publish(MQTT_LIGHT_ONE_STATE_TOPIC.c_str(),PAYLOAD_OFF,true);
  }
}

//LIGA O RELAY TWO e envia o estado ON por MQTT
void turnOnTwo(){
  digitalWrite(RELAY_TWO,LOW);
  if (checkMqttConnection()){
    client.publish(MQTT_LIGHT_TWO_STATE_TOPIC.c_str(),PAYLOAD_ON,true);
  }
}

//DESLIGA O RELAY TWO e envia o estado OFF por MQTT
void turnOffTwo(){
  digitalWrite(RELAY_TWO,HIGH);
  if (checkMqttConnection()){  
    client.publish(MQTT_LIGHT_TWO_STATE_TOPIC.c_str(),PAYLOAD_OFF,true);
  }
}

//Inverte o Estado do RELAY ON (ex: se ele estiver ligado então desliga e vice versa)
void toggleSwitchOne() {
  if(digitalRead(RELAY_ONE)){
   turnOnOne();
  }else{
   turnOffOne();
  }  
}

//Inverte o Estado do RELAY TWO (ex: se ele estiver ligado então desliga e vice versa)
void toggleSwitchTwo() {
  if(digitalRead(RELAY_TWO)){
   turnOnTwo();
  }else{
   turnOffTwo();
  }  
}



//Verifica se a ligação está ativa, caso não este liga-se e subscreve aos tópicos de interesse
bool checkMqttConnection(){
  if (!client.connected()) {
    if (MQTT_AUTH ? client.connect(HOSTNAME.c_str(),MQTT_USERNAME, MQTT_PASSWORD) : client.connect(HOSTNAME.c_str())) {
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
  //Atualização do estado dos botões com tratamento de bounce
  debouncer1.update();
  debouncer2.update();
  debouncer3.update();
  bool realOneState = debouncer1.read();
  if(lastButtonOneState != realOneState){
    lastButtonOneState = realOneState;
    toggleSwitchOne();
  }
  bool realTwoState = debouncer2.read();
  if(lastButtonTwoState != realTwoState){
    lastButtonTwoState = realTwoState;
    toggleSwitchTwo();
  }
  bool realThreeState = debouncer3.read();
  if(lastButtonThreeState != realThreeState){
    lastButtonThreeState = realThreeState;
    client.publish(MQTT_LIGHT_THREE_STATE_TOPIC.c_str(), realThreeState ? PAYLOAD_OFF : PAYLOAD_ON,true);
  }
  
  //Bloco que verifica se está ligado ao Wi-Fi
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



