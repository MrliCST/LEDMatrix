#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <Adafruit_NeoMatrix.h>
#include <Adafruit_GFX.h>
#include "core/config.h"

class LedMatrix // LED矩阵
{
public:
    static LedMatrix &instance();

    void begin(); // 初始化矩阵 + 字体
    void clear(); // fillScreen(0) + show()
    void show();  // matrix.show()
    void setBrightness(int val);
    // 画点、线、矩形
    void drawPixel(int x, int y, uint16_t color);
    void drawFastVLine(int x, int y, int h, uint16_t color);
    void fillRect(int x, int y, int w, int h, uint16_t color);
    // 画单色位图
    void drawBitmap(int x, int y, const uint8_t *bmp, int w, int h, uint16_t color);
    // 画列优先单色位图（与 DIGIT/bell/xmark 字模格式一致）
    void drawColumnMajorBitmap(int x, int y, const uint8_t *bmp, int w, int h, uint16_t color);
    // 画彩色位图
    void drawRGBBitmap(int x, int y, const uint16_t *bmp, int w, int h);
    // 画3x5数字/符号
    void drawDigit3x5(uint8_t digit, int x, int y, uint16_t color);
    void drawColon(int x, int y, uint16_t color);
    void drawDash(int x, int y, uint16_t color);
    // 文字打印流
    void setCursor(int x, int y);      // 设置光标位置
    void setTextColor(uint16_t color); // 设置文字颜色
    void print(const String &s);       // 打印字符串
    void setFont(const GFXfont *f);    // 设置字体
    void setTextWrap(bool w);          // 设置文字是否换行

    uint16_t color(uint8_t r, uint8_t g, uint8_t b); // 直接调 matrix.Color

    // 外部可直接拿引用做复杂绘制（谨慎）
    Adafruit_NeoMatrix &raw() { return *_matrix; }

private:
    LedMatrix() = default;
    Adafruit_NeoMatrix *_matrix = nullptr;
};

#endif
