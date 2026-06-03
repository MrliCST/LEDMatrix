#include <Arduino.h>
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"
#include "hal/microphone.h"
#include "hal/buzzer.h"
#include "hal/buttons.h"
#include "net/wifi_manager.h"
#include "net/ntp.h"
#include "net/web_server.h"

static void startConfigMode();
static void startNormalMode();

void setup()
{
    // 初始化串口
    Serial.begin(115200);
    // 全局存储管理器
    Store::instance().load();
    // HAL 层初始化
    LedMatrix::instance().begin();
    Microphone::instance().begin();
    Buzzer::instance().begin();
    Buttons::instance().begin();
    // 状态机
    StateMachine::instance().init();
    // 配网模式
    if (Store::instance().wifi().apConfig)
    {
        startConfigMode();
        return;
    }
    // 正常模式
    startNormalMode();
}

void loop()
{
    // 基础物理层：按键轮询心跳，判断单击、双击、长按事件
    Buttons::instance().tick();

    // 定期网络层：动态对时回归
    if (StateMachine::instance().isCheckingTime())
    {
        bool ok = WifiManager::instance().connect(30);
        if (ok)
        {
            Ntp::instance().sync(30);
            WifiManager::instance().disconnect();
        }
        StateMachine::instance().setCheckingTime(false);
    }

    // 事件响应层：智能闹钟触发与自动复位
    if (Ntp::instance().isAlarmPending())
    {
        Ntp::instance().clearAlarmPending();
        Page cur = StateMachine::instance().currentPage();
        if (cur != Page::SETTING && cur != Page::CLOCK)
        {
            StateMachine::instance().gotoPage(Page::TIME);
            auto &clk = Store::instance().clock();
            Buzzer::instance().playSong(clk.bellIndex, true);
            Ntp::instance().setBelling(true);
            int32_t sec = Ntp::secondsUntilAlarm(clk.hour, clk.minute);
            Ntp::instance().startClockTicker(sec);
        }
    }

    StateMachine::instance().update();
    delay(10);
}

static void startConfigMode()
{
    // UI 状态机切页：SETTING
    StateMachine::instance().gotoPage(Page::SETTING);
    // Wi-Fi 工作模式从 WIFI_STA（无线终端，即接收端）切换为 WIFI_AP（软接入点，即发送端）。
    WifiManager::instance().startAP();
    // 启动Web服务器：80端口
    WebServerWrapper::instance().start();
}

static void startNormalMode()
{
    bool wifiOk = WifiManager::instance().connect(30);

    if (!wifiOk)
    {
        auto &m = LedMatrix::instance();
        m.clear();
        m.setCursor(4, 1);
        m.print("WIFI X");
        m.show();
        delay(1000);
        StateMachine::instance().gotoPage(Page::RHYTHM);
        return;
    }
    // NTR对时30s
    bool ntpOk = Ntp::instance().sync(30);
    WifiManager::instance().disconnect();

    if (!ntpOk) // 对时失败,则进入RHYTHM页面
    {
        auto &m = LedMatrix::instance();
        m.clear();
        m.setCursor(2, 1);
        m.print("TIME X");
        m.show();
        delay(1000);
        StateMachine::instance().gotoPage(Page::RHYTHM);
        return;
    }
    // 对时成功

    Ntp::instance().startTicker();

    //
    auto &clk = Store::instance().clock();
    if (clk.enabled) // 如果闹钟启用
    {
        //{目标时间戳} - {当前时间戳} = {剩余的相对秒数 } (sec)
        int32_t sec = Ntp::secondsUntilAlarm(clk.hour, clk.minute);
        Ntp::instance().startClockTicker(sec);
    }

    StateMachine::instance().gotoPage(Page::TIME);
}
