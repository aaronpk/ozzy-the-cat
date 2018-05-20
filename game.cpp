#include "common.h"

/*  Defines  */

#define BOXES   128
#ifdef DEBUG
#define SNOWS   16
#else
#define SNOWS   64
#endif

enum {
    STATE_START = 0,
    STATE_GAME,
    STATE_OVER,
    STATE_PAUSE,
};

enum {
    ALIGN_LEFT = 0,
    ALIGN_CENTER,
    ALIGN_RIGHT,
};

#define coord(x)        ((x) * 64)
#define coordInv(x)     ((x) / 64)
#define secs(s)         ((s) * 60)
#define mid(a, b, c)    max(min((b), (c)), (a))
#define sign(n)         (((n) > 0) - ((n) < 0))

/*  Typedefs  */

typedef struct {
    int16_t x, y;
    int8_t  vx, vy;
} BOX;

typedef struct {
    int8_t x, y;
} SNOW;

/*  Local Functions  */

static void startGame(void);
static void moveCat(void);
static void initBoxes(void);
static void throwFirstBox(void);
static void moveBoxes(void);
static void boundBox(BOX *pBox, int gap);
static int  getNewBoxIndex(void);
static void initSnows(void);
static void moveSnows(void);

static void drawCat(void);
static void drawStartLogo(void);
static void drawBoxes(void);
static void drawSnows(void);
static void drawStrings(void);
static int  drawFigure(int16_t x, int16_t y, int value, uint8_t align);

/*  Local Variables  */

PROGMEM static const uint8_t imgBox[4][24] = { // 12x12 x4
    {0xf0, 0xc, 0x6, 0x2, 0xe1, 0x79, 0x79, 0xe1, 0x2, 0x6, 0xc, 0xf0, 0x00, 0x3, 0x6, 0x4, 0x9, 0x8, 0x8, 0x9, 0x4, 0x6, 0x3, 0x00, },
    {0xf0, 0xc, 0x6, 0x92, 0x91, 0xf1, 0xf1, 0x61, 0x62, 0x6, 0xc, 0xf0, 0x00, 0x3, 0x6, 0x4, 0x8, 0x8, 0x8, 0x8, 0x4, 0x6, 0x3, 0x00, },
    {0xf0, 0xc, 0x6, 0x2, 0x79, 0xe1, 0xe1, 0x79, 0x2, 0x6, 0xc, 0xf0, 0x00, 0x3, 0x6, 0x4, 0x8, 0x9, 0x9, 0x8, 0x4, 0x6, 0x3, 0x00, },
    {0xf0, 0xc, 0x6, 0x62, 0x61, 0xf1, 0xf1, 0x91, 0x92, 0x6, 0xc, 0xf0, 0x00, 0x3, 0x6, 0x4, 0x8, 0x8, 0x8, 0x8, 0x4, 0x6, 0x3, 0x00, }
};

