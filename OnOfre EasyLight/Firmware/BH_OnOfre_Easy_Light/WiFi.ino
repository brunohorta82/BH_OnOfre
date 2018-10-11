#include <JustWifi.h>
bool scan = false;
void activateScan(){
  scan = true;
}
bool needScan(){
  return scan;
}

 void stopScan(){
  scan = false;
 logger("[WIFI] WI-Fi Network's Scanner Stoped");
 }

void reloadWiFiConfig(){
       jw.disconnect(); 
       jw.setHostname(getHostname().c_str());
       jw.cleanNetworks();
       jw.setSoftAP(getHostname().c_str(),getConfigJson().get<String>("apSecret").c_str());
       if(getConfigJson().get<bool>("staticIp")){
        jw.addNetwork(getConfigJson().get<String>("wifiSSID").c_str(), getConfigJson().get<String>("wifiSecret").c_str(),getConfigJson().get<String>("wifiIp").c_str(),getConfigJson().get<String>("wifiGw").c_str(),getConfigJson().get<String>("wifiMask").c_str());
       }else{
        jw.addNetwork(getConfigJson().get<String>("wifiSSID").c_str(), getConfigJson().get<String>("wifiSecret").c_str());
      }
     // reloadWifi = false;
 }
 
void scanNewWifiNetworks(){
    unsigned char result = WiFi.scanNetworks();
    if (result == WIFI_SCAN_FAILED) {
       publishOnEventSource("wifi-networks","Scan Failed");
       logger("[WIFI] Scan Failed");
    } else if (result == 0) {
       publishOnEventSource("wifi-networks","No networks found");
       logger("[WIFI] No networks found");
    } else {
        for (int8_t i = 0; i < result; ++i) {
            String ssid_scan;
            int32_t rssi_scan;
            uint8_t sec_scan;
            uint8_t* BSSID_scan;
            int32_t chan_scan;
            bool hidden_scan;
            char buffer[128];
            WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan, hidden_scan);
            snprintf_P(buffer, sizeof(buffer),
                PSTR("BSSID: %02X:%02X:%02X:%02X:%02X:%02X SEC: %s RSSI: %3d CH: %2d SSID: %s"),
                BSSID_scan[1], BSSID_scan[2], BSSID_scan[3], BSSID_scan[4], BSSID_scan[5], BSSID_scan[6],
                (sec_scan != ENC_TYPE_NONE ? "YES" : "NO "),
                rssi_scan,
                chan_scan,
                (char *) ssid_scan.c_str()
            );
            String out = String(buffer);
          publishOnEventSource("wifi-networks",out);
          logger("[WIFI] "+out);
        }
    }
    WiFi.scanDelete();
    stopScan();
 }
 void dissableAP(){
  jw.enableAP(false);
  }
void setupWiFi(){
  jw.setHostname(getHostname().c_str());
  jw.subscribe(infoCallback);
  jw.setSoftAP(getHostname().c_str(),getConfigJson().get<String>("apSecret").c_str());

  jw.enableAP(false);
  jw.enableAPFallback(true);
  jw.enableSTA(true);
  jw.cleanNetworks();
    if(getConfigJson().get<bool>("staticIp")){
    jw.addNetwork(getConfigJson().get<String>("wifiSSID").c_str(), getConfigJson().get<String>("wifiSecret").c_str(),getConfigJson().get<String>("wifiIp").c_str(),getConfigJson().get<String>("wifiGw").c_str(),getConfigJson().get<String>("wifiMask").c_str(),getConfigJson().get<String>("wifiGw").c_str());
  }else{
    jw.addNetwork(getConfigJson().get<String>("wifiSSID").c_str(), getConfigJson().get<String>("wifiSecret").c_str());
  }
}


void loopWiFi(){
  jw.loop();
}
JsonObject& wifiJSONStatus(){
  DynamicJsonBuffer jsonBuffer(32);
  JsonObject& wifi = jsonBuffer.createObject();
  wifi.set("wifiSSID",getConfigJson().get<String>("wifiSSID"));
  wifi.set("status",jw.connected());
  wifi.set("signal",WiFi.RSSI());
  wifi.set("wifiIp",WiFi.localIP().toString());  
  wifi.set("wifiMask",WiFi.subnetMask().toString());  
  wifi.set("wifiGw",WiFi.gatewayIP().toString());  
  wifi.set("apOn",WiFi.getMode() & WIFI_AP);
  return wifi;
}


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
        updateNetworkConfig();
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
      publishOnEventSource("wifi-log",msg);
      logger(msg);
};

