#include "net/web_server.h"
#include "core/store.h"
#include "net/wifi_manager.h"
#include <WiFi.h>
#include <SPIFFS.h>

// 单例模式
WebServerWrapper& WebServerWrapper::instance() {
    static WebServerWrapper ws;
    return ws;
}

// 开启Web服务器
void WebServerWrapper::start() {
    // 挂载文件系统（用于读取 data/index.html）
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS 挂载失败");
    }

    _server = new WebServer(80);

    // 监听 GET 和 POST 请求
    _server->on("/", HTTP_GET, [this]() { _onRoot(); });
    _server->on("/configwifi", HTTP_POST, [this]() { _onConfigWifi(); });

    // 未匹配路由 → 重定向到根
    _server->onNotFound([this]() { _onRoot(); });

    // 启动服务器
    _server->begin();
}

// 轮询监听客户端请求
void WebServerWrapper::handleClient() {
    _server->handleClient();
}



// 处理根路由
void WebServerWrapper::_onRoot() {
    _server->send(200, "text/html", _buildPage());
}

// 处理配置WiFi路由
void WebServerWrapper::_onConfigWifi() {
    // 解析表单中的ssid和pass
    if (_server->hasArg("ssid")) {
        WifiConfig cfg = Store::instance().wifi();
        cfg.ssid = _server->arg("ssid");
        cfg.pass = _server->arg("pass");
        cfg.apConfig = false; // 退出配置状态
        Store::instance().saveWifi(cfg);
    }

    // 解析表单中的color
    if (_server->hasArg("color")) {
        String rgb = _server->arg("color");
        int r = rgb.substring(0, rgb.indexOf(',')).toInt();
        int g = rgb.substring(rgb.indexOf(',') + 1, rgb.lastIndexOf(',')).toInt();
        int b = rgb.substring(rgb.lastIndexOf(',') + 1).toInt();

        ColorConfig cfg = Store::instance().color();
        cfg.r = r; cfg.g = g; cfg.b = b;
        Store::instance().saveColor(cfg);
    }

    // 发送响应
    _server->send(200, "text/html",
        "<h2>配置成功！</h2><p>设备即将重启...</p>"
        "<script>setTimeout(function(){location.href='/';},2000);</script>"
    );

    delay(1000);
    ESP.restart();
}

// 处理未匹配路由
void WebServerWrapper::_onNotFound() {
    _onRoot();
}



// 扫描当前的WiFi网络，并返回选项列表
String WebServerWrapper::_scanWiFiOptions() {
    String opts;
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
        opts += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
    }
    return opts;
}

// 构建根页面html
String WebServerWrapper::_buildPage() {
    // 从 SPIFFS 读取配网页模板
    File f = SPIFFS.open("/index.html", "r");
    if (!f) {
        return "<h2>配网页加载失败</h2><p>请检查 SPIFFS 是否已上传 data/ 目录</p>";
    }

    String html = f.readString();
    f.close();

    // 替换占位符
    auto& wifi = Store::instance().wifi();
    auto& color = Store::instance().color();

    html.replace("{{WIFI_OPTIONS}}", _scanWiFiOptions());
    html.replace("{{PASS_VALUE}}", wifi.pass);
    html.replace("{{RGB_VALUES}}", String(color.r) + "," + String(color.g) + "," + String(color.b));

    return html;
}

