#include "pages/setting_page.h"
#include "net/web_server.h"

void SettingPage::enter(Page from) {
    // 什么都不做，WiFi AP 和 WebServer 已在 setup 里启动
}

void SettingPage::update() {
    WebServerWrapper::instance().handleClient();
}

void SettingPage::onBtn3Short() {
    // 停止显示 IP 的任务 → 清屏 → 根据是否有 ssid 决定连 WiFi 还是跳转
    // 同原 btn3click case SETTING 的逻辑
}
