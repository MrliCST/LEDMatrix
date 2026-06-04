#include "pages/setting_page.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"
#include <net/web_server.h>

void SettingPage::enter(Page from) {
    Serial.println("Setting::enter");

    auto& m = LedMatrix::instance();
    m.clear();
    m.setCursor(0, 1);
    m.print("setting");
    m.show();
}

void SettingPage::update() {
    WebServerWrapper::instance().handleClient();  // 处理 web 请求
}

void SettingPage::onBtn3Short() {
    StateMachine::instance().gotoPage(Page::TIME);
}
