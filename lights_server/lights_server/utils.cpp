#include "stdafx.h"


led_color hsv_to_rgb ( hsv_color hsv )
{
	led_color rgb;
	unsigned char region, p, q, t;
	uint16_t h, s, v, remainder;

	//grey
	if ( hsv.h.s == 0 ) {
		rgb.c.r = hsv.h.v;
		rgb.c.g = hsv.h.v;
		rgb.c.b = hsv.h.v;
		return rgb;
	}

	// converting to 16 bit to prevent overflow
	h = hsv.h.h;
	s = hsv.h.s;
	v = hsv.h.v;

	region = h / 43;
	remainder = ( h - ( region * 43 ) ) * 6;

	p = ( v * ( 255 - s ) ) >> 8;
	q = ( v * ( 255 - ( ( s * remainder ) >> 8 ) ) ) >> 8;
	t = ( v * ( 255 - ( ( s * ( 255 - remainder ) ) >> 8 ) ) ) >> 8;

	switch ( region ) {
	case 0:
		rgb.c.r = ( uint8_t ) v;
		rgb.c.g = t;
		rgb.c.b = p;
		break;

	case 1:
		rgb.c.r = q;
		rgb.c.g = ( uint8_t ) v;
		rgb.c.b = p;
		break;

	case 2:
		rgb.c.r = p;
		rgb.c.g = ( uint8_t ) v;
		rgb.c.b = t;
		break;

	case 3:
		rgb.c.r = p;
		rgb.c.g = q;
		rgb.c.b = ( uint8_t ) v;
		break;

	case 4:
		rgb.c.r = t;
		rgb.c.g = p;
		rgb.c.b = ( uint8_t ) v;
		break;

	default:
		rgb.c.r = ( uint8_t ) v;
		rgb.c.g = p;
		rgb.c.b = q;
		break;
	}

	return rgb;
}

uint32_t TweenC1toC2 ( led_color c1, led_color c2,int amount )
{
	led_color result;
	float percent ,r,g,b;

	result.rgb = 0;

	if ( amount < 0 ) {
		amount = 0;
	}

	if ( amount > 100 ) {
		amount = 100 ;
	}

	percent = ( float ) amount/100.0f;

	/// mix two colours, simple linear interpolate , won't work well for all colour transitions
	r = ( c1.c.r + ( c2.c.r - c1.c.r ) * percent ) ;
	g = ( c1.c.g + ( c2.c.g - c1.c.g ) * percent ) ;
	b = ( c1.c.b + ( c2.c.b - c1.c.b ) * percent ) ;

	result.c.r = ( uint8_t ) r;
	result.c.g = ( uint8_t ) g;
	result.c.b = ( uint8_t ) b;


	return result.rgb;
}

void Tween1 ( void )
{
	hsv_color tween;
	static led_color rgb,src,tmp;
	static hsv_color result ;

	src.c.r = rand() %255;
	src.c.g = rand() %255;
	src.c.b = rand() %255;

	tmp.rgb = getColor ( rand() %StripLights_COLOR_WHEEL_SIZE );

	tween = rgb_to_hsv ( tmp );

	result.hsv = TweenerHSV (
	                 0,
					 StripLights_COLUMNS,
	                 tmp.rgb,
	                 tween.hsv,
	                 10
	                 ,1 );

	// Tweener( 100,src.rgb );

	src.c.r += 5- ( rand() %10 );
	src.c.g += 5- ( rand() %10 );
	src.c.b += 5- ( rand() %10 );

	result.hsv = TweenerHSV (
					StripLights_COLUMNS,
					 StripLights_COLUMNS,
	                 result.hsv,
	                 tween.hsv,
	                 10
	                 ,-1
	             );

}

uint8_t TweenU8toU8 ( uint8_t source, uint8_t target,int amount )
{
	float percent,temp;

	percent = ( float ) amount/100.0f;

	temp = ( source + ( target - source ) * percent ) ;

	return ( uint8_t ) temp;

}

