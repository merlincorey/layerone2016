// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include < afxwin.h>

#define NUMBER_NODES        (	1 )


#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <winsock2.h>
#include <stdint.h>
#include <math.h>

#include "bass.h"
#include "basswasapi.h"



extern volatile unsigned char update[NUMBER_NODES];

#undef RGB

#define RGB(r,g,b)          ((COLORREF)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "bass.lib")
#pragma comment(lib, "Winmm.lib")


typedef struct threaddata_tag {

    volatile double before_time, after_time;
    LARGE_INTEGER m_depart;
    volatile SOCKET sock;
    LARGE_INTEGER now;
    LARGE_INTEGER freq;

    sockaddr_in dest_ip[24];

} threaddata;


typedef union rgb_t {

    uint32_t rgb;

    struct rgb_tag {
        uint8_t b, r, g, pad;
    } c;

} led_color;


typedef union hsv_t {

    uint32_t hsv;

    struct hsv_tag {
        uint8_t h, s, v, pad;
    } h;

} hsv_color;
#define StripLights_MEMORY_TYPE  StripLights_MEMORY_RGB

DWORD PlayFile ( const char *file );
DWORD PlayFile ( void );

#define StripLights_COLUMNS     (20)
#define StripLights_ROWS        1
#define StripLights_MIN_X        0u
#define StripLights_MAX_X        (StripLights_COLUMNS - 1)
#define StripLights_MIN_Y        0u
#define StripLights_MAX_Y        (StripLights_ROWS - 1)

//#define StripLights_RBCOLORS     48
#define StripLights_COLOR_WHEEL_SIZE  24


#define StripLights_RED_MASK   0x0000FF00
#define StripLights_GREEN_MASK 0x000000FF
#define StripLights_BLUE_MASK  0x00FF0000

#if(StripLights_MEMORY_TYPE == StripLights_MEMORY_RGB)
#define getColor( a ) StripLights_CLUT[a]
#else  /* Else use lookup table */
#define getColor( a ) a
#endif


#define StripLights_CWHEEL_SIZE 24
#define StripLights_YELLOW      getColor(1)
#define StripLights_GREEN       getColor(4)
#define StripLights_ORANGE      getColor(20)
#define StripLights_BLACK       getColor((0 + StripLights_CWHEEL_SIZE))
#define StripLights_OFF         getColor((0 + StripLights_CWHEEL_SIZE))
#define StripLights_LTBLUE      getColor((1 + StripLights_CWHEEL_SIZE))
#define StripLights_MBLUE       getColor((2 + StripLights_CWHEEL_SIZE))
#define StripLights_BLUE        getColor((3 + StripLights_CWHEEL_SIZE))
#define StripLights_LTGREEN     getColor((4 + StripLights_CWHEEL_SIZE))
#define StripLights_MGREEN      getColor((8 + StripLights_CWHEEL_SIZE))
//#define StripLights_GREEN       (12 + StripLights_CWHEEL_SIZE)
#define StripLights_LTRED       getColor((16 + StripLights_CWHEEL_SIZE))
#define StripLights_LTYELLOW    getColor((20 + StripLights_CWHEEL_SIZE))
#define StripLights_MGRED       getColor((32 + StripLights_CWHEEL_SIZE))
#define StripLights_RED         getColor((48 + StripLights_CWHEEL_SIZE))
#define StripLights_MAGENTA     getColor((51 + StripLights_CWHEEL_SIZE))
#define StripLights_WHITE       getColor((63 + StripLights_CWHEEL_SIZE))
extern UINT32 GetRand ( int size );

uint32_t TweenerALLHSV ( uint16_t count, uint32_t from, uint32_t to, uint32_t delay );
uint32_t TweenerHSV ( uint16_t startx,  uint16_t count, uint32_t from, uint32_t to, uint32_t delay, int direction );
void Stripe ( void );
void SingleLEDPingPong ( uint16_t count , uint8_t fade_amount, uint32_t c1 );
void CandyCane ( uint16_t count , uint32_t c1, uint32_t c2 );
void Snake ( uint16_t count );
void Twinkle ( uint16_t count );
void ColorWheel ( uint16_t count );
void Tweener ( uint16_t count , uint32_t source );
void ColorFader ( int count  , uint32_t color );
void Tween1 ( void );
void Icicle ( uint8_t redraw, uint8_t length, int fade_amount );
void Sparkler ( uint16_t runtime, int fade_amount , int num_sparkles , char white );
void CandyCaneSmooth ( uint16_t count , led_color c1, led_color c2 );

