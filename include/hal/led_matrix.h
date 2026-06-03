#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <Adafruit_NeoMatrix.h>
#include <Adafruit_GFX.h>
#include "core/config.h"

class LedMatrix {
public:
    static LedMatrix& instance();

    void begin();                                    // 初始化矩阵 + 字体
    void clear();                                    // fillScreen(0) + show()
    void show();                                     // matrix.show()
    void setBrightness(int val);

    void   drawPixel(int x, int y, uint16_t color);
    void   drawFastVLine(int x, int y, int h, uint16_t color);
    void   drawBitmap(int x, int y, const uint8_t* bmp, int w, int h, uint16_t color);
    void   drawRGBBitmap(int x, int y, const uint16_t* bmp, int w, int h);
    void   setCursor(int x, int y);
    void   print(const String& s);
    void   setFont(const GFXfont* f);
    void   setTextWrap(bool w);

    uint16_t color(uint8_t r, uint8_t g, uint8_t b);  // 直接调 matrix.Color

    // 外部可直接拿引用做复杂绘制（谨慎）
    Adafruit_NeoMatrix& raw() { return *_matrix; }

private:
    LedMatrix() = default;
    Adafruit_NeoMatrix* _matrix = nullptr;
};

#endif
