#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#include <WiFi.h>
#include "core/config.h"

/**
 * 负责WIFI的连接，热点的开启
 */
class WifiManager {
public:
    static WifiManager& instance();

    // 连接wifi
    bool connect(int timeoutSec);   
    void disconnect();
    bool isConnected() const { return _connected; }

    // 热点
    void startAP();
    IPAddress apIP() const;

    // wokwi 虚拟网络
    void connectAndSaveWokwiWIFI();

private:
    WifiManager() = default;
    bool _connected = false;
};

#endif
