#include "hal/led_matrix.h"
#include <Fonts/MyFont.h>

// 获取单例
LedMatrix& LedMatrix::instance() {
    static LedMatrix m;
    return m;
}

// 初始化
void LedMatrix::begin() {
    _matrix = new Adafruit_NeoMatrix(
        MATRIX_SIDE, MATRIX_SIDE, MATRIX_COUNT, 1, LED_DATA_PIN,
        NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE +
        NEO_TILE_TOP  + NEO_TILE_LEFT  + NEO_TILE_ROWS  + NEO_TILE_PROGRESSIVE,
        NEO_GRB + NEO_KHZ800
    );
    _matrix->begin();
    _matrix->setFont(&MyFont);
    _matrix->setTextWrap(false);
    _matrix->setBrightness(BRIGHTNESS_DEFAULT);
}

void LedMatrix::clear() { _matrix->fillScreen(0); _matrix->show(); }
void LedMatrix::show()  { _matrix->show(); }
void LedMatrix::setBrightness(int val) { _matrix->setBrightness(val); }

void   LedMatrix::drawPixel(int x, int y, uint16_t c)   { _matrix->drawPixel(x, y, c); }
void   LedMatrix::drawFastVLine(int x, int y, int h, uint16_t c) { _matrix->drawFastVLine(x, y, h, c); }
void   LedMatrix::drawBitmap(int x, int y, const uint8_t* b, int w, int h, uint16_t c) { _matrix->drawBitmap(x, y, b, w, h, c); }
void   LedMatrix::drawRGBBitmap(int x, int y, const uint16_t* b, int w, int h) { _matrix->drawRGBBitmap(x, y, b, w, h); }
void   LedMatrix::setCursor(int x, int y) { _matrix->setCursor(x, y); }
void   LedMatrix::print(const String& s) { _matrix->print(s); }
void   LedMatrix::setFont(const GFXfont* f) { _matrix->setFont(f); }
void   LedMatrix::setTextWrap(bool w) { _matrix->setTextWrap(w); }
uint16_t LedMatrix::color(uint8_t r, uint8_t g, uint8_t b) { return _matrix->Color(r, g, b); }