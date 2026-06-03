#include "net/wifi_manager.h"
#include "core/store.h"

// 获取实例对象
WifiManager& WifiManager::instance() { 
    static WifiManager wm; 
    return wm; 
}

// 连接wifi
bool WifiManager::connect(int timeoutSec) {
    auto& wifi = Store::instance().wifi();
    WiFi.begin(wifi.ssid.c_str(), wifi.pass.c_str());

    // 最多等待30S
    int ticks = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if (++ticks > 2 * timeoutSec) {
            _connected = false;
            return false;
        }
    }

    // 连接成功
    _connected = true;
    return true;
}

// 断开wifi
void WifiManager::disconnect() { 
    WiFi.disconnect(); 
    _connected = false; 
}

// 开启热点
void WifiManager::startAP() {
    WiFi.enableAP(true); // 开启热点

    // ip，网关，子网掩码
    IPAddress ip(AP_IP_O1, AP_IP_O2, AP_IP_O3, AP_IP_O4);  
    IPAddress gw(AP_IP_O1, AP_IP_O2, AP_IP_O3, 254);  
    IPAddress sn(255, 255, 255, 0);
    WiFi.softAPConfig(ip, gw, sn); 

    WiFi.softAP(AP_SSID); // 热点名称    
}

// 获取本机的ip地址 
IPAddress WifiManager::apIP() const { 
    return WiFi.softAPIP(); 
}
