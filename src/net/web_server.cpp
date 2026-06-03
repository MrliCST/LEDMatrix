#include "net/web_server.h"
#include "core/store.h"
#include "net/wifi_manager.h"
#include <WiFi.h>

WebServerWrapper& WebServerWrapper::instance() { static WebServerWrapper ws; return ws; }

void WebServerWrapper::start() {
    _server = new WebServer(80);

    _server->on("/", HTTP_GET, [this]() { _onRoot(); });
    _server->on("/configwifi", HTTP_POST, [this]() { _onConfigWifi(); });
    _server->onNotFound([this]() { _onRoot(); });
    _server->begin();
}

void WebServerWrapper::handleClient() { _server->handleClient(); }

String WebServerWrapper::_scanWiFiOptions() {
    // 同原 scanWiFi()，拼接 <option> 字符串
    String opts;
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
        opts += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
    }
    return opts;
}

String WebServerWrapper::_buildPage() {
    // 从 data/index.html 读取模板，替换 {{WIFI_OPTIONS}} {{PASS_VALUE}} {{RGB_VALUES}}
    // 或者和原来一样用字符串拼接（临时方案）
    return "";
}

void WebServerWrapper::_onRoot() {
    _server->send(200, "text/html", _buildPage());
}

void WebServerWrapper::_onConfigWifi() {
    // 解析 ssid / pass / red / green / blue
    // 调 Store::saveWifi(...)
    // 调 Store::saveColor(...)
    // _server->send(200, "text/html", "配置成功，即将重启...");
    // ESP.restart();
}

void WebServerWrapper::_onNotFound() {
    _onRoot();
}
