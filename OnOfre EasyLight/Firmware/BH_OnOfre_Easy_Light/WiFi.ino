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
void setupWiFi(){
  jw.setHostname(hostname.c_str());
  jw.subscribe(infoCallback);
  jw.enableAP(false);
  jw.enableAPFallback(true);
  jw.enableSTA(true);
  jw.cleanNetworks();
  jw.addNetwork(wifiSSID.c_str(), wifiSecret.c_str());
}
void loopWiFi(){
  jw.loop();
}
