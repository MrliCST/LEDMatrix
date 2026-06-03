#ifndef WEB_SERVER_H
#define WEB_SERVER_H
#include <WebServer.h>

class WebServerWrapper {
public:
    static WebServerWrapper& instance();

    void start();                // 注册路由 + server.begin()
    void handleClient();         // server.handleClient()

private:
    WebServerWrapper() = default;
    WebServer* _server = nullptr;

    void _onRoot();
    void _onConfigWifi();
    void _onNotFound();
    String _buildPage();         // 拼接 HTML 页面
    String _scanWiFiOptions();   // 扫描 WiFi 生成 <option> 列表
};

#endif
