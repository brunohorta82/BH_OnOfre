#ifdef BHPZEM
#include <Timing.h> //https://github.com/scargill/Timing
#include <PZEM004T.h> //https://github.com/olehs/PZEM004T
//TEMPERATURA
#include <OneWire.h>
#define DIRECTION_PIN 14
#define PZEM_READINDS_TOPIC  "/readings/status"
#define RX_PIN 4
#define TX_PIN 5 
#define DS18B20_PIN 12
#define MAX_ATTEMPTS 5
DeviceAddress sensores[8];
IPAddress pzemIP(192, 168, 1, 1);
PZEM004T pzem(RX_PIN, TX_PIN);
Timing timerRead;
//DALLAS
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress devAddr[15];  // array of (up to) 15 temperature sensors
String devAddrNames[15];  // array of (up to) 15 temperature sensors
int sensorsCount = 0;

float requestTemperature(DeviceAddress deviceAddress){
  float temp = 0;
   do {
    temp = sensors.getTempC( deviceAddress);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}



int directionSignal(){
  if(getConfigJson().get<bool>("directionCurrentDetection"))
    return digitalRead(DIRECTION_PIN) ? -1 : 1;
  return 1;
}

float getVoltage() {
  int i = 0;
  float r = -1.0;
  do {
    r = pzem.voltage(pzemIP);
    i++;
  } while ( i < MAX_ATTEMPTS && r < 0.0);
  return r;
}

float getCurrent() {
  int i = 0;
  float r = -1.0;
  do {
    r = pzem.current(pzemIP);
    i++;
  } while ( i < MAX_ATTEMPTS && r < 0.0);
  return r;
}

float getPower() {
  int i = 0;
  float r = -1.0;
  do {
    r = pzem.power(pzemIP);
    i++;
  } while ( i < MAX_ATTEMPTS && r < 0.0);
  return r;
}

float getEnergy() {
  int i = 0;
  float r = -1.0;
  do {
    r = pzem.energy(pzemIP);
    i++;
  } while ( i < MAX_ATTEMPTS && r < 0.0);
  return r;
}
void setupBHPzem() {
 
  //PZEM SETUP
  pzem.setAddress(pzemIP);
  delay(1000);// WAITING FOR PZEM CONECTION
  pinMode(DIRECTION_PIN,INPUT);
  sensors.begin();
  sensorsCount = sensors.getDeviceCount();
  oneWire.reset_search();
  for (int i=0; i<sensorsCount; i++){
   if (!oneWire.search(devAddr[i])){ 
    logger("Unable to find temperature sensors address");
   }
  }
  for (int a=0; a<sensorsCount; a++){ 
  String addr  = "";
    for (uint8_t i = 0; i < 8; i++){
      if (devAddr[0][i] < 16) addr+="0";
      addr+=String(devAddr[a][i], HEX);
   }
  devAddrNames[a] = addr;
  }
  timerRead.begin(0);
}
bool pzemError = false;
int pzemErrorAttemps = 0;

void loopBHPzem() {
      if (timerRead.onTimeout(getConfigJson().get<unsigned int>("notificationInterval")) ){
    /*    float v = pzemError ? -1 :  getVoltage();
        float i = pzemError ? -1 :   getCurrent();
        float p = pzemError ? -1 :   getPower()*directionSignal();
        float e = pzemError ? -1 :  getEnergy()/1000;
        sensors.requestTemperatures();
        String temperatures= "";
        String displayTemps = "";
        for(int a = 0 ;a < sensorsCount; a++){
          float t = requestTemperature(devAddr[a]);
          displayTemps += "t"+String(a+1)+": "+((int)t)+ " ºC ";
          temperatures += "\"temp_"+devAddrNames[a]+"\":"+String(t)+",";
        }
      if(v < 0 && i < 0 && p < 0 && e <= 0){
        pzemError = true;
        pzemErrorAttemps++;
        if(pzemErrorAttemps == 1){
          logger("[PZEM] Check connections, can't get data.");
        }
        if(pzemErrorAttemps > 5){
          logger("[PZEM] Retry get data...");
          pzemErrorAttemps = 0;
          pzemError = false;
        }
        
      }
      if(!pzemError){
        logger("[PZEM] V: "+String(v));
        logger("[PZEM] A: "+String(i));
        logger("[PZEM] W: "+String(p));
        logger("[PZEM] kWh: "+String(e));
      }
      
      String cachedReadings = "{"+ temperatures+"\"voltagem\":" + String(v) + ",\"amperagem\":" + String(i) + ",\"potencia\":" + String(p) + ",\"contador\":" + String(e)+",\"config\":" + String(FIRMWARE_VERSION) +"}";
      publishData(cachedReadings);
     printOnDisplay(v, i, p, e, displayTemps);
     */
    }
  
}
void publishData(String cachedReadings){
  //WEB PANEL
  publishOnEventSource("dashboard",cachedReadings);
  //MQTT
  publishOnMqtt(PZEM_READINDS_TOPIC,cachedReadings,false);
  //EMON CMS
  publishOnEmoncms(cachedReadings);
}

#endif
