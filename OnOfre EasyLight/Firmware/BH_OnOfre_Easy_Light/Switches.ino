#include <DebounceEvent.h> //https://github.com/xoseperez/debounceevent

DebounceEvent switchOne = DebounceEvent(SWITCH_ONE, callback, BUTTON_SWITCH | BUTTON_SET_PULLUP);
DebounceEvent switchTwo = DebounceEvent(SWITCH_TWO, callback, BUTTON_SWITCH | BUTTON_SET_PULLUP);

void callback(uint8_t pin, uint8_t event, uint8_t count, uint16_t length) {
  int index = pin  == SWITCH_ONE ? 0 : 1;
  toogleNormal(MQTT_RELAY_MAP[index]);
}

void webSwitchPublish(int gpio){
  Serial.println("PUSH");
  if(gpio == RELAY_ONE){
    publishOnEventSource("dashboard","{\"id\":\""+String(SWITCH_ONE)+"\",\"gpio_control\":\""+String(RELAY_ONE)+"\",\"type\":\"relay\",\"icon\":\"fa-lightbulb-o\",\"name\":\""+switchIO12Name+"\",\"state\":\""+relayState(RELAY_ONE)+"\"}");
  }else if (gpio == RELAY_TWO){
    publishOnEventSource("dashboard","{\"id\":\""+String(SWITCH_TWO)+"\",\"gpio_control\":\""+String(RELAY_TWO)+"\",\"type\":\"relay\",\"icon\":\"fa-lightbulb-o\",\"name\":\""+switchIO13Name+"\",\"state\":\""+relayState(RELAY_TWO)+"\"}");
  }
  
}

void loopSwitchs(){
   switchOne.loop();
   switchTwo.loop();
}