PROGMEM static const uint8_t imgCat[8][48] = { // 24x12 x8
    {
        0x7C, 0xFE, 0xC7, 0xFE, 0xFC, 0xC6, 0xFF, 0xFE, 0xFC, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
        0xE0, 0xC0, 0xC0, 0x80, 0xC0, 0x62, 0x32, 0x1C, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x0F, 0x0F,
        0x03, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x03, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x0F, 0x0F, 0x00, 0x00
    },{
        0x7C, 0xFE, 0xC7, 0xFE, 0xFC, 0xC6, 0xFF, 0xFE, 0xFC, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
        0xE0, 0xC0, 0xC0, 0x80, 0xC0, 0xC0, 0x62, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x0F,
        0x0F, 0x03, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x07, 0x07, 0x03, 0x03, 0x07, 0x07, 0x03, 0x00, 0x00
    },{
        0x7C, 0xFE, 0xC7, 0xFE, 0xFC, 0xC6, 0xFF, 0xFE, 0xFC, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
        0xE0, 0xC0, 0xC0, 0x80, 0x80, 0x00, 0x80, 0xF8, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x03,
        0x0F, 0x0F, 0x03, 0x0F, 0x0F, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x0F, 0x0F, 0x03, 0x01, 0x01, 0x00
    },{
        0x7C, 0xFE, 0xDF, 0xFE, 0xFC, 0xDE, 0xFF, 0xFE, 0xFC, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
        0xE0, 0xC0, 0xC0, 0x80, 0xC0, 0xC0, 0x62, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x07,
        0x07, 0x03, 0x03, 0x07, 0x07, 0x03, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x0F, 0x0F, 0x03, 0x00, 0x00
    },{
        0x1C, 0x32, 0x62, 0xC0, 0x80, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0xC0, 0xFC,
        0xFE, 0xFF, 0xC6, 0xFC, 0xFE, 0xC7, 0xFE, 0x7C, 0x00, 0x00, 0x0F, 0x0F, 0x03, 0x03, 0x0F, 0x0F,
        0x03, 0x03, 0x03, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x0F, 0x0F, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00
    },{
        0x3C, 0x62, 0xC0, 0xC0, 0x80, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0xC0, 0xFC,
        0xFE, 0xFF, 0xC6, 0xFC, 0xFE, 0xC7, 0xFE, 0x7C, 0x00, 0x00, 0x03, 0x07, 0x07, 0x03, 0x03, 0x07,
        0x07, 0x03, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00
    },{
        0xF8, 0x80, 0x00, 0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0xC0, 0xFC,
        0xFE, 0xFF, 0xC6, 0xFC, 0xFE, 0xC7, 0xFE, 0x7C, 0x00, 0x01, 0x01, 0x03, 0x0F, 0x0F, 0x03, 0x03,
        0x0F, 0x0F, 0x03, 0x0F, 0x0F, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00
    },{
        0x3C, 0x62, 0xC0, 0xC0, 0x80, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0xC0, 0xFC,
        0xFE, 0xFF, 0xDE, 0xFC, 0xFE, 0xDF, 0xFE, 0x7C, 0x00, 0x00, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x0F,
        0x0F, 0x03, 0x03, 0x07, 0x07, 0x03, 0x03, 0x07, 0x07, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00
    }
};

PROGMEM static const uint8_t imgStartLogo[256] = { // 64x32
    0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x60, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0x70, 0x30, 0x30, 0x70, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0x80, 0xc0, 0xe0, 0x70, 0x38, 0x18, 0x00, 0x00, 0x00, 0xff, 0xff, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0x70, 0x30, 0x30, 0x70, 0xe0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x1, 0x7, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7, 0xf, 0x1c, 0x38, 0x30, 0x30, 0x38, 0x1c, 0xf, 0x7, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x3, 0x7, 0xe, 0x1c, 0x38, 0x30, 0x00, 0x00, 0x00, 0x7, 0xf, 0x1c, 0x38, 0x30, 0x30, 0x00, 0x00, 0x7, 0xf, 0x1c, 0x38, 0x30, 0x30, 0x38, 0x1c, 0x1f, 0x3f, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7, 0x1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1, 0x3, 0x6, 0x6, 0x6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6, 0x6, 0x6, 0x3, 0x1, 0x00, 0x00, 0x00, 0x00, 
};

