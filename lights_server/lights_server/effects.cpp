#include "stdafx.h"

void Fire2012 ( int cold );

//shortcut for generate a sparkle
inline uint8_t calculate_sparkle ( register uint8_t i )
{
    register uint8_t rnd;

    // every so often return very dark, to make it sparkle
    if ( rand() % 20 == 10 ) {
        return 3;
    }

    // pick value from hole range
    rnd = ( rand() % 255 );

    // scale down by level
    rnd /= ( i + 1 ) ;

    // scale down again if near end
    if ( i > 4 ) {
        rnd /= 2;
    }

    if ( i > 6 ) {
        rnd /= 2;
    }

    return rnd;
}

uint32_t TweenerHSV ( uint16_t startx, uint16_t count, uint32_t from, uint32_t to, uint32_t delay, int direction )
{
    int i;
    int offset;
    led_color frgb, trgb;
    hsv_color  src, target, result;

    trgb.rgb = to;
    frgb.rgb = from;

    src = rgb_to_hsv ( frgb ) ;
    target = rgb_to_hsv ( trgb );

    result = src;

    offset = startx;

    for ( i = 1 ; i < count; i ++ ) {
        result.h.h = TweenU8toU8 ( src.h.h, target.h.h, i );

        StripLights_PixelHSV ( offset, 0, result );

        offset += direction;

        if ( offset < ( int ) StripLights_MIN_X ) {
            offset = startx ;
        }

        if ( offset > startx + count ) {
            offset = StripLights_MIN_X ;
        }

        BOOT_CHECK();

        if ( delay ) {
            while ( StripLights_Ready() == 0 );

            StripLights_Trigger ( 1 );
            CyDelay ( delay );
        }

    }

    while ( StripLights_Ready() == 0 );

    StripLights_Trigger ( 1 );

    CyDelay ( 5 );


    return result.hsv;
}

uint32_t TweenerALLHSV ( uint16_t count, uint32_t from, uint32_t to, uint32_t delay )
{
    int i;

    led_color frgb, trgb;
    hsv_color  src, target, result;

    if ( to == from )
    { return 0; }

    trgb.rgb = to;
    frgb.rgb = from;

    src = rgb_to_hsv ( frgb ) ;
    target = rgb_to_hsv ( trgb );

    result = src;

    for ( int j = 0; j < count; j++ ) {
        for ( i = 1; i < 100; i++ ) {
            result.h.h = TweenU8toU8 ( src.h.h, target.h.h, i );

            StripLights_DisplayClearHSV ( result );

            if ( delay ) {
                CyDelay ( delay );
            }

            BOOT_CHECK();
        }
    }

    return result.hsv;
}


// quick helper function for testing hsv/rgb.
void StripLights_PixelHSV ( int32_t x, int32_t y, hsv_color hsv )
{
    led_color rgb;

    rgb = hsv_to_rgb ( hsv ) ;

    StripLights_Pixel ( x, y, rgb.rgb );
}

// quick helper function for testing hsv/rgb.
void StripLights_DisplayClearHSV ( hsv_color hsv )
{
    led_color rgb;

    rgb = hsv_to_rgb ( hsv ) ;

    StripLights_DisplayClear ( rgb.rgb );
}

#define CANDY_BLOCK_SIZE (4)

void CandyCane ( uint16_t count , uint32_t c1, uint32_t c2 )
{
    int i, x;
    uint8_t flip = 0;

    // Candy cane

    // loop effect for this many times
    for ( i = 0; i < count ; i++ ) {

        // all strip, for every other led
        for ( x = StripLights_MIN_X; x <= StripLights_MAX_X; x += CANDY_BLOCK_SIZE ) {
            // if flipped. draw c1,c2 otherwise c2,c1
            if ( flip ) {
                for ( int y = 0; y < CANDY_BLOCK_SIZE / 2; y++ ) {
                    StripLights_Pixel ( x + y, 0, c1 );
                }

                for ( int y = 0; y < CANDY_BLOCK_SIZE / 2; y++ ) {
                    StripLights_Pixel ( ( CANDY_BLOCK_SIZE / 2 ) + x + y, 0, c2 );
                }
            }

            else {
                for ( int y = 0; y < CANDY_BLOCK_SIZE / 2; y++ ) {
                    StripLights_Pixel ( x + y, 0, c2 );
                }

                for ( int y = 0; y < CANDY_BLOCK_SIZE / 2; y++ ) {
                    StripLights_Pixel ( ( CANDY_BLOCK_SIZE / 2 ) + x + y, 0, c1 );
                }
            }
        }

        // toggle flip
        flip = 1 - flip;

        // wait and trigger
        while ( StripLights_Ready() == 0 );

        StripLights_Trigger ( 1 );

        // delay between transitions
        CyDelay ( 275 );

        BOOT_CHECK();
    }
}

