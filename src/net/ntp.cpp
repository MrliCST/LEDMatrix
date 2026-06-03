#include "net/ntp.h"
#include "core/config.h"
#include "core/store.h"
#include "core/state_machine.h"
#include <WiFi.h>
#include <time.h>
#include <Ticker.h>

static Ticker _ntpTicker;
static Ticker _clockTicker;
static bool   _isBelling = false;

Ntp& Ntp::instance() { static Ntp n; return n; }

bool Ntp::sync(int timeoutSec) {
    configTime(8 * 3600, 0, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

    struct tm ti;
    time_t start;
    time(&start);

    while (!getLocalTime(&ti)) {
        time_t now;
        time(&now);
        if ((now - start) > timeoutSec) { _ok = false; return false; }
        delay(500);
    }
    _ok = true;
    return true;
}

void Ntp::startTicker() {
    _ntpTicker.attach(TIME_CHECK_INTERVAL, []() {
        StateMachine::instance().setCheckingTime(true);  // loop() 里处理
    });
}

void Ntp::stopTicker() {
    _ntpTicker.detach();
}

int32_t Ntp::secondsUntilAlarm(int hour, int minute) {
    time_t now;
    time(&now);
    struct tm* t = localtime(&now);
    struct tm alarmTm = *t;
    alarmTm.tm_hour = hour;
    alarmTm.tm_min  = minute;
    alarmTm.tm_sec  = 0;
    int32_t diff = mktime(&alarmTm) - now;
    if (diff < 0) diff += 86400;  // 已经过了，明天
    return diff;
}
