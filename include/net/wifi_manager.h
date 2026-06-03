#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#include <WiFi.h>
#include "core/config.h"

class WifiManager {
public:
    static WifiManager& instance();

    // STA 模式
    bool connect(int timeoutSec);        // 返回是否连接成功
    void disconnect();
    bool isConnected() const { return _connected; }

    // AP 模式（配网）
    void startAP();
    IPAddress apIP() const;

private:
    WifiManager() = default;
    bool _connected = false;
};

#endif
