#include <DebounceEvent.h> //https://github.com/xoseperez/debounceevent

DebounceEvent switchOne = DebounceEvent(SWITCH_ONE, callback, BUTTON_SWITCH | BUTTON_SET_PULLUP);
DebounceEvent switchTwo = DebounceEvent(SWITCH_TWO, callback, BUTTON_SWITCH | BUTTON_SET_PULLUP);

void callback(uint8_t pin, uint8_t event, uint8_t count, uint16_t length) {
  int index = pin  == SWITCH_ONE ? 0 : 1;
  toogleNormal(MQTT_RELAY_TOPIC(index,false),MQTT_RELAY_MAP[index]);
}



void loopSwitchs(){
   switchOne.loop();
   switchTwo.loop();
  }
