#ifndef STORE_H
#define STORE_H

#include <Preferences.h>
#include <Arduino.h>
#include "config.h"

struct WifiConfig {
    String ssid;
    String pass;
    bool   apConfig;
};

struct ColorConfig {
    uint8_t  r, g, b;        // 主色 RGB
    uint16_t mainColor;      // matrix.Color(r,g,b)
    uint16_t weekColor;      // 星期栏反色
};

struct ModeConfig {
    int timeMode;            // TIME_FORMAT_H_M_S / _H_M / _DATE
    int rhythmMode;          // RHYTHM_MODE_1 ~ 4
    int animMode;            // ANIM_MODE_1 ~ 3
};

struct ClockConfig {
    int  hour;
    int  minute;
    int  bellIndex;          // 铃声编号
    bool enabled;
};

struct SystemConfig {
    int brightness;
};

class Store {
public:
    static Store& instance();

    void load();

    void saveWifi(const WifiConfig& cfg);
    void saveColor(const ColorConfig& cfg);
    void saveMode(const ModeConfig& cfg);
    void saveClock(const ClockConfig& cfg);
    void saveBrightness(int val);

    const WifiConfig&   wifi()   const { return _wifi; }
    const ColorConfig&  color()  const { return _color; }
    const ModeConfig&   mode()   const { return _mode; }
    const ClockConfig&  clock()  const { return _clock; }
    int brightness() const { return _sys.brightness; }

private:
    Store() = default;
    Store(const Store&) = delete;
    Store& operator=(const Store&) = delete;

    Preferences   _prefs;
    WifiConfig    _wifi;
    ColorConfig   _color;
    ModeConfig    _mode;
    ClockConfig   _clock;
    SystemConfig  _sys;

    void _calcColors();
};

#endif