/*
 * FadeLED - Tween one LED to a specified colour
 *
 */
void FadeLED ( uint16_t i, uint32_t target, int percentage )
{
    led_color trgb, tmp;

    trgb.rgb = StripLights_GetPixel ( i, 0 );

    tmp.rgb = target;

    trgb.rgb = TweenC1toC2 ( trgb, tmp, percentage ) ;

    StripLights_Pixel ( i, 0, trgb.rgb );
}

#undef CANDY_BLOCK_SIZE
#define CANDY_BLOCK_SIZE ( 6 )

void CandyCaneSmooth ( uint16_t count , led_color c1, led_color c2 )
{
    int i, x, percentage;
    uint8_t flip = 0;
    uint32_t t1, t2;
    led_color black;
    black.rgb = 0;

    if ( c1.rgb == c2.rgb ) {
        return;
    }

    // loop effect for this many times
    for ( i = 0; i < count ; i++ ) {

        for ( percentage = 0; percentage <= 100; percentage += 10 ) {

            //  calculate target colours
            t1 = TweenC1toC2 ( c1, c2, percentage );
            t2 = TweenC1toC2 ( c2, c1, percentage );

            // all strip, for every other led
            for ( x = StripLights_MIN_X; x <= StripLights_MAX_X; x += CANDY_BLOCK_SIZE ) {

                for ( int y = 0; y < CANDY_BLOCK_SIZE / 2; y++ ) {
                    StripLights_Pixel ( x + y, 0, t1 );
                }

                for ( int y = 0; y < CANDY_BLOCK_SIZE / 2; y++ ) {
                    StripLights_Pixel ( ( CANDY_BLOCK_SIZE / 2 ) + x + y, 0, t2 );
                }

            }

            // toggle flip
            flip = 1 - flip;

            // wait and trigger
            while ( StripLights_Ready() == 0 );

            StripLights_Trigger ( 1 );

            // delay between transitions
            CyDelay ( 120 );
        }

        for ( percentage = 100; percentage > 0; percentage -= 10 ) {

            //  calculate target colours
            t1 = TweenC1toC2 ( c1, c2, percentage );
            t2 = TweenC1toC2 ( c2, c1, percentage );

            // all strip, for every other led
            for ( x = StripLights_MIN_X; x <= StripLights_MAX_X; x += CANDY_BLOCK_SIZE ) {

                for ( int y = 0; y < CANDY_BLOCK_SIZE / 2; y++ ) {
                    StripLights_Pixel ( x + y, 0, t1 );
                }

                for ( int y = 0; y < CANDY_BLOCK_SIZE / 2; y++ ) {
                    StripLights_Pixel ( ( CANDY_BLOCK_SIZE / 2 ) + x + y, 0, t2 );
                }

            }


            // toggle flip
            flip = 1 - flip;

            // wait and trigger
            while ( StripLights_Ready() == 0 );

            StripLights_Trigger ( 1 );

            // delay between transitions
            CyDelay ( 120 );



            BOOT_CHECK();
        }
    }
}