// Courier New Bold digits
PROGMEM static const uint8_t imgFigures[10][20] = { // 10x14 x10
  {0x00, 0xf0, 0xfc, 0xe, 0x6, 0x6, 0xe, 0xfc, 0xf0, 0x00, 0x00, 0x3, 0xf, 0x1c, 0x18, 0x18, 0x1c, 0xf, 0x3, 0x00, },
  {0x00, 0xc, 0xc, 0xc, 0xfe, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x18, 0x18, 0x18, 0x00, },
  {0x00, 0x18, 0x1c, 0xe, 0x6, 0x86, 0xce, 0x7c, 0x38, 0x00, 0x00, 0x18, 0x1c, 0x1e, 0x1b, 0x19, 0x18, 0x18, 0x18, 0x00, },
  {0x00, 0xc, 0xc, 0xc6, 0xc6, 0xc6, 0xe6, 0xfc, 0x38, 0x00, 0xc, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x19, 0xf, 0x7, 0x00, },
  {0x00, 0x80, 0xc0, 0x70, 0x38, 0xe, 0xfe, 0xfe, 0x00, 0x00, 0x00, 0x3, 0x3, 0x3, 0x1b, 0x1b, 0x1f, 0x1f, 0x1b, 0x00, },
  {0x00, 0x7e, 0x7e, 0x66, 0x66, 0x66, 0xe6, 0xc6, 0x80, 0x00, 0xc, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x1c, 0xf, 0x7, 0x00, },
  {0x00, 0xe0, 0xf8, 0xdc, 0x6c, 0x66, 0xe6, 0xc6, 0x86, 0x00, 0x00, 0x3, 0xf, 0x1c, 0x18, 0x18, 0x1c, 0xf, 0x7, 0x00, },
  {0x00, 0xe, 0xe, 0x6, 0x6, 0x86, 0xf6, 0xfe, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x1f, 0x7, 0x00, 0x00, 0x00, },
  {0x00, 0x38, 0xfc, 0xc6, 0xc6, 0xc6, 0xc6, 0xfc, 0x38, 0x00, 0x00, 0x7, 0xf, 0x18, 0x18, 0x18, 0x18, 0xf, 0x7, 0x00, },
  {0x00, 0x78, 0xfc, 0xce, 0x86, 0x86, 0xce, 0xfc, 0xf0, 0x00, 0x00, 0x18, 0x18, 0x19, 0x19, 0xd, 0xe, 0x7, 0x1, 0x00, },
};

// Script style digits
/*
PROGMEM static const uint8_t imgFigures[10][20] = { // 10x14 x10
    {
        0xC0, 0xF8, 0xFC, 0x1E, 0x06, 0x02, 0xFE, 0xFE, 0xF8, 0x00,
        0x07, 0x1F, 0x1F, 0x18, 0x18, 0x1E, 0x0F, 0x07, 0x00, 0x00
    },{
        0x00, 0x00, 0x10, 0x18, 0xE8, 0xFC, 0xFE, 0x1E, 0x02, 0x00,
        0x10, 0x10, 0x18, 0x1E, 0x1F, 0x1F, 0x11, 0x10, 0x00, 0x00
    },{
        0x00, 0x00, 0x08, 0x0C, 0x86, 0xC2, 0xFE, 0x7E, 0x1C, 0x00,
        0x10, 0x1C, 0x1E, 0x1B, 0x19, 0x18, 0x1C, 0x0C, 0x00, 0x00
    },{
        0x00, 0x00, 0x04, 0x46, 0x62, 0xFE, 0xDE, 0x8E, 0x00, 0x00,
        0x18, 0x18, 0x10, 0x18, 0x1C, 0x0F, 0x07, 0x03, 0x00, 0x00
    },{
        0x00, 0x80, 0xC0, 0xE0, 0x30, 0xFC, 0xFE, 0xFE, 0x0E, 0x00,
        0x02, 0x03, 0x03, 0x02, 0x1E, 0x1F, 0x1F, 0x03, 0x02, 0x00
    },{
        0x00, 0x00, 0x00, 0x38, 0x3E, 0xE6, 0xE6, 0xC6, 0x07, 0x03,
        0x18, 0x18, 0x10, 0x18, 0x1C, 0x0F, 0x07, 0x03, 0x00, 0x00
    },{
        0xC0, 0xE0, 0xF8, 0x78, 0x2C, 0xE6, 0xE6, 0xC2, 0x02, 0x00,
        0x07, 0x1F, 0x1F, 0x18, 0x18, 0x1F, 0x0F, 0x07, 0x00, 0x00
    },{
        0x00, 0x08, 0x0E, 0x8E, 0xC6, 0x76, 0x3E, 0x0E, 0x06, 0x02,
        0x00, 0x1C, 0x1E, 0x0F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    },{
        0x00, 0x00, 0x9C, 0xBC, 0xFE, 0xE2, 0xF2, 0x1E, 0x0C, 0x00,
        0x0E, 0x1F, 0x1F, 0x10, 0x18, 0x1F, 0x0F, 0x07, 0x00, 0x00
    },{
        0x00, 0x78, 0xFC, 0xFE, 0x86, 0x82, 0xFE, 0xFE, 0xF8, 0x00,
        0x10, 0x10, 0x18, 0x18, 0x0E, 0x07, 0x07, 0x01, 0x00, 0x00
    }
};
*/

