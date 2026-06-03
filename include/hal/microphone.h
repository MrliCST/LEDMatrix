#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <Arduino.h>
#include "core/config.h"

class Microphone {
public:
    static Microphone& instance();

    void begin();            // 配置引脚 + 计算 sampling_period_us
    void sample();           // 采 1024 点 → FFT → 填 _bands[32]

    const int* bands() const { return _bands; }

private:
    Microphone() = default;

    int  _bands[32];
    long _samplingPeriodUs;

    double _vReal[SAMPLES];
    double _vImag[SAMPLES];

    void _accumulate(int fftIdx, int bandIdx);
};

#endif
