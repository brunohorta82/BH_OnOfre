#include <Ticker.h>
Ticker pulseTicker;

void turnOn(String topic, int gpio, bool inverted) {
  pinMode(gpio,OUTPUT);
 if(inverted){
      digitalWrite( gpio,LOW);
    }else{
      digitalWrite( gpio,HIGH);
    }
    mqttSend(topic,true,PAYLOAD_ON);  
    logger("[RELAY GPIO: "+String(gpio)+"] ON");
}

void turnOff(String topic,int gpio, bool inverted) {
    pinMode(gpio,OUTPUT);
  if(inverted){
      digitalWrite( gpio,HIGH);
    }else{
      digitalWrite( gpio,LOW);
    }
    mqttSend(topic,true,PAYLOAD_OFF);  
    logger("[RELAY GPIO: "+String(gpio)+"] OFF");
}

void turnOnInverted(String topic, int gpio) {
  turnOn(topic,gpio,true);
  
}

void turnOffInverted(String topic, int gpio) {
  turnOff(topic, gpio,true);
}
void toogleNormal(String topic, int gpio){
  if(digitalRead(gpio)){
    turnOffNormal(topic,gpio);
   }else{
    turnOnNormal(topic,gpio);
   }
 }
void turnOnNormal(String topic, int gpio) {
  turnOn(topic, gpio,false);
}

void turnOffNormal(String topic,int gpio) {
  turnOff(topic, gpio,false);
}

// FIRST ON AFTER DELAY OFF
void pulseOn(String topic,int gpio, bool inverted, int delay) {
  if(inverted){
     turnOnInverted(topic,gpio);
     //pulseTicker.once(delay, turnOffInverted,gpio);
    }else{
       turnOnNormal(topic,gpio);
      // pulseTicker.once(delay, turnOffNormal,gpio);
    } 
}

// FIRST OFF AFTER DELAY ON
void pulseOff(String topic, int gpio, bool inverted, int delay) {
  if(inverted){
     turnOffInverted(topic,gpio);
     //pulseTicker.once(delay, turnOnInverted,gpio);
    }else{
       turnOffNormal(topic,gpio);
      // pulseTicker.once(delay, turnOnNormal,gpio);
    } 
}

void mqttqttRelayControl(String topic, String payload){
 if(payload.equals(PAYLOAD_ON)){
  for(int i = 0 ; i < NUMBER_OF_MQTT_RELAYS; i++){
    if(topic.equals(MQTT_RELAY_TOPIC(i,true))){
      turnOnNormal(MQTT_RELAY_TOPIC(i,false),MQTT_RELAY_MAP[i]);
      logger("[RELAY "+String(i+1)+"] ON");
      }
    }
  }else if (payload.equals(PAYLOAD_OFF)){
    for(int i = 0 ; i < NUMBER_OF_MQTT_RELAYS; i++){
    if(topic.equals(MQTT_RELAY_TOPIC(i,true))){
      turnOffNormal(MQTT_RELAY_TOPIC(i,false),MQTT_RELAY_MAP[i]);
   
      
      }
    }
  }
}
