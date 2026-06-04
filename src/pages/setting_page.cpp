#include "pages/setting_page.h"
#include "hal/led_matrix.h"

void SettingPage::enter(Page from) {
    auto& m = LedMatrix::instance();
    m.clear();
    m.setCursor(0, 1);
    m.print("setting");
    m.show();
}

void SettingPage::update() {
}

void SettingPage::onBtn3Short() {
    // 停止显示 IP 的任务 → 清屏 → 根据是否有 ssid 决定连 WiFi 还是跳转
    // 同原 btn3click case SETTING 的逻辑
}
