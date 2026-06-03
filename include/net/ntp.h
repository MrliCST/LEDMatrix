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

private:
    Ntp() = default;
    bool _ok = false;
};

#endif