uint32_t TweenC1toC2 ( led_color c1, led_color c2, int amount );
int TweenC1toC2Range ( uint16_t count, uint16_t x, uint32_t source, uint32_t target );
hsv_color RgbToHsv ( led_color rgb );
led_color HsvToRgb ( hsv_color hsv );
void StripLights_PixelHSV ( int32_t x, int32_t y, hsv_color hsv );
void StripLights_DisplayClearHSV ( hsv_color hsv );
led_color hsv_to_rgb ( hsv_color hsv );
hsv_color rgb_to_hsv ( led_color rgb );
uint8_t TweenU8toU8 ( uint8_t source, uint8_t target, int amount );
void FadeToColor ( uint16_t startx, uint16_t count, uint32_t target, uint32_t delay, int direction ) ;
void FadeStrip (  uint16_t start, int16_t length , int percentage );
uint32_t AddColor ( led_color c1, led_color c2 );
void Select ( unsigned char ch );
int StripLights_Ready ( void );
void StripLights_Trigger ( int i );
void CyDelay ( uint32_t ms );
void StripLights_Pixel ( int32_t x, int32_t y, uint32_t color );
uint32_t StripLights_GetPixel ( int32_t x, int32_t y );
void StripLights_DisplayClear ( uint32_t color );
void StripLights_MemClear ( uint32_t color );

#define BOOT_CHECK()
#define UART_UartPutString  printf

extern uint8_t  StripLights_ledArray[StripLights_ROWS][StripLights_COLUMNS * 3];
extern uint8_t  last_StripLights_ledArray[StripLights_ROWS][StripLights_COLUMNS * 3];
extern const uint32_t StripLights_CLUT[];


///////////////////////////////////////////////////////////////////////
//
// typdefs for fixed-point fractional types.
//
// sfract7 should be interpreted as signed 128ths.
// fract8 should be interpreted as unsigned 256ths.
// sfract15 should be interpreted as signed 32768ths.
// fract16 should be interpreted as unsigned 65536ths.
//
// Example: if a fract8 has the value "64", that should be interpreted
//          as 64/256ths, or one-quarter.
//
//
//  fract8   range is 0 to 0.99609375
//                 in steps of 0.00390625
//
//  sfract7  range is -0.9921875 to 0.9921875
//                 in steps of 0.0078125
//
//  fract16  range is 0 to 0.99998474121
//                 in steps of 0.00001525878
//
//  sfract15 range is -0.99996948242 to 0.99996948242
//                 in steps of 0.00003051757
//

typedef uint8_t   fract8;   // ANSI: unsigned short _Fract
typedef int8_t    sfract7;  // ANSI: signed   short _Fract
typedef uint16_t  fract16;  // ANSI: unsigned       _Fract
typedef int16_t   sfract15; // ANSI: signed         _Fract
void FadeLED ( uint16_t i, uint32_t target, int percentage );

led_color HeatColor ( uint8_t temperature );
uint8_t scale8_video ( uint8_t i, fract8 scale );
led_color ColdColor ( uint8_t temperature );
void Fire2012a ( int cold );
void Sparky ( uint16_t count );
void Falloff ( uint16_t count );

#define MAX_ADAPTER_NAME_LENGTH 256
#define MAX_ADAPTER_DESCRIPTION_LENGTH 128
#define MAX_ADAPTER_ADDRESS_LENGTH 8

int InitBASS ( HINSTANCE hInst );

#define SPECWIDTH 640   // display width
#define SPECHEIGHT 127  // height (changing requires palette adjustments too)
extern HWND win;
extern char tmp_buffer[256];

//Functions
void GetMacAddress ( unsigned char * , struct in_addr );