void Twinkle ( uint16_t count )
{
    int i, x;
    led_color col;
    uint32_t startColor;
    startColor = StripLights_WHITE;

    StripLights_MemClear ( 0 );

    col.rgb = 0;

    for ( x = 0; x <= count; x++ ) {

        col.c.r = rand();
        col.c.g = rand();
        col.c.b = rand();

        startColor = col.rgb;

        StripLights_Pixel ( rand() % StripLights_MAX_X, 0, startColor );

        for ( i = StripLights_MIN_X; i <= StripLights_MAX_X; i++ ) {
            col.rgb = StripLights_GetPixel ( i, 0 );

            if ( col.c.r > 0 ) {
                col.c.r -= col.c.r / 2;
            }

            if ( col.c.g > 0 ) {
                col.c.g -= col.c.g / 2;
            }

            if ( col.c.b > 0 ) {
                col.c.b -= col.c.b / 2;
            }

            StripLights_Pixel ( i, 0, col.rgb );
        }

        StripLights_Trigger ( 1 );
    }
}


void Icicle ( uint8_t redraw, uint8_t length, int fade_amount )
{
    int x, j, i;
    led_color temp;

    temp.rgb = 0;

    // for entire length of strip, plus engough to move it off the display)
    for ( x = StripLights_MIN_X; x <= StripLights_MAX_X + ( length * 2 ); x++ ) {

        if ( fade_amount ) {
            // Fade  strip
            FadeStrip ( StripLights_MIN_X, StripLights_MAX_X , fade_amount );
        }

        else {
            StripLights_MemClear ( 0 );
        }

        // draw in same place 8 times
        for ( j = 0 ; j < redraw ; j++ ) {

            // length of icicle
            for ( i = 0; i < length; i++ ) {
                // caculate a randow twink based on current position in length
                temp.c.r =
                    temp.c.g =
                        temp.c.b = calculate_sparkle ( i  );

                // draw a pixel at x+i
                StripLights_Pixel ( x + i, 0, temp.rgb );

                CyDelay ( 1 );
            }

            //push current data to led strip
            StripLights_Trigger ( 1 );
            CyDelay ( 3 );
        }

        // check if firmware load requested
    }
}

void FadeStrip (  uint16_t start, int16_t length , int percentage )
{
    led_color trgb;
    int i;
    led_color black;
    black.rgb = 0;

    for ( i = start; i <= start + length; i++ ) {

        // get pixel
        trgb.rgb = StripLights_GetPixel ( i, 0 );

        trgb.rgb = TweenC1toC2 ( trgb, black , percentage ) ;

        StripLights_Pixel ( i, 0, trgb.rgb );
    }
}


void CyDelay ( uint32_t ms )
{
    Sleep ( ms );
}

int StripLights_Ready ( void )
{
    int i;
    i = 0;

    for ( int x = 0; x < NUMBER_NODES; x++ ) {
        i += update[x];
    }

    return i ? 0 : 1;

}



void Sparkler ( uint16_t runtime, int fade_amount , int num_sparkles , char white )
{
    int x, j;
    led_color temp;

    // length of time to run
    for ( x = 0; x <= runtime ; x++ ) {
        if ( fade_amount ) {
            // Fade  strip
            FadeStrip ( StripLights_MIN_X, StripLights_MAX_X , fade_amount );
        }

        else {
            StripLights_MemClear ( 0 );
        }


        // draw in same place 8 times
        for ( j = 0 ; j < num_sparkles ; j++ ) {

            temp.c.r = calculate_sparkle ( j );

            if ( white ) {
                temp.c.g = temp.c.b = temp.c.r;
            }

            else {
                temp.c.g = calculate_sparkle ( j );
                temp.c.b = calculate_sparkle ( j );
            }

            // draw a pixel
            StripLights_Pixel ( rand() % ( StripLights_MAX_X + 1 ), 0, temp.rgb );
        }

        // strip ready?
        while ( StripLights_Ready() == 0 );

        //push current data to led strip
        StripLights_Trigger ( 1 );
        CyDelay ( 3 );
    }

    if ( fade_amount ) {
        // fade at end
        for ( x = 0; x <= 200 ; x++ ) {
            // Fade  strip
            FadeStrip ( StripLights_MIN_X, StripLights_MAX_X , fade_amount );

            // strip ready?
            while ( StripLights_Ready() == 0 );

            //push current data to led strip
            StripLights_Trigger ( 1 );
            CyDelay ( 3 );
        }
    }
}

