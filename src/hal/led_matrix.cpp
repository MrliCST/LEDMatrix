#include "hal/led_matrix.h"
#include <Fonts/MyFont.h>

static const uint8_t DIGIT[10][3] PROGMEM = {
    {0x0E, 0x11, 0x0E}, // 0
    {0x12, 0x1F, 0x10}, // 1
    {0x19, 0x15, 0x13}, // 2
    {0x11, 0x15, 0x1F}, // 3
    {0x07, 0x04, 0x1F}, // 4
    {0x17, 0x15, 0x1D}, // 5
    {0x0E, 0x15, 0x1D}, // 6
    {0x01, 0x01, 0x1F}, // 7
    {0x1F, 0x15, 0x1F}, // 8
    {0x17, 0x15, 0x1F}, // 9
};

static const uint8_t COLON_BMP[2] PROGMEM = {0x0A, 0x0A};
static const uint8_t DASH_BMP[3] PROGMEM = {0x04, 0x04, 0x04};

// 获取单例
LedMatrix &LedMatrix::instance() {
    static LedMatrix m;
    return m;
}

// 初始化
void LedMatrix::begin() {
    // 计算总宽度（32）和总高度（8）
    int totalWidth = MATRIX_SIDE * MATRIX_COUNT; // 8 * 4 = 32
    int totalHeight = MATRIX_SIDE;               // 8

    // 使用单面板构造函数（共 5 个参数）
    _matrix = new Adafruit_NeoMatrix(
        totalWidth, totalHeight, LED_DATA_PIN,
        NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
        NEO_GRB + NEO_KHZ800
    );
    
    _matrix->begin();
    _matrix->setFont(&MyFont);
    _matrix->setTextWrap(false);
    _matrix->setBrightness(BRIGHTNESS_DEFAULT);
}

void LedMatrix::clear()
{
    _matrix->fillScreen(0);
    _matrix->show();
}
void LedMatrix::show() { _matrix->show(); }
void LedMatrix::setBrightness(int val) { _matrix->setBrightness(val); }

void LedMatrix::drawPixel(int x, int y, uint16_t c) { _matrix->drawPixel(x, y, c); }
void LedMatrix::drawFastVLine(int x, int y, int h, uint16_t c) { _matrix->drawFastVLine(x, y, h, c); }
void LedMatrix::fillRect(int x, int y, int w, int h, uint16_t c) { _matrix->fillRect(x, y, w, h, c); }
void LedMatrix::drawBitmap(int x, int y, const uint8_t *b, int w, int h, uint16_t c) { _matrix->drawBitmap(x, y, b, w, h, c); }
void LedMatrix::drawRGBBitmap(int x, int y, const uint16_t *b, int w, int h) { _matrix->drawRGBBitmap(x, y, b, w, h); }
void LedMatrix::setCursor(int x, int y) { _matrix->setCursor(x, y); }
void LedMatrix::setTextColor(uint16_t color) { _matrix->setTextColor(color); }
void LedMatrix::print(const String &s) { _matrix->print(s); }
void LedMatrix::setFont(const GFXfont *f) { _matrix->setFont(f); }
void LedMatrix::setTextWrap(bool w) { _matrix->setTextWrap(w); }
uint16_t LedMatrix::color(uint8_t r, uint8_t g, uint8_t b) { return _matrix->Color(r, g, b); }

void LedMatrix::drawColumnMajorBitmap(int x, int y, const uint8_t *bmp, int w, int h, uint16_t color)
{
    for (int col = 0; col < w; col++)
    {
        uint8_t mask = pgm_read_byte(&bmp[col]);
        for (int row = 0; row < h; row++)
        {
            if (mask & (1 << row))
                _matrix->drawPixel(x + col, y + row, color);
        }
    }
}

void LedMatrix::drawDigit3x5(uint8_t digit, int x, int y, uint16_t color)
{
    if (digit > 9)
        return;
    for (int col = 0; col < 3; col++)
    {
        uint8_t mask = pgm_read_byte(&DIGIT[digit][col]);
        for (int row = 0; row < 5; row++)
        {
            if (mask & (1 << row))
                _matrix->drawPixel(x + col, y + row, color);
        }
    }
}

void LedMatrix::drawColon(int x, int y, uint16_t color)
{
    for (int col = 0; col < 2; col++)
    {
        uint8_t mask = pgm_read_byte(&COLON_BMP[col]);
        for (int row = 0; row < 5; row++)
        {
            if (mask & (1 << row))
                _matrix->drawPixel(x + col, y + row, color);
        }
    }
}

void LedMatrix::drawDash(int x, int y, uint16_t color)
{
    for (int col = 0; col < 3; col++)
    {
        uint8_t mask = pgm_read_byte(&DASH_BMP[col]);
        for (int row = 0; row < 5; row++)
        {
            if (mask & (1 << row))
                _matrix->drawPixel(x + col, y + row, color);
        }
    }
}
