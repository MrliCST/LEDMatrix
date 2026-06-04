#include "core/store.h"

Store& Store::instance() {
    static Store s;
    return s;
}

void Store::load() {
    _prefs.begin("matrix");

    _wifi.ssid     = _prefs.getString("ssid", "");
    _wifi.pass     = _prefs.getString("pass", "");
    _wifi.apConfig = _prefs.getBool("apConfig", true);

    _color.r = _prefs.getInt("red",   255);
    _color.g = _prefs.getInt("green", 0);
    _color.b = _prefs.getInt("blue",  0);

    _mode.timeMode   = _prefs.getInt("timePage",   TIME_FORMAT_H_M_S);
    _mode.rhythmMode = _prefs.getInt("rhythmPage", RHYTHM_MODE_1);
    _mode.animMode   = _prefs.getInt("animPage",   ANIM_MODE_1);

    _clock.hour      = _prefs.getInt("clockH",       0);
    _clock.minute    = _prefs.getInt("clockM",       0);
    _clock.bellIndex = _prefs.getInt("clockBellNum", 0);
    _clock.enabled   = _prefs.getBool("clockOpen", false);

    _sys.brightness  = _prefs.getInt("brightness", BRIGHTNESS_DEFAULT);

    _prefs.end();
    _calcColors();
}

void Store::saveWifi(const WifiConfig& cfg) {
    _wifi = cfg;
    _prefs.begin("matrix");
    _prefs.putString("ssid",     cfg.ssid);
    _prefs.putString("pass",     cfg.pass);
    _prefs.putBool("apConfig",   cfg.apConfig);
    _prefs.end();
}

void Store::saveColor(const ColorConfig& cfg) {
    _color = cfg;
    _prefs.begin("matrix");
    _prefs.putInt("red",   cfg.r);
    _prefs.putInt("green", cfg.g);
    _prefs.putInt("blue",  cfg.b);
    _prefs.end();
    _calcColors();
}

void Store::saveMode(const ModeConfig& cfg) {
    _mode = cfg;
    _prefs.begin("matrix");
    _prefs.putInt("timePage",   cfg.timeMode);
    _prefs.putInt("rhythmPage", cfg.rhythmMode);
    _prefs.putInt("animPage",   cfg.animMode);
    _prefs.end();
}

void Store::saveClock(const ClockConfig& cfg) {
    _clock = cfg;
    _prefs.begin("matrix");
    _prefs.putInt("clockH",       cfg.hour);
    _prefs.putInt("clockM",       cfg.minute);
    _prefs.putInt("clockBellNum", cfg.bellIndex);
    _prefs.putBool("clockOpen",   cfg.enabled);
    _prefs.end();
}

void Store::saveBrightness(int val) {
    _sys.brightness = val;
    _prefs.begin("matrix");
    _prefs.putInt("brightness", val);
    _prefs.end();
}

void Store::_calcColors() {
    _color.mainColor = ((_color.r & 0xF8) << 8) | ((_color.g & 0xFC) << 3) | (_color.b >> 3);

    int maxC = max(_color.r, max(_color.g, _color.b));
    bool found = false;
    int wr = _color.r, wg = _color.g, wb = _color.b;
    if (wr == maxC && !found) { found = true; } else { wr = 255 - wr; }
    if (wg == maxC && !found) { found = true; } else { wg = 255 - wg; }
    if (wb == maxC && !found) { found = true; } else { wb = 255 - wb; }
    _color.weekColor = ((wr & 0xF8) << 8) | ((wg & 0xFC) << 3) | (wb >> 3);
}
