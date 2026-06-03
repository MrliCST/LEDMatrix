#include "net/wifi_manager.h"
#include "core/store.h"

WifiManager& WifiManager::instance() { static WifiManager wm; return wm; }

bool WifiManager::connect(int timeoutSec) {
    auto& wifi = Store::instance().wifi();
    WiFi.begin(wifi.ssid.c_str(), wifi.pass.c_str());

    int ticks = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if (++ticks > 2 * timeoutSec) {
            _connected = false;
            return false;
        }
    }
    _connected = true;
    return true;
}

void WifiManager::disconnect() { WiFi.disconnect(); _connected = false; }

void WifiManager::startAP() {
    WiFi.enableAP(true);
    IPAddress ip(AP_IP_O1, AP_IP_O2, AP_IP_O3, AP_IP_O4);
    IPAddress gw(AP_IP_O1, AP_IP_O2, AP_IP_O3, 254);
    IPAddress sn(255, 255, 255, 0);
    WiFi.softAPConfig(ip, gw, sn);
    WiFi.softAP(AP_SSID);
}

IPAddress WifiManager::apIP() const { return WiFi.softAPIP(); }