void ColorWheel ( uint16_t count )
{
    static int i = 0xAAA , x;
    static uint32_t color;
    static uint32_t startColor ;

    if ( i >= count ) {
        i = 0;
    }

    if ( i == 0xaaa ) {
        i = 0;
        color = 0;
        startColor = 0;
    }

    for ( ; i < count ; i++ ) {
        color = startColor;

        for ( x = StripLights_MIN_X; x <= StripLights_MAX_X; x++ ) {
            StripLights_Pixel ( x, 0, getColor ( color ) );

            color++;

            if ( color >= StripLights_COLOR_WHEEL_SIZE ) {
                color = 0;
            }
        }

        startColor++;

        if ( startColor >= StripLights_COLOR_WHEEL_SIZE ) {
            startColor = 0;
        }

        while ( StripLights_Ready() == 0 );

        StripLights_Trigger ( 1 );

        CyDelay ( 50 );

    }
}

void SingleLEDPingPong ( uint16_t count , uint8_t fade_amount, uint32_t color )
{
    int i, x;

    for ( i = 0; i < count ; i++ ) {
        for ( x = StripLights_MIN_X; x <= StripLights_MAX_X; x++ ) {
            if ( fade_amount ) {
                // Fade  strip
                FadeStrip ( StripLights_MIN_X, StripLights_MAX_X , fade_amount );
            }

            else {
                StripLights_MemClear ( 0 );
            }

            StripLights_Pixel ( x, 0, color );

            while ( StripLights_Ready() == 0 );

            StripLights_Trigger ( 1 );
            CyDelay ( 5 );
            BOOT_CHECK();
        }

        for ( x = StripLights_MIN_X; x <= StripLights_MAX_X; x++ ) {
            if ( fade_amount ) {
                // Fade  strip
                FadeStrip ( StripLights_MIN_X, StripLights_MAX_X , fade_amount );
            }

            else {
                StripLights_MemClear ( 0 );
            }

            StripLights_Pixel ( StripLights_MAX_X - x, 0, color );

            while ( StripLights_Ready() == 0 );

            StripLights_Trigger ( 1 );

            CyDelay ( 5 );

            BOOT_CHECK();
        }
    }
}

// snake tail chaser
void Meet ( uint16_t count )
{
    int x;
    uint32_t startColor;
    uint32_t startColor2;

    startColor = getColor ( rand() % 73 ); // StripLights_RED;
    startColor2 = getColor ( rand() % 73 ); // StripLights_RED;

    for ( x = StripLights_MIN_X; x <= StripLights_MAX_X + 50; x++ ) {


        StripLights_Pixel ( x, 0, startColor );

        if ( rand() % 10 == 5 ) {
            startColor += 0x010101;
        }

        StripLights_Pixel ( StripLights_MAX_X - x, 0, startColor2 );

        if ( rand() % 10 == 5 ) {
            startColor += 0x010101;
        }

        if ( rand() % 10 == 5 ) {
            startColor2 += 0x010101;
        }

        FadeStrip ( StripLights_MIN_X, StripLights_MAX_X, 5 );

        while ( StripLights_Ready() == 0 );

        StripLights_Trigger ( 1 );
        CyDelay ( 15 );

    }
}

void Flashes ( uint16_t count )
{

}

void twinks ( int xbase, int length )
{
    for ( int x = 0; x < length; x++ ) {

        uint32_t r = calculate_sparkle ( rand() % 10 );

        StripLights_Pixel ( xbase + x , 0, RGB ( r, r, r ) );

    }
}
void Sparky ( uint16_t count )
{
    int x1;

    for ( int x = 0; x < count; x++ ) {
        // pick a starting spot
        x1 = rand() % StripLights_MAX_X;

        twinks ( x1, 5 + ( rand() % 5 ) );

        StripLights_Trigger ( 1 );
    }


}