PROGMEM static const uint8_t imgDash[8] = { // 8x4
    0x0F, 0x0F, 0x03, 0x00, 0x0F, 0x0F, 0x03, 0x00
};

PROGMEM static const uint8_t imgReady[168] = { // 56x19 "catch the tokens!"
    0x00, 0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x88, 0x00, 0x40, 0xa8, 0xa8, 0xa8, 0xf0, 0x00, 0x8, 0x7e, 0x88, 0x88, 0x00, 0x70, 0x88, 0x88, 0x88, 0x88, 0x00, 0xfe, 0x10, 0x8, 0x8, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8, 0x7e, 0x88, 0x88, 0x00, 0xfe, 0x10, 0x8, 0x8, 0xf0, 0x00, 0x70, 0xa8, 0xa8, 0xa8, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0xf8, 0x20, 0x20, 0x00, 0xc0, 0x20, 0x20, 0x20, 0xc0, 0x00, 0xf8, 0x80, 0x40, 0x20, 0x00, 0xc0, 0xa0, 0xa0, 0xa0, 0xc0, 0x00, 0xe0, 0x40, 0x20, 0x20, 0xc0, 0x00, 0x40, 0xa0, 0xa0, 0xa0, 0x20, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1, 0x2, 0x2, 0x00, 0x1, 0x2, 0x2, 0x2, 0x1, 0x00, 0x3, 0x00, 0x1, 0x2, 0x00, 0x1, 0x2, 0x2, 0x2, 0x00, 0x00, 0x3, 0x00, 0x00, 0x00, 0x3, 0x00, 0x2, 0x2, 0x2, 0x2, 0x1, 0x00, 0x2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

PROGMEM static const uint8_t imgFever[248] = { // 124x14 "turbo tokens!"
    0x00, 0x00, 0x00, 0x60, 0x60, 0xfc, 0xfc, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00, 0xe0, 0xe0, 0xe0, 0x00, 0x00, 0xe0, 0xe0, 0xe0, 0x00, 0x00, 0xe0, 0xe0, 0xc0, 0xc0, 0x60, 0x60, 0x60, 0x00, 0x00, 0xfe, 0xfe, 0xfe, 0x60, 0x60, 0xe0, 0xe0, 0xc0, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0x60, 0x60, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0xfc, 0xfc, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0x60, 0x60, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0xfe, 0xfe, 0xfe, 0x80, 0xc0, 0xe0, 0x60, 0x60, 0x60, 0x00, 0x80, 0xc0, 0xe0, 0x60, 0x60, 0x60, 0xc0, 0xc0, 0x00, 0x00, 0xe0, 0xe0, 0xc0, 0x40, 0x60, 0x60, 0xe0, 0xc0, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0x60, 0x60, 0x60, 0x60, 0xc0, 0x00, 0x00, 0x00, 0xfc, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf, 0x1f, 0x1c, 0x18, 0x18, 0x18, 0x00, 0x00, 0xf, 0x1f, 0x1f, 0x18, 0x18, 0xf, 0x1f, 0x1f, 0x00, 0x00, 0x1f, 0x1f, 0x1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0xf, 0x18, 0x18, 0x1c, 0x1f, 0xf, 0x00, 0x00, 0x7, 0xf, 0x1c, 0x18, 0x18, 0x1c, 0xf, 0x7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf, 0x1f, 0x1c, 0x18, 0x18, 0x18, 0x00, 0x00, 0x7, 0xf, 0x1c, 0x18, 0x18, 0x1c, 0xf, 0x7, 0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x3, 0x3, 0xf, 0x1e, 0x18, 0x10, 0x00, 0x7, 0xf, 0x1f, 0x1b, 0x1b, 0x1b, 0x1b, 0xb, 0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x00, 0x00, 0xc, 0xd, 0x19, 0x1b, 0x1b, 0x1b, 0x1e, 0xe, 0x00, 0x00, 0x8, 0x1d, 0x1d, 0x8, 0x00, 0x00, 0x00, 
};

PROGMEM static const uint8_t imgOver[512] = { // 128x32 "access token expired"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0xf8, 0xf8, 0xc0, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x80, 0x00, 0x00, 0xfc, 0xfc, 0xfc, 0x00, 0x80, 0xc0, 0xc0, 0x40, 0x40, 0x00, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x80, 0x00, 0x00, 0xc0, 0xc0, 0x80, 0x80, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x3d, 0x36, 0x36, 0x36, 0x17, 0x3f, 0x3f, 0x00, 0x00, 0xf, 0x1f, 0x3f, 0x30, 0x30, 0x30, 0x39, 0x10, 0x00, 0x00, 0xf, 0x1f, 0x3f, 0x30, 0x30, 0x30, 0x39, 0x10, 0x00, 0x00, 0xf, 0x1f, 0x3f, 0x36, 0x36, 0x36, 0x3f, 0x17, 0x00, 0x00, 0x11, 0x1b, 0x33, 0x36, 0x36, 0x36, 0x3d, 0x1c, 0x00, 0x00, 0x11, 0x1b, 0x33, 0x36, 0x36, 0x36, 0x3d, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x3f, 0x38, 0x30, 0x30, 0x30, 0x00, 0x00, 0xf, 0x1f, 0x39, 0x30, 0x30, 0x39, 0x1f, 0x1f, 0x00, 0x00, 0x3f, 0x3f, 0x3f, 0x7, 0x7, 0x1f, 0x3c, 0x30, 0x20, 0xf, 0x1f, 0x3f, 0x36, 0x36, 0x36, 0x3f, 0x17, 0x00, 0x00, 0x3f, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0x60, 0x60, 0x60, 0xc0, 0xc0, 0x00, 0x00, 0x20, 0x60, 0xe0, 0x80, 0x80, 0xe0, 0xe0, 0x20, 0x00, 0x00, 0xe0, 0xe0, 0xc0, 0x60, 0x60, 0xe0, 0xe0, 0xc0, 0x00, 0x00, 0x60, 0x60, 0x60, 0x62, 0xe7, 0xe7, 0x2, 0x00, 0x00, 0x00, 0xe0, 0xe0, 0xc0, 0xc0, 0x60, 0x60, 0x60, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0x60, 0x60, 0x60, 0xc0, 0xc0, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0x60, 0x60, 0xde, 0xfe, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7, 0xf, 0x1f, 0x1b, 0x1b, 0x1b, 0x1f, 0xb, 0x00, 0x00, 0x10, 0x1c, 0x1f, 0x7, 0x7, 0xf, 0x1c, 0x10, 0x10, 0x00, 0xff, 0xff, 0xff, 0x18, 0x18, 0x1c, 0x1f, 0xf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7, 0xf, 0x1f, 0x1b, 0x1b, 0x1b, 0x1f, 0xb, 0x00, 0x00, 0xf, 0x1f, 0x1c, 0x18, 0x18, 0xf, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x8, 0x1d, 0x1d, 0x8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

PROGMEM static const byte soundStart[] = {
    0x90, 72, 0, 100, 0x80, 0, 25,
    0x90, 74, 0, 100, 0x80, 0, 25,
    0x90, 76, 0, 100, 0x80, 0, 25,
    0x90, 77, 0, 100, 0x80, 0, 25,
    0x90, 79, 0, 200, 0x80, 0xF0
};

PROGMEM static const byte soundOver[] = {
    0x90, 55, 0, 120, 0x80, 0, 10,
    0x90, 54, 0, 140, 0x80, 0, 20,
    0x90, 53, 0, 160, 0x80, 0, 30,
    0x90, 52, 0, 180, 0x80, 0, 40,
    0x90, 51, 0, 200, 0x80, 0, 50,
    0x90, 50, 0, 220, 0x80, 0, 60,
    0x90, 49, 0, 240, 0x80, 0, 70,
    0x90, 48, 0, 260, 0x80, 0xF0
};

static uint8_t  state;
static bool     toDraw;
static uint32_t gameFrames;
static int      counter;
static int      timer;
static int8_t   ignoreCnt;
static uint     score;
static bool     isHiscore;
static uint8_t  catX;
static uint8_t  catAnim;
static uint8_t  meowY;
static uint8_t  boxCnt, boxIdx;
static BOX      boxAry[BOXES];
static SNOW     snowAry[SNOWS];

/*---------------------------------------------------------------------------*/
/*                              Main Functions                               */
/*---------------------------------------------------------------------------*/

void initGame(void)
{
    initBoxes();
    initSnows();
    catX = 160;
    catAnim = 0;
    meowY = 0;
    startGame();
}

bool updateGame(void)
{
#ifdef DEBUG
    if (dbgRecvChar == 'z') {
        initBoxes();
        startGame();
        dprintln("Reset");
}
#endif

    /*  In case of pausing  */
    if (state == STATE_PAUSE) {
        if (arduboy.buttonDown(A_BUTTON | B_BUTTON)) {
            state = STATE_GAME;
            ignoreCnt = 30; // a half second
            toDraw = true;
            dprintln("Resume");
        }
        if (arduboy.buttonDown(DOWN_BUTTON)) {
          // Abort immediately and return to title screen
          return true;
        }
        return false;
    }

    /*  Usual case  */
    moveCat();
    moveBoxes();
    if (SNOWS - boxCnt > 0) moveSnows();
    counter--;
    if (state == STATE_START) {
        if (counter == secs(1)) {
            throwFirstBox();
        } else if (counter == 0) {
            state = STATE_GAME;
        }
    } else if (state == STATE_GAME) {
        if (timer > 0) timer--;
        gameFrames++;
        if (boxCnt == 0) {
            state = STATE_OVER;
            isHiscore = (setLastScore(score, gameFrames) == 0);
            counter = secs(8);
            arduboy.playScore2(soundOver, 1);
            dprint("Game Over: score=");
            dprintln(score);
        } else if (ignoreCnt == 0 && arduboy.buttonDown(A_BUTTON | B_BUTTON)) {
            state = STATE_PAUSE;
            dprintln("Pause");
        }
    } else if (state == STATE_OVER) {
        if (ignoreCnt == 0 && arduboy.buttonDown(A_BUTTON | B_BUTTON)) {
            startGame();
        }
    }
    if (ignoreCnt > 0) ignoreCnt--;
    toDraw = true;

    return (state == STATE_OVER && counter == 0);
}

void drawGame(void)
{
    if (toDraw) {
        if (boxCnt >= BOXES / 3 && state == STATE_GAME && counter % 2 == 0) {
            return; // Frame skip
        }
        arduboy.clear();
        arduboy.drawFastVLine2(0, 0, 64, WHITE);
        arduboy.drawFastVLine2(127, 0, 64, WHITE);
        drawSnows();
        if (state != STATE_PAUSE) drawBoxes();
        drawCat();
        if (state == STATE_START) drawStartLogo();
        drawStrings();
        toDraw = false;
    }
}

/*---------------------------------------------------------------------------*/
/*                             Control Functions                             */
/*---------------------------------------------------------------------------*/

static void startGame()
{
    state = STATE_START;
    gameFrames = 0;
    counter = secs(2);
    timer = secs(120); // 2 minutes
    score = 0;
    arduboy.playScore2(soundStart, 0);
    dprintln("Start Game");
}

static void moveCat(void)
{
    int vx = 0;
    if (state == STATE_START) {
        vx = sign(64 - catX);
        catX += vx;
    } else {
        if (arduboy.buttonPressed(LEFT_BUTTON)) vx--;
        if (arduboy.buttonPressed(RIGHT_BUTTON)) vx++;
        catX = mid(0, catX + vx, 128);
    }

    if (vx != 0) {
        catAnim = ((catAnim & 0xF) + 1) & 0xF;
        if (vx > 0) catAnim |= 0x10;
    } else {
        catAnim &= 0x10;
    }

    if (meowY > 0) {
        meowY--;
    } else if (arduboy.buttonDown(DOWN_BUTTON)) {
        meowY = 8;
        arduboy.playScore2(soundMeow, 2);
        dprintln("meow");
    }
}

static void initBoxes(void)
{
    for (int i = 0; i < BOXES; i++) {
        boxAry[i].x = -1;
    }
    boxCnt = 0;
    boxIdx = 0;
}

static void throwFirstBox(void)
{
    BOX *pBox = &boxAry[0];
    pBox->x = coord(64);
    pBox->y = coord(32);
    pBox->vx = 0;
    pBox->vy = -64;
    boxCnt = 1;
}

static void moveBoxes(void)
{
    boolean isBound = false;
    for (int i = 0; i < BOXES; i++) {
        BOX *pBox = &boxAry[i];
        if (pBox->x < 0) continue;
        uint16_t lastY = pBox->y;
        pBox->x += pBox->vx;
        pBox->y += pBox->vy;
        pBox->vy++;

        if (pBox->x < coord(4)) {
            pBox->x = coord(4);
            pBox->vx = -pBox->vx * 3 / 4;
        } else if (pBox->x > coord(124)) {
            pBox->x = coord(124);
            pBox->vx = -pBox->vx * 3 / 4;
        }

        int gap = pBox->x - coord(catX);
        if (timer > 0 && pBox->y >= coord(60) && lastY < coord(60) && abs(gap) <= coord(10)) {
            boundBox(pBox, coordInv(gap));
            isBound = true;
            score++;
        } else if (pBox->y > coord(70)) {
            pBox->x = -1;
            boxCnt--;
        }
    }

    if (isBound) {
        arduboy.tunes.tone((timer > secs(30)) ? 1440 : 1920, 15);
        dprint("boxCnt=");
        dprintln(boxCnt);
    }
}

static void boundBox(BOX *pBox, int gap)
{
    int16_t x = pBox->x;
    int8_t  vx = pBox->vx;
    int8_t  vy = -pBox->vy / 2;

    int mul = 2;
    if (timer > secs(30)) {
        mul = 2;
        gap -= 4;
    } else {
        mul = 3;
        gap -= 8;
    }
    for (int i = 0; i < mul; i++, gap += 8) {
        if (i > 0 && boxCnt < BOXES) {
            pBox = &boxAry[getNewBoxIndex()];
            pBox->x = x;
            boxCnt++;
        }
        pBox->y = coord(60);
        pBox->vx = vx + gap;
        pBox->vy = vy - (40 - abs(gap));
    }
}

static int getNewBoxIndex(void)
{
    do {
        boxIdx = (boxIdx + 1) % BOXES;
    } while (boxAry[boxIdx].x >= 0);
    return boxIdx;
}

static void initSnows(void)
{
    for (int i = 0; i < SNOWS; i++) {
        SNOW *pSnow = &snowAry[i];
        pSnow->x = random(128);
        pSnow->y = random(64);
    }
}

static void moveSnows(void)
{
    for (int i = 0; i < SNOWS; i++) {
        SNOW *pSnow = &snowAry[i];
        pSnow->x += random(-1, 2);
        pSnow->y += random(2);
        if (pSnow->y >= 64) {
            pSnow->x = random(128);
            pSnow->y = 0;
        }
    }
}

/*---------------------------------------------------------------------------*/
/*                              Draw Functions                               */
/*---------------------------------------------------------------------------*/

static void drawCat(void)
{
    if (meowY > 0) {
        arduboy.printEx(catX - 12, 46 + meowY / 2, F("MEOW"));
    }
    arduboy.fillRect2(catX - 12 + (catAnim & 0x10), 55, 8, 3, BLACK);
    arduboy.drawBitmap(catX - 12, 52, imgCat[catAnim / 4], 24, 12, WHITE);
}

static void drawStartLogo(void)
{
    arduboy.drawBitmap(counter * 2 - 112, 16, imgStartLogo, 64, 32, WHITE);
}

static void drawBoxes(void)
{
    for (int i = 0; i < BOXES; i++) {
        BOX *pBox = &boxAry[i];
        if (pBox->x < 0) continue;
        int16_t x = coordInv(pBox->x) - 4;
        int16_t y = coordInv(pBox->y) - 4;
        int8_t anim = x % 4;
        arduboy.drawBitmap(x, y, imgBox[anim], 12, 12, WHITE);
    }
}

static void drawSnows(void)
{
    for (int i = 0, c = SNOWS - boxCnt; i < c; i++) {
        SNOW *pSnow = &snowAry[i];
        arduboy.drawPixel(pSnow->x, pSnow->y, WHITE);
    }
}

static void drawStrings(void)
{
    if (state == STATE_START) {
        arduboy.drawBitmap(36, 0, imgReady, 56, 19, WHITE);
    } else {
        /*  Score  */
        drawFigure(2, 0, score, ALIGN_LEFT);

        /*  Timer  */
        if (timer >= secs(60)) {
            drawFigure(92, 0, timer / 3600, ALIGN_LEFT);
            arduboy.drawBitmap(102, 0, imgDash, 4, 4, WHITE);
            drawFigure(116, 0, timer % 3600 / 60, ALIGN_RIGHT);
        } else {
            drawFigure(98, 0, timer / 60, ALIGN_RIGHT);
            arduboy.drawBitmap(108, 0, imgDash, 8, 4, WHITE);
            drawFigure(116, 0, timer % 60 / 6, ALIGN_LEFT);
        }
        if (state == STATE_GAME) {
            if (timer <= secs(30) && timer > secs(27)) {
                arduboy.drawBitmap(2, 16, imgFever, 124, 14, (timer % 8 < 4) ? WHITE : BLACK);
            }
        } else if (state == STATE_OVER) {
            int y = 12;
            if (counter > 458) {
                y -= (counter - 458) * 2;
            } else if (counter > secs(7)) {
                y += counter % 2;
            }
            arduboy.drawBitmap(0, y, imgOver, 128, 32, WHITE);
            if (isHiscore && counter % 8 < 4) {
                arduboy.printEx(31, 46, F("NEW RECORD!"));
            }
        } else if (state == STATE_PAUSE) {
            arduboy.printEx(49, 46, F("PAUSE"));
        }
    }
}

static int  drawFigure(int16_t x, int16_t y, int value, uint8_t align)
{
    int k = (value > 9) ? drawFigure(x - align * 5, y, value / 10, align) : 0;
    arduboy.drawBitmap(x + k, y, imgFigures[value % 10], 10, 14, WHITE);
    return k + 10 - align * 5;
}
