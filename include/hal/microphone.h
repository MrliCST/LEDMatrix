#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <Arduino.h>
#include "core/config.h"

class Microphone
{
public:
    static Microphone &instance();

    void begin();  // 配置引脚 + 计算 sampling_period_us
    void sample(); // 采 1024 点 → FFT → 填 _bands[32]

    const int *bands() const { return _bands; }

private:
    Microphone() = default;

    int _bands[32];         // 把人类能听到的声音频率范围（低音、中音、高音）平均切成了 32 份。
    long _samplingPeriodUs; // 采样周期（微秒）

    double _vReal[SAMPLES]; // 实部
    double _vImag[SAMPLES]; // 虚部

    void _accumulate(int fftIdx, int bandIdx);
};

#endif