void Falloff ( uint16_t count )
{
    int x, y;
    uint32_t startColor = 0;
    uint32_t startColor2 = 0;


    for ( int k = 0; k < 5; k++ ) {
        y = 1;

        startColor = 0;
        startColor2 = 0;

        while ( startColor == startColor2 ) {
            startColor = GetRand ( 24 ); // StripLights_RED;
            startColor2 = GetRand ( 24 ); // StripLights_RED;
        }

        for ( x = y - 1; x <= y; x++ ) {


            StripLights_Pixel ( x, 0, startColor );
            FadeStrip ( x - 5, 10, 10 );

            StripLights_Pixel ( StripLights_MAX_X - x, 0, startColor2 );
            FadeStrip ( ( StripLights_MAX_X - ( x - 5 ) ), 10, 10 );

            while ( StripLights_Ready() == 0 );

            StripLights_Trigger ( 1 );
            //CyDelay(5);

            y++;

            if ( y > StripLights_MAX_X + 20 ) {
                break;
            }
        }
    }
}



// snake tail chaser
void Snake ( uint16_t count )
{
    int i, x;
    uint32_t startColor;

    startColor = getColor ( rand() % 73 ); // StripLights_RED;

    for ( x = StripLights_MIN_X + 1; x <= StripLights_MAX_X + 30; x++ ) {

        if ( x & 6 )
            for ( i = StripLights_MIN_X; i <= StripLights_MAX_X; i++ ) {
                uint32_t colour = StripLights_GetPixel ( i, 0 );
                StripLights_Pixel ( i, 0, colour / 2 );
            }

        StripLights_Pixel ( x, 0, startColor );

        while ( StripLights_Ready() == 0 );

        StripLights_Trigger ( 1 );
        CyDelay ( 15 );

        if ( x % 10 == 5 ) {
            startColor += 0x010101;
        }

        BOOT_CHECK();
    }
}

void ColorFader ( int count , uint32_t color )
{
    while ( count-- ) {
        FadeToColor ( 0, StripLights_MAX_X + 1, color, 10, 1 );
    }
}


void FadeToColor ( uint16_t startx, uint16_t count, uint32_t target, uint32_t delay, int direction )
{
    int j, i;
    int offset, oldoffset;

    led_color frgb, trgb;
    hsv_color  src, target_hsv, result;

    frgb.rgb = target;

    src = rgb_to_hsv ( frgb ) ;

    offset = startx;

    for ( j = 0 ; j < 100 ; j ++ ) {

        oldoffset = offset;

        for ( i = 0 ; i < count; i ++ ) {

            // get colour of current LED at offset
            trgb.rgb = StripLights_GetPixel ( offset, 0 );

            // convert current led color to hsv
            target_hsv = rgb_to_hsv ( trgb );

            result = target_hsv;
            // tween, what we want to  what it is at percentage i
            result.h.h = TweenU8toU8 ( target_hsv.h.h, src.h.h, j );
            result.h.s = TweenU8toU8 ( target_hsv.h.s, src.h.s, j );
            result.h.v = TweenU8toU8 ( target_hsv.h.v, src.h.v, j );

            // update pixel
            StripLights_PixelHSV ( offset, 0, result );

            // handle travel direction of pixel
            offset += direction;

            if ( offset < ( int ) StripLights_MIN_X ) {
                offset = startx ;
            }

            if ( offset > startx + count ) {
                offset = StripLights_MIN_X ;
            }

        }

        // check bootloader mode
        BOOT_CHECK();

        // if wants a delay, update led strip and delay
        if ( delay ) {
            while ( StripLights_Ready() == 0 );

            StripLights_Trigger ( 1 );
            CyDelay ( delay );
        }

        offset = oldoffset;

    }
}

static int pos = 0, dir = 1; // Position, direction of "eye"

