#include "net/ntp.h"
#include "core/config.h"
#include "core/store.h"
#include "core/state_machine.h"
#include <WiFi.h>
#include <time.h>
#include <Ticker.h>

static Ticker _ntpTicker;         // 定时授时器
static Ticker _clockTicker;       // 定时闹钟
static bool   _isBelling = false; // 是否正在响铃

// Network Time Protocol
// 创建网络时间协议对象
Ntp& Ntp::instance() { 
    static Ntp n; 
    return n; 
}

// 向网络请求授时
bool Ntp::sync(int timeoutSec) {
    configTime(8 * 3600, 0, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);  // 东八区，无夏令时

    struct tm ti;
    time_t start;
    time(&start);  // 记录当前的时间戳

    // 如果没有获取到时间，则等待
    while (!getLocalTime(&ti)) {
        time_t now;
        time(&now);
        if ((now - start) > timeoutSec) { _ok = false; return false; }  // 超时处理
        delay(500);
    }

    _ok = true;
    return true;
}

// 开始定时对时器
void Ntp::startTicker() {
    // 定时5小时，校准一次
    _ntpTicker.attach(TIME_CHECK_INTERVAL, []() {
        StateMachine::instance().setCheckingTime(true);  // loop() 里处理
    });
}

// 关闭定时对时器
void Ntp::stopTicker() {
    _ntpTicker.detach();
}

// 现在离闹钟响还有多少秒
int32_t Ntp::secondsUntilAlarm(int hour, int minute) {
    time_t now;
    time(&now);  // 获取当前时间戳
    struct tm* t = localtime(&now);  // 转为 "年月日时分秒"

    // 拷贝一份, 修改为闹钟的响铃时间
    struct tm alarmTm = *t;
    alarmTm.tm_hour = hour;
    alarmTm.tm_min  = minute;
    alarmTm.tm_sec  = 0;

    // 计算距离闹钟的秒数
    int32_t diff = mktime(&alarmTm) - now;
    if (diff < 0) diff += 86400;  // 已经过了，明天
    return diff;
}
