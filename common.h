#ifndef COMMON_H
#define COMMON_H

#include "MyArduboy.h"

/*  Defines  */

//#define DEBUG
#define APP_TITLE       "OZZY THE CAT"
#define APP_INFO        "OZY-CAT VER 0.04"
#define APP_RELEASED    "MAY 2018"

/*  Typedefs  */

typedef unsigned char   uchar;
typedef unsigned int    uint;

/*  Global Functions  */

void initLogo(void);
bool updateLogo(void);
void drawLogo(void);

void initTitle(void);
bool updateTitle(void);
void drawTitle(void);
uint8_t setLastScore(int score, uint32_t time);

void initGame(void);
bool updateGame(void);
void drawGame(void);

/*  Global Variables  */

extern MyArduboy arduboy;

PROGMEM static const byte soundMeow[] = {
    0x90, 117, 0, 30,
    0x90, 116, 0, 30,
    0x90, 115, 0, 30,
    0x90, 113, 0, 40, 0x80, 0xF0
};


/*  For Debugging  */

#ifdef DEBUG
extern bool             dbgPrintEnabled;
extern char             dbgRecvChar;
#define dprint(...)     (!dbgPrintEnabled || Serial.print(__VA_ARGS__))
#define dprintln(...)   (!dbgPrintEnabled || Serial.println(__VA_ARGS__))
#else
#define dprint(...)
#define dprintln(...)
#endif

#endif // COMMON_H