void Larson1 ( int count  )
{
    for ( int j = 0; j < count; j++ ) {
        // Draw 5 pixels centered on pos.  setPixelColor() will clip any
        // pixels off the ends of the strip, we don't need to watch for that.
        StripLights_Pixel ( pos - 2, 0,	RGB ( 0x10, 0x00, 0x00 ) ); // Dark red
        StripLights_Pixel ( pos - 1, 0,	RGB ( 0x80, 0x00, 0x00 ) ); // Medium red
        StripLights_Pixel ( pos, 0,		RGB ( 0xFF, 0x30, 0x00 ) ); // Center pixel is brightest
        StripLights_Pixel ( pos + 1, 0,	RGB ( 0x80, 0x00, 0x00 ) ); // Medium red
        StripLights_Pixel ( pos + 2, 0,	RGB ( 0x10, 0x00, 0x00 ) ); // Dark red

        while ( StripLights_Ready() == 0 );

        StripLights_Trigger ( 1 );

        CyDelay ( 1 );

        // Rather than being sneaky and erasing just the tail pixel,
        // it's easier to erase it all and draw a new one next time.
        StripLights_MemClear ( 0 );


        // Using this instead of the memclear would leave a trail behind it.
        //FadeStrip(StripLights_MIN_X, StripLights_MAX_X, 6);

        // Bounce off ends of strip
        pos += dir;

        if ( pos < 0 ) {
            pos = 1;
            dir = -dir;

        } else
            if ( pos >= StripLights_MAX_X ) {
                pos = StripLights_MAX_X - 2;
                dir = -dir;
            }
    }
}

UINT32 GetRand ( int size )
{
    return getColor ( rand() % size );

}

