 #ifdef BHPZEM
#include <WiFiClientSecure.h>

void publishOnEmoncms(JsonObject& json){
  String emoncmsUrl = getConfigJson().get<String>("emoncmsUrl");
  String emoncmsApiKey = getConfigJson().get<String>("emoncmsApiKey");
  String emoncmsPrefix = getConfigJson().get<String>("emoncmsPrefix");
  int emoncmsPort = getConfigJson().get<int>("emoncmsPort");
  if(WiFi.status() != WL_CONNECTED || emoncmsUrl.equals(NULL) || emoncmsApiKey.equals(NULL) ||  emoncmsUrl.equals("") || emoncmsApiKey.equals(""))return;

     String jsonStr = "";
     json.printTo(jsonStr);
     String url = emoncmsPrefix+ "/input/post?node="+getConfigJson().get<String>("nodeId")+"&apikey="+emoncmsApiKey+"&json="+jsonStr;
          if(emoncmsUrl.startsWith("https://")){
           emoncmsUrl.replace("https://","");
           WiFiClientSecure clienthttps;
        logger("[EMONCMS] Try HTTPS Connection...");
        if (!clienthttps.connect(emoncmsUrl,emoncmsPort)) {
          logger("[EMONCMS] Connection failed");
          return;
        }
      
        clienthttps.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + String(emoncmsUrl) + "\r\n" +
                   "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (clienthttps.available() == 0) {
          if (millis() - timeout > 5000) {
           logger("[EMONCMS] >>> Client Timeout !");
            clienthttps.stop();
            return;
          }
        }
        logger("[EMONCMS] HTTPS Data sent OK");
          }else{
         emoncmsUrl.replace("http://","");
         WiFiClient clienthttp;
            if (!clienthttp.connect(emoncmsUrl,emoncmsPort)) {
              logger("[EMONCMS] Connection failed");
              
              return;
            }
            clienthttp.print(String("GET ") + url + " HTTP/1.1\r\n" +
                       "Host: " + emoncmsUrl + "\r\n" +
                       "Connection: close\r\n\r\n");
            unsigned long timeout = millis();
            while (clienthttp.available() == 0) {
              if (millis() - timeout > 5000) {
                logger("[EMONCMS] >>> Client Timeout !");
                clienthttp.stop();
                return;
              }
            }
          logger("[EMONCMS] HTTP Data sent OK");
       }
}
#endif
