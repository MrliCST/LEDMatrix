#include "hal/microphone.h"
#include <arduinoFFT.h>

static ArduinoFFT<double> FFT;

Microphone& Microphone::instance() {
    static Microphone m;
    return m;
}

void Microphone::begin() {
    pinMode(AUDIO_IN_PIN, INPUT);
    _samplingPeriodUs = round(1000000.0 * (1.0 / SAMPLING_FREQ));
}

void Microphone::sample() {
    // 1. 归零
    for (int i = 0; i < 32; i++) _bands[i] = 0;

    // 2. 等间隔采样
    for (int i = 0; i < SAMPLES; i++) {
        long t0 = micros();
        _vReal[i] = analogRead(AUDIO_IN_PIN);
        _vImag[i] = 0;
        while ((micros() - t0) < _samplingPeriodUs) { /* busy-wait */ }
    }

    // 3. FFT
    FFT.dcRemoval();
    FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(FFT_FORWARD);
    FFT.complexToMagnitude();

    // 4. 按频段分桶 (15个桶，原始代码的分桶规则)
    for (int i = 2; i < SAMPLES/2; i++) {
        if (_vReal[i] <= NOISE) continue;
             if (i >  6 && i <=  9) _bands[0]  += (int)_vReal[i];
        else if (i >  9 && i <= 11) _bands[1]  += (int)_vReal[i];
        else if (i > 11 && i <= 13) _bands[2]  += (int)_vReal[i];
        else if (i > 13 && i <= 15) _bands[3]  += (int)_vReal[i];
        else if (i > 15 && i <= 17) _bands[4]  += (int)_vReal[i];
        else if (i > 17 && i <= 19) _bands[5]  += (int)_vReal[i];
        else if (i > 19 && i <= 21) _bands[6]  += (int)_vReal[i];
        else if (i > 21 && i <= 23) _bands[7]  += (int)_vReal[i];
        else if (i > 23 && i <= 25) _bands[8]  += (int)_vReal[i];
        else if (i > 25 && i <= 27) _bands[9]  += (int)_vReal[i];
        else if (i > 27 && i <= 29) _bands[10] += (int)_vReal[i];
        else if (i > 29 && i <= 31) _bands[11] += (int)_vReal[i];
        else if (i > 31 && i <= 33) _bands[12] += (int)_vReal[i];
        else if (i > 33 && i <= 35) _bands[13] += (int)_vReal[i];
        else if (i > 35 && i <= 38) _bands[14] += (int)_vReal[i];
        // 原代码只分了 15 个桶，剩余 17 个留空（兼容后期扩展）
    }
}