void Select ( unsigned char ch )
{
    srand ( clock() );

    printf ( "%c - ", ch );

    switch ( ch ) {
        case 'a':
            printf ( "Icicle\n" );
            Icicle ( 2, 9, 6 );
            break;

        case 'b':
            printf ( "Sparkler RGB\n" );
            Sparkler ( 300, 10, 18, 0 );
            break;

        case 'c':
            printf ( "Sparkler White\n" );
            Sparkler ( 300, 1, 18, 1 );
            break;

        case 'd':
            printf ( "ColorFader Loop\n" );
            StripLights_DisplayClear ( getColor ( rand() % StripLights_COLOR_WHEEL_SIZE ) );

            for ( int a = 0; a < 3; a++ ) {
                ColorFader ( 1, getColor ( rand() % StripLights_COLOR_WHEEL_SIZE ) );
            }

            break;

        case 'e':
            printf ( "StripLights_WHITE\n" );
            StripLights_DisplayClear ( StripLights_WHITE );
            break;

        case 'f':
            printf ( "StripLights_BLUE\n" );
            StripLights_DisplayClear ( StripLights_BLUE );
            break;

        case 'g':
            printf ( "SingleLEDPingPong\n" );
            SingleLEDPingPong ( 1, 6, getColor ( rand() % 74 ) );

            break;

        case 'h':
            printf ( "Tween1\n" );
            Tween1();

            break;

        case 'i':
            printf ( "ColorFader\n" );
            ColorFader ( 1, getColor ( rand() % StripLights_COLOR_WHEEL_SIZE ) );

            break;

        case 'j':
            printf ( "CandyCane\n" );
            CandyCane ( 40, StripLights_RED, StripLights_WHITE );

            break;

        case 'k':
            printf ( "Snake\n" );
            Snake ( 5 );
            break;

        case 'l':
            printf ( "ColorWheel\n" );

            for ( int a = 0; a < 25; a++ ) {
                ColorWheel ( 10 );
            }

            break;

        case 'm':
            printf ( "CandyCane R\n" );

            for ( int a = 0; a < 2; a++ ) {
                CandyCane ( 5, getColor ( rand() % 74 ), StripLights_WHITE );
            }

            break;

        case 'n':
            led_color c1, c2;

            printf ( "CandyCaneSmooth\n" );

            c1.rgb = 0;
            c2.rgb = 0;

            while ( c1.rgb == c2.rgb ) {
                c1.rgb = getColor ( rand() % StripLights_COLOR_WHEEL_SIZE );
                c2.rgb = getColor ( rand() % StripLights_COLOR_WHEEL_SIZE );
            }

            CandyCaneSmooth ( 5, c1, c2 );

            break;

        case 'o':
            printf ( "Fire2012 h\n" );
            break;

            for ( int a = 0; a < 550; a++ ) {
                Fire2012 ( 0 );
                CyDelay ( 1000 / 60 );
            }

            break;

        case 'p':
            printf ( "Fire2012 c\n" );

            for ( int a = 0; a < 200; a++ ) {
                Fire2012 ( 1 );
                CyDelay ( 1000 / 60 );
            }

            break;

        case 'q':
            printf ( "Fire2012a c\n" );

            for ( int a = 0; a < 200; a++ ) {
                Fire2012a ( 0 );
                CyDelay ( 1000 / 60 );
            }

            break;

        case 'r':
            printf ( "Meet(5)\n" );
            Meet ( 5 );
            break;

        case 's':
            printf ( "Larson1\n" );

            for ( int a = 0; a < 10; a++ ) {
                Larson1 ( StripLights_MAX_X + 1 );
            }

            break;

        case 't':
            printf ( "TweenerALLHSV\n" );
            TweenerALLHSV ( 6, GetRand ( 72 ), GetRand ( 72 ), 2 );
            break;

        case 'u':
            printf ( "TweenerHSV\n" );
            TweenerHSV ( 0, StripLights_MAX_X, GetRand ( 72 ), GetRand ( 72 ), 5, 1 );
            break;

        case 'v':
            printf ( "Sparky\n" );
            Sparky ( 200 );
            break;

        case 'w':
            printf ( "Falloff\n" );
            Falloff ( 100 );
            break;


        case 'x': {

                printf ( "ping pong fader\n" );

                for ( float ii = 1; ii < 128.0f; ii += 15 )
                    for ( float i = 0; i < 3; i += 0.1f ) {
                        float val;
                        val = ( sinf ( i ) * ii );
                        StripLights_DisplayClear ( RGB ( val, ( val ), val ) );
                    }

                for ( int i = 0; i < 100; i++ ) {
                    FadeStrip ( StripLights_MIN_X, StripLights_MAX_X, i );
                    StripLights_Trigger ( 1 );
                }

                break;
            }

        case 'y':

            float val;

            for ( float y = 0; y < StripLights_MAX_X; y++ ) {
                for ( float x = 0; x < StripLights_MAX_X; x += 1.0f ) {
                    val = ( 1 + sinf ( ( 45.0f + ( x + y ) ) / 10 ) ) * 128.0f;
                    StripLights_Pixel ( x, 0, RGB ( val, ( val ), val ) );
                }

                StripLights_Trigger ( 1 );
            }

            for ( int i = 0; i < 100; i++ ) {
                FadeStrip ( StripLights_MIN_X, StripLights_MAX_X, i );
                StripLights_Trigger ( 1 );
            }

            break;

        case ' ':
            printf ( "StripLights_BLACK\n" );
            StripLights_DisplayClear ( 0 );
            break;


        case '8':

            printf ( "PlayFile\n" );
            PlayFile();
            break;

        case '1':
            printf ( "StripLights_RED\n" );
            StripLights_DisplayClear ( StripLights_RED );
            break;

        case '2':
            printf ( "StripLights_GREEN\n" );
            StripLights_DisplayClear ( StripLights_GREEN );
            break;

        case '3':

            printf ( "StripLights_BLUE\n" );
            StripLights_DisplayClear ( StripLights_BLUE );
            break;

        case '0':
            while ( !_kbhit() )
            { StripLights_Trigger ( 1 ); }

            break;

        //brg
        case '4':
            printf ( "StripLights_RED\n" );
            StripLights_DisplayClear ( RGB ( 0, 0xff, 0 ) );
            break;

        case '5':
            printf ( "StripLights_GREEN\n" );
            StripLights_DisplayClear ( RGB ( 0, 0, 0xff ) );
            break;

        case '6':
            printf ( "StripLights_BLUE\n" );
            StripLights_DisplayClear ( RGB ( 0xff, 0, 0 ) );
            break;

        case '7':
            printf ( "StripLights_WHITE\n" );
            StripLights_DisplayClear ( RGB ( 0xff, 0xff, 0xff ) );
            break;

    }
}
// Fire2012 by Mark Kriegsman, July 2012
// as pat of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120


uint8_t qadd8 ( uint8_t i, uint8_t j )
{
    int t = i + j;

    if ( t > 255 ) {
        t = 255;
    }

    return t;
}


// qsub8: subtract one byte from another, saturating at 0x00
uint8_t qsub8 ( uint8_t i, uint8_t j )
{
    int t = i - j;

    if ( t < 0 ) {
        t = 0;
    }

    return t;
}

