#ifndef NTP_H
#define NTP_H

#include <Arduino.h>

class Ntp {
public:
    static Ntp& instance();

    bool sync(int timeoutSec);       // 返回是否对时成功
    void startTicker();              // 启动定时对时
    void stopTicker();
    bool isOk() const { return _ok; }

    /// 计算距离闹钟的剩余秒数（如果闹钟开启）
    static int32_t secondsUntilAlarm(int hour, int minute);

    // 闹钟倒计时
    void startClockTicker(int32_t seconds);
    void stopClockTicker();

    // 响铃状态
    bool isBelling() const;
    void setBelling(bool v);

    // 闹钟触发信号（Ticker回调设标志，loop()处理）
    bool isAlarmPending() const;
    void clearAlarmPending();

private:
    Ntp() = default;
    bool _ok = false;

    static void _onAlarm();
};

#endif