hsv_color rgb_to_hsv ( led_color rgb )
{
	hsv_color hsv;
	unsigned char rgbMin, rgbMax;

	rgbMin = rgb.c.r < rgb.c.g ? ( rgb.c.r < rgb.c.b ? rgb.c.r : rgb.c.b ) : ( rgb.c.g < rgb.c.b ? rgb.c.g : rgb.c.b );
	rgbMax = rgb.c.r > rgb.c.g ? ( rgb.c.r > rgb.c.b ? rgb.c.r : rgb.c.b ) : ( rgb.c.g > rgb.c.b ? rgb.c.g : rgb.c.b );

	hsv.h.v = rgbMax;

	if ( hsv.h.v == 0 ) {
		hsv.h.h = 0;
		hsv.h.s = 0;
		return hsv;
	}

	hsv.h.s = 255 * ( ( long ) ( rgbMax - rgbMin ) ) / hsv.h.v;

	if ( hsv.h.s == 0 ) {
		hsv.h.h = 0;
		return hsv;
	}

	if ( rgbMax == rgb.c.r ) {
		hsv.h.h = 0 + 43 * ( rgb.c.g - rgb.c.b ) / ( rgbMax - rgbMin );
	}

	else if ( rgbMax == rgb.c.g ) {
		hsv.h.h = 85 + 43 * ( rgb.c.b - rgb.c.r ) / ( rgbMax - rgbMin );
	}

	else {
		hsv.h.h = 171 + 43 * ( rgb.c.r - rgb.c.g ) / ( rgbMax - rgbMin );
	}

	return hsv;
}


// CRGB HeatColor( uint8_t temperature)
//
// Approximates a 'black body radiation' spectrum for
// a given 'heat' level.  This is useful for animations of 'fire'.
// Heat is specified as an arbitrary scale from 0 (cool) to 255 (hot).
// This is NOT a chromatically correct 'black body radiation'
// spectrum, but it's surprisingly close, and it's fast and small.
//
// On AVR/Arduino, this typically takes around 70 bytes of program memory,
// versus 768 bytes for a full 256-entry RGB lookup table.

led_color HeatColor ( uint8_t temperature )
{
	led_color heatcolor;

	// Scale 'heat' down from 0-255 to 0-191,
	// which can then be easily divided into three
	// equal 'thirds' of 64 units each.
	uint8_t t192 = scale8_video ( temperature, 192 );

	// calculate a value that ramps up from
	// zero to 255 in each 'third' of the scale.
	uint8_t heatramp = t192 & 0x3F; // 0..63
	heatramp <<= 2; // scale up to 0..252

	// now figure out which third of the spectrum we're in:
	if ( t192 & 0x80 ) {
		// we're in the hottest third
		heatcolor.c.r = 255; // full red
		heatcolor.c.g = 255; // full green
		heatcolor.c.b = heatramp; // ramp up blue

	}

	else if ( t192 & 0x40 ) {
		// we're in the middle third
		heatcolor.c.r = 255; // full red
		heatcolor.c.g = heatramp; // ramp up green
		heatcolor.c.b = 0; // no blue

	}

	else {
		// we're in the coolest third
		heatcolor.c.r = heatramp; // ramp up red
		heatcolor.c.g = 0; // no green
		heatcolor.c.b = 0; // no blue
	}

	return heatcolor;
}

led_color ColdColor ( uint8_t temperature )
{
	led_color heatcolor;

	// Scale 'heat' down from 0-255 to 0-191,
	// which can then be easily divided into three
	// equal 'thirds' of 64 units each.
	uint8_t t192 = scale8_video ( temperature, 192 );

	// calculate a value that ramps up from
	// zero to 255 in each 'third' of the scale.
	uint8_t heatramp = t192 & 0x3F; // 0..63
	heatramp <<= 2; // scale up to 0..252

	// now figure out which third of the spectrum we're in:
	if ( t192 & 0x80 ) {
		// we're in the hottest third
		heatcolor.c.r = 255; // full red
		heatcolor.c.g = 255; // full green
		heatcolor.c.b = heatramp; // ramp up blue

	}

	else if ( t192 & 0x40 ) {
		// we're in the middle third
		heatcolor.c.r = 0; // full red
		heatcolor.c.g = 128; // ramp up green
		heatcolor.c.b = heatramp; // no blue

	}

	else {
		// we're in the coolest third
		heatcolor.c.r = 0; // ramp up red
		heatcolor.c.g = 0; // no green
		heatcolor.c.b = heatramp; // no blue
	}

	return heatcolor;
}
