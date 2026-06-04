#include "hal/microphone.h"
#include <arduinoFFT.h>

static ArduinoFFT<double> *FFT = nullptr;

Microphone &Microphone::instance()
{
    static Microphone m;
    return m;
}

void Microphone::begin()
{
    pinMode(AUDIO_IN_PIN, INPUT);
    _samplingPeriodUs = round(1000000.0 * (1.0 / SAMPLING_FREQ)); // 计算采样周期=25us
    FFT = new ArduinoFFT<double>(_vReal, _vImag, SAMPLES, SAMPLING_FREQ);
}

void Microphone::sample()
{
    // 1. 归零
    for (int i = 0; i < 32; i++)
        _bands[i] = 0;

    // 2. 等间隔采样
    for (int i = 0; i < SAMPLES; i++)
    {
        long t0 = micros();
        _vReal[i] = analogRead(AUDIO_IN_PIN); // 读取34引脚的电压值
        _vImag[i] = 0;
        while ((micros() - t0) < _samplingPeriodUs)
        { /* busy-wait */
        }
    }

    // 3. FFT
    FFT->dcRemoval();                                 // 去直流分量
    FFT->windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); // 汉明窗
    FFT->compute(FFT_FORWARD);                        // (时域转频域）：任何复杂的声波波形，在物理上本质上都是由无数个不同频率、不同振幅的正弦波叠加而成的（傅里叶定律）
    FFT->complexToMagnitude();                        //_vReal[i]，索引 i 代表频率的高低（即音调），而数组里存的值代表该频率的振幅大小（即音量）

    // // 4. 按频段分桶 (15个桶，原始代码的分桶规则)
    // for (int i = 2; i < SAMPLES/2; i++) {
    //     if (_vReal[i] <= NOISE) continue;
    //          if (i >  6 && i <=  9) _bands[0]  += (int)_vReal[i];
    //     else if (i >  9 && i <= 11) _bands[1]  += (int)_vReal[i];
    //     else if (i > 11 && i <= 13) _bands[2]  += (int)_vReal[i];
    //     else if (i > 13 && i <= 15) _bands[3]  += (int)_vReal[i];
    //     else if (i > 15 && i <= 17) _bands[4]  += (int)_vReal[i];
    //     else if (i > 17 && i <= 19) _bands[5]  += (int)_vReal[i];
    //     else if (i > 19 && i <= 21) _bands[6]  += (int)_vReal[i];
    //     else if (i > 21 && i <= 23) _bands[7]  += (int)_vReal[i];
    //     else if (i > 23 && i <= 25) _bands[8]  += (int)_vReal[i];
    //     else if (i > 25 && i <= 27) _bands[9]  += (int)_vReal[i];
    //     else if (i > 27 && i <= 29) _bands[10] += (int)_vReal[i];
    //     else if (i > 29 && i <= 31) _bands[11] += (int)_vReal[i];
    //     else if (i > 31 && i <= 33) _bands[12] += (int)_vReal[i];
    //     else if (i > 33 && i <= 35) _bands[13] += (int)_vReal[i];
    //     else if (i > 35 && i <= 38) _bands[14] += (int)_vReal[i];
    //     // 原代码只分了 15 个桶，剩余 17 个留空（兼容后期扩展）
    // }
    // 4. 重构后的 32 频段对数分桶算法（完美适配 32x8 屏幕）
    // 探索范围：从 i=2 (约80Hz低音) 到 i=240 (约9.3kHz高音)，覆盖绝大部分音乐频率

    int numBands = 32; // 对应你 32 列屏幕
    int startBin = 2;  // 跳过 0 和 1 (0Hz是直流，1Hz太低没意义)
    int endBin = 240;  // 最高采集到 ~9.3kHz，覆盖人声和几乎所有乐器的基音

    // 人耳是对数探测器
    double logStart = log10((double)startBin);       // log10(2) ≈ 0.301
    double logEnd = log10((double)endBin);           // log10(240) ≈ 2.380
    double logStep = (logEnd - logStart) / numBands; // 每个桶的步长（2.380 - 0.301 = 2.079）/ 32 = 0.0659

    // 遍历 32 个桶，动态计算每个桶应该吞噬哪些 FFT 节点
    for (int bandIdx = 0; bandIdx < numBands; bandIdx++)
    {
        // 核心数学：用指数函数还原对数区间，算出每个桶的物理边界
        int binStart = (int)pow(10, logStart + bandIdx * logStep);
        int binEnd = (int)pow(10, logStart + (bandIdx + 1) * logStep);

        // 确保边界合法，至少包含一个节点
        if (binEnd <= binStart)
            binEnd = binStart + 1;

        long long bandEnergy = 0;
        int binsCount = 0;

        // 将该区间内的所有频率能量进行物理累加
        for (int i = binStart; i < binEnd && i < (SAMPLES / 2); i++)
        {
            if (_vReal[i] > NOISE)
            {
                bandEnergy += (int)_vReal[i];
                binsCount++;
            }
        }

        // 求平均值（或者可以直接保留总能量，根据你后面的缩放调整）
        if (binsCount > 0)
        {
            _bands[bandIdx] = bandEnergy / binsCount;
        }
        else
        {
            _bands[bandIdx] = 0;
        }
    }
}
