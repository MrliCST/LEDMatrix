#ifndef SETTING_PAGE_H
#define SETTING_PAGE_H
#include "pages/page_base.h"

/**
 * 配网页面 (SETTING)
 *
 * 启动后若 WiFi 未配置，进入此页面：
 *   - ESP32 开启 AP 热点 "EasyMatrix"
 *   - 手机连上热点后，浏览器访问 192.168.1.1 进行配网
 *   - 本页面的 update() 循环处理 Web 请求
 *
 * 按键行为：
 *   短按 按键1: 无
 *   短按 按键2: 无
 *   短按 按键3: 确认配网 — 停止显示 IP，根据是否已填 ssid 决定连 WiFi 还是跳转
 *   长按: 均无
 */
class SettingPage : public PageBase {
public:
    void enter(Page from) override;       // WiFi AP 和 WebServer 在 setup() 中已启动
    void update() override;               // 处理 WebServer 客户端请求 (WebServerWrapper::handleClient)
    void onBtn3Short() override;          // 确认配网 → 清屏 → 连 WiFi 或跳转
};

#endif
