#ifndef WEB_SERVER_H
#define WEB_SERVER_H
#include <WebServer.h>

/**
 * WebServerWrapper: 负责启动 Web 服务器，并处理 HTTP 请求
*/
class WebServerWrapper {
public:
    static WebServerWrapper& instance();

    void start();         // 注册路由并启动服务器
    void handleClient();  // 轮询监听http请求

private:
    WebServerWrapper() = default;
    WebServer* _server = nullptr;

    void _onRoot();        // 根页面
    void _onConfigWifi();  // 配置页面
    void _onNotFound();    // 找不到页面

    String _buildPage();         // 拼接 HTML 页面
    String _scanWiFiOptions();   // 扫描 WiFi 生成 <option> 列表
};

#endif