// scale8: scale one byte by a second one, which is treated as
//         the numerator of a fraction whose denominator is 256
//         In other words, it computes i * (scale / 256)
//         4 clocks AVR, 2 clocks ARM
uint8_t scale8 ( uint8_t i, fract8 scale )
{
    return
        ( ( int ) i * ( int ) ( scale ) ) >> 8;
}


//  The "video" version of scale8 guarantees that the output will
//  be only be zero if one or both of the inputs are zero.  If both
//  inputs are non-zero, the output is guaranteed to be non-zero.
//  This makes for better 'video'/LED dimming, at the cost of
//  several additional cycles.
uint8_t scale8_video ( uint8_t i, fract8 scale )
{
    uint8_t j = ( ( ( int ) i * ( int ) scale ) >> 8 ) + ( ( i && scale ) ? 1 : 0 );
    // uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    // uint8_t j = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
    return j;
}

#define RAND16_2053  2053
#define RAND16_13849 13849
#define RAND16_SEED  1337
uint16_t rand16seed = RAND16_SEED;


uint8_t random8l ( void )
{
    rand16seed = ( rand16seed * RAND16_2053 ) + RAND16_13849;
    return ( uint8_t ) rand16seed;
}

uint16_t random16()
{
    rand16seed = ( rand16seed * RAND16_2053 ) + RAND16_13849;
    return rand16seed;
}


uint8_t random8 ( uint8_t lim )
{
    uint8_t r = random8l();
    r = scale8 ( r, lim );
    return r;
}

uint8_t random8a ( uint8_t min, uint8_t lim )
{
    uint8_t delta = lim - min;
    uint8_t r = random8 ( delta ) + min;
    return r;
}

#define NUM_LEDS    (StripLights_MAX_X)


void Fire2012 ( int cold )
{
    // Array of temperature readings at each simulation cell
    static byte heat[NUM_LEDS];

    // Step 1.  Cool down every cell a little
    for ( int i = 0; i < NUM_LEDS; i++ ) {
        heat[i] = qsub8 ( heat[i],  random8a ( 0, ( ( COOLING * 10 ) / NUM_LEDS ) + 2 ) );
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for ( int k = NUM_LEDS - 1; k >= 2; k-- ) {
        heat[k] = ( heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if ( random8l() < SPARKING ) {
        int y = random8 ( 7 );
        heat[y] = qadd8 ( heat[y], random8a ( 160, 255 ) );
    }

    // Step 4.  Map from heat cells to LED colors
    for ( int j = 0; j < NUM_LEDS; j++ ) {
        if ( cold ) {
            StripLights_Pixel ( j, 0, ColdColor ( heat[j] ).rgb );
        }

        else {
            StripLights_Pixel ( j, 0, HeatColor ( heat[j] ).rgb );
        }
    }

    while ( StripLights_Ready() == 0 );

    StripLights_Trigger ( 1 );
    //CyDelay ( 16 );
}

void Fire2012a ( int cold )
{
    // Array of temperature readings at each simulation cell
    static byte heat[NUM_LEDS];

    // Step 1.  Cool down every cell a little
    for ( int i = 0; i < NUM_LEDS; i++ ) {
        heat[i] = qsub8 ( heat[i],  random8a ( 0, ( ( COOLING * 10 ) / NUM_LEDS ) + 2 ) );
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for ( int k = NUM_LEDS - 3; k > 0; k-- ) {
        heat[k] = ( heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if ( random8l() < SPARKING ) {
        int y = random8 ( 7 );
        heat[y] = qadd8 ( heat[y], random8a ( 160, 255 ) );
    }


    // Step 4.  Map from heat cells to LED colors
    for ( int j = 0; j < NUM_LEDS; j++ ) {
        if ( cold ) {
            StripLights_Pixel ( j, 0, ColdColor ( heat[j] ).rgb );
        }

        else {
            StripLights_Pixel ( j, 0, HeatColor ( heat[j] ).rgb );
        }
    }

    while ( StripLights_Ready() == 0 );

    StripLights_Trigger ( 1 );
    //CyDelay ( 16 );
}
