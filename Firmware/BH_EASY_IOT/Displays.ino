#ifdef BHPZEM
#include <Bounce2.h> // https://github.com/thomasfredericks/Bounce2
#include <Wire.h>  
#include "SSD1306.h" //https://github.com/ThingPulse/esp8266-oled-ssd1306
#define DISPLAY_SDA 2 //-1 if you don't use display
#define DISPLAY_SCL 13 //-1 if you don't use display
#define DISPLAY_BTN 16

bool displayOn = true;
SSD1306 display(0x3c, DISPLAY_SDA,DISPLAY_SCL);
bool lastState = false;
Bounce debouncer = Bounce();
void setupDisplay(){
    pinMode(DISPLAY_BTN,INPUT_PULLDOWN_16);
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(5,0, "BH PZEM");
    display.display();
    
  debouncer.attach(DISPLAY_BTN);
  debouncer.interval(5); // interval in ms
}
void printOnDisplay(float _voltage, float _amperage, float _power, float _energy, String _temperatures){
  display.clear(); 
  display.setFont(ArialMT_Plain_16);
  display.drawString(5,0, String(_power)+"W");
  display.setFont(ArialMT_Plain_10);
  display.drawString(5,16, String(_energy)+" kWh");
  display.drawString(5,26, String(_voltage)+" V");
  display.drawString(5,36, String(_amperage)+" A");
  display.drawString(5,46, String(_temperatures));
  display.display();
}

void loopSwitchDisplay(){
  debouncer.update();
  int value = debouncer.read();
   if(lastState != value){
     lastState = value;
      if ( value) {
   
    
     if(displayOn){
      display.displayOff();
      displayOn  = false;
     }else{
      display.displayOn();
      displayOn  = true;
     }
    }
   }
  } 
  
  
#endif
