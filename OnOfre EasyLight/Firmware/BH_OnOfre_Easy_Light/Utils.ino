#include <JustWifi.h>
void infoWifi() {

    if (WiFi.isConnected()) {
        uint8_t * bssid = WiFi.BSSID();
        logger("[WIFI] MODE STA -------------------------------------");
        logger("[WIFI] SSID  "+String(WiFi.SSID().c_str()));
        logger("[WIFI] BSSID "+String(bssid[0])+":"+String(bssid[1])+":"+String(bssid[2])+":"+String(bssid[3])+":"+String(bssid[4])+":"+String(bssid[5]));
        logger("[WIFI] CH  "+ String(WiFi.channel()));
        logger("[WIFI] RSSI  "+String(WiFi.RSSI()));
        logger("[WIFI] IP   "+WiFi.localIP().toString());
        logger("[WIFI] MAC   "+String( WiFi.macAddress().c_str()));
        logger("[WIFI] GW    "+WiFi.gatewayIP().toString());
        logger("[WIFI] MASK  "+WiFi.subnetMask().toString());
        logger("[WIFI] DNS   "+WiFi.dnsIP().toString());
        logger("[WIFI] HOST  "+String( WiFi.hostname().c_str()));
        logger("[WIFI] ----------------------------------------------");
    }

    if (WiFi.getMode() & WIFI_AP) {
        logger("[WIFI] MODE AP --------------------------------------");
        logger("[WIFI] SSID  "+String( jw.getAPSSID().c_str()));
        logger("[WIFI] IP    "+String( WiFi.softAPIP().toString().c_str()));
        logger("[WIFI] MAC   "+String( WiFi.softAPmacAddress().c_str()));
        logger("[WIFI] ----------------------------------------------");

    }

}

void infoCallback(justwifi_messages_t code, char * parameter) {
    String msg = "";
    switch (code){
      case MESSAGE_TURNING_OFF:
      msg = "[WIFI] Turning OFF";
      break;
      case MESSAGE_TURNING_ON:
      msg = "[WIFI] Turning ON";
      break;
      case MESSAGE_SCANNING:
      msg = "[WIFI] Scanning";
      break;
      case MESSAGE_SCAN_FAILED:
      msg = "[WIFI] Scan failed";
      break;
      case MESSAGE_NO_NETWORKS:
      msg = "[WIFI] No networks found";
      break;
      case MESSAGE_NO_KNOWN_NETWORKS:
      msg = "[WIFI] No known networks found";
      break;
      case MESSAGE_FOUND_NETWORK:
      msg = "[WIFI] "+String(parameter);
      break;
      case MESSAGE_CONNECTING:
       msg = String("[WIFI] Connecting to ")+String(parameter);
      break;
      case MESSAGE_CONNECT_WAITING:
        // too much noise
      break;
      case MESSAGE_CONNECT_FAILED:
       msg = "[WIFI] Could not connect to "+String(parameter);
      break;
      case MESSAGE_CONNECTED:
        infoWifi();
        setupMQTT();
      break;
      case MESSAGE_DISCONNECTED:
       msg = "[WIFI] Disconnected";
      break;
      case MESSAGE_ACCESSPOINT_CREATED:
       infoWifi();
      break;
      case MESSAGE_ACCESSPOINT_DESTROYED:
       msg = "[WIFI] Disconnecting access point";
      break;
      case MESSAGE_ACCESSPOINT_CREATING:
       msg = "[WIFI] Creating access point";
      break;
      case MESSAGE_ACCESSPOINT_FAILED:
       msg = "[WIFI] Could not create access point";
      break;
      case MESSAGE_WPS_START:
       msg = "[WIFI] WPS started";
      break;
      case MESSAGE_WPS_SUCCESS:
       msg = "[WIFI] WPS succeded!";
      break;

      case MESSAGE_WPS_ERROR:
       msg = "[WIFI] WPS failed";
      break;
      case MESSAGE_SMARTCONFIG_START:
       msg = "[WIFI] Smart Config started";
      break;
      case MESSAGE_SMARTCONFIG_SUCCESS:
       msg = "[WIFI] Smart Config succeded!";
      break;
      case MESSAGE_SMARTCONFIG_ERROR:
       msg = "[WIFI] Smart Config failed";
      break;
      
      }
        logger(msg);
};

String wifiJSONStatus(){
    return ("{\"wifiSSID\":\""+wifiSSID+"\",\"status\":"+String(jw.connected())+",\"signal\":\""+String(WiFi.RSSI())+"\"}");
}

String split(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

