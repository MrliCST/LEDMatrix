#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ---- 引脚（来源: diagram.json） ----
#define LED_DATA_PIN          23   // matrix6:DIN → esp:23
#define AUDIO_IN_PIN          34   // esp:34 → mic1:1
#define RANDOM_SEED_PIN       35   // 悬空引脚，给 randomSeed 用
#define BTN1_PIN              13   // esp:13 → btn1:2.l
#define BTN2_PIN              14   // esp:14 → btn2:2.l
#define BTN3_PIN              27   // esp:27 → btn3:2.l
#define BUZZER_PIN            25   // bz1:2 → esp:25

// ---- 点阵屏 ----
#define MATRIX_SIDE            8
#define MATRIX_COUNT           4
#define MATRIX_WIDTH          (MATRIX_SIDE * MATRIX_COUNT)   // 32
#define MATRIX_HEIGHT         MATRIX_SIDE                    // 8

// ---- 亮度 ----
#define BRIGHTNESS_MIN          5
#define BRIGHTNESS_MAX        145
#define BRIGHTNESS_SPACING      5
#define BRIGHTNESS_DEFAULT    130

// ---- 麦克风 ----
#define SAMPLING_FREQ        40000
#define SAMPLES               1024
#define NOISE                  200

// ---- 动画 ----
#define ANIM_INTERVAL1         60
#define ANIM_INTERVAL2         80
#define ANIM_INTERVAL3        100

// ---- NTP ----
#define TIME_CHECK_INTERVAL 18000              // 5小时

#define NTP_SERVER_1 "ntp2.aliyun.com"
#define NTP_SERVER_2 "ntp3.ict.ac.cn"
#define NTP_SERVER_3 "ntp4.ntsc.ac.cn"

// ---- SoftAP ----
#define AP_SSID         "EasyMatrix"
#define AP_IP_O1        192
#define AP_IP_O2        168
#define AP_IP_O3          1
#define AP_IP_O4          1

// ---- 枚举 ----
enum class Page        { SETTING, TIME, RHYTHM, ANIM, CLOCK, BRIGHT };
enum class ButtonEvent { SHORT_PRESS, LONG_PRESS };

enum TimeMode   { TIME_FORMAT_H_M_S = 1, TIME_FORMAT_H_M = 2, TIME_FORMAT_DATE = 3 };
enum RhythmMode { RHYTHM_MODE_1 = 1, RHYTHM_MODE_2 = 2, RHYTHM_MODE_3 = 3, RHYTHM_MODE_4 = 4 };
enum AnimMode   { ANIM_MODE_1 = 1,   ANIM_MODE_2 = 2,   ANIM_MODE_3 = 3 };
enum ClockSel   { CLOCK_SEL_H = 1,  CLOCK_SEL_M = 2,  CLOCK_SEL_BELL = 3 };

#endif
