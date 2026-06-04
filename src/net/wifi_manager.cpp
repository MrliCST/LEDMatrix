#include "net/wifi_manager.h"
#include "core/store.h"
#include <esp_task_wdt.h>

// 获取实例对象
WifiManager& WifiManager::instance() { 
    static WifiManager wm; 
    return wm; 
}

// 连接wifi
bool WifiManager::connect(int timeoutSec) {
    auto& wifi = Store::instance().wifi();
    esp_task_wdt_init(30, true); // WiFi.begin() 内部同步阻塞不喂狗，拉长超时
    WiFi.begin(wifi.ssid.c_str(), wifi.pass.c_str());
    esp_task_wdt_init(5, true);  // 恢复默认

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

// 连接并保存wokwi仿真器的虚拟wifi
void WifiManager::connectAndSaveWokwiWIFI(){
    Serial.println("[Wokwi] 连接虚拟 WiFi...");
    
    // 仿真环境缺陷，延长喂狗间隔时间
    esp_task_wdt_init(30, true);
    WiFi.begin("Wokwi-GUEST", "");  // 连接wifi
    esp_task_wdt_init(5, true);

    // 等待连接成功
    int ticks = 0;
    while (WiFi.status() != WL_CONNECTED && ticks < 60) {
        delay(500);
        ticks++;
        Serial.print(".");
    }
    Serial.println();

    // 判定是否连接成功
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Wokwi] WiFi 连接失败！");
        return;
    }

    // 连接成功
    Serial.print("[Wokwi] 已连接, IP: ");
    Serial.println(WiFi.localIP());

    // 不再开启web服务器，因为这个 Wokwi-GUEST 是虚拟仿真器唯一的!
    // 不需要在web进行配置，也没有其他WIFI供选择。

    // 退出配网模式, 保存wifi信息
    auto wifi = Store::instance().wifi();
    wifi.apConfig = false;   
    wifi.ssid = "Wokwi-GUEST";
    wifi.pass = "";
    Store::instance().saveWifi(wifi);

    Serial.println("[Wokwi] WiFi 已配置，继续启动...");
    WiFi.disconnect(true);  // 断开虚拟 WiFi，后续用正常流程重连
    delay(100);             // 等待射频关闭 + NVS 落盘
}
