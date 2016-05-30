/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include <stdio.h>
#include <stdlib.h>
#include <device.h>

#include <effects.h>


led_color hsv_to_rgb(hsv_color hsv)
{
    led_color rgb;
    unsigned char region, p, q, t;
    uint16_t h, s, v, remainder;

	//grey
    if (hsv.h.s == 0)
    {
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
    remainder = (h - (region * 43)) * 6; 

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.c.r = v;
            rgb.c.g = t;
            rgb.c.b = p;
            break;
        case 1:
            rgb.c.r = q;
            rgb.c.g = v;
            rgb.c.b = p;
            break;
        case 2:
            rgb.c.r = p;
            rgb.c.g = v;
            rgb.c.b = t;
            break;
        case 3:
            rgb.c.r = p;
            rgb.c.g = q;
            rgb.c.b = v;
            break;
        case 4:
            rgb.c.r = t;
            rgb.c.g = p;
            rgb.c.b = v;
            break;
        default:
            rgb.c.r = v;
            rgb.c.g = p;
            rgb.c.b = q;
            break;
    }

    return rgb;
}

hsv_color rgb_to_hsv(led_color rgb)
{
    hsv_color hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.c.r < rgb.c.g ? (rgb.c.r < rgb.c.b ? rgb.c.r : rgb.c.b) : (rgb.c.g < rgb.c.b ? rgb.c.g : rgb.c.b);
    rgbMax = rgb.c.r > rgb.c.g ? (rgb.c.r > rgb.c.b ? rgb.c.r : rgb.c.b) : (rgb.c.g > rgb.c.b ? rgb.c.g : rgb.c.b);

    hsv.h.v = rgbMax;
    if (hsv.h.v == 0)
    {
        hsv.h.h = 0;
        hsv.h.s = 0;
        return hsv;
    }

    hsv.h.s = 255 * ((long)(rgbMax - rgbMin)) / hsv.h.v;
    if (hsv.h.s == 0)
    {
        hsv.h.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.c.r)
        hsv.h.h = 0 + 43 * (rgb.c.g - rgb.c.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.c.g)
        hsv.h.h = 85 + 43 * (rgb.c.b - rgb.c.r) / (rgbMax - rgbMin);
    else
        hsv.h.h = 171 + 43 * (rgb.c.r - rgb.c.g) / (rgbMax - rgbMin);

    return hsv;
}

uint8 TweenU8toU8(uint8 source, uint8 target,int amount)
{
    float percent,temp;
	
	percent = (float)amount/100.0f;
    
	temp = (source + (target - source) * percent ) ;
	
	return temp;

}

uint32 TweenC1toC2(led_color c1, led_color c2,int amount)
{
    led_color result;
    float percent ,r,g,b;
    
	if ( amount < 0 ) amount = 0;
	if (amount > 100 ) amount = 100 ;
	
    percent = (float)amount/100.0f;
    
    /// mix two colours, simple linear interpolate , won't work well for all colour transitions
    r = (c1.c.r + ( c2.c.r - c1.c.r ) * percent ) ;
    g = (c1.c.g + ( c2.c.g - c1.c.g ) * percent ) ;
    b = (c1.c.b + ( c2.c.b - c1.c.b ) * percent ) ;
	
	result.c.r = r;
	result.c.g = g;
	result.c.b = b;
    if ( 0 ) {
		char buffer[1024];
		sprintf(buffer,"%d) %02x %02x %02x  %02x %02x %02x  = %02x %02x %02x\r\n\n",amount,
												c1.c.r, c1.c.g, c1.c.b, 
												c2.c.r, c2.c.g, c2.c.b, 
												result.c.r,result.c.g,result.c.b);
		UART_UartPutString( buffer );
	}      
    return result.rgb;
}

int TweenC1toC2Range( uint16_t count, uint16_t x, uint32 source, uint32 target)
{
    int i,tx=0;
    led_color result,c1,c2;
    float percent ,r,g,b;
    c1.rgb = source;
    c2.rgb = target;
    
    // i * diff = 100% for steps
    
    float diff = 1.0f / count ;
    
    for ( i = 0 ; i < count ; i++ ) {
        
        percent = (float)i* diff;;
        
        /// mix two colours, simple linear interpolate , won't work well for all colour transitions
        r = (c1.c.r + ( c2.c.r - c1.c.r ) * percent ) ;
        g = (c1.c.g + ( c2.c.g - c1.c.g ) * percent ) ;
        b = (c1.c.b + ( c2.c.b - c1.c.b ) * percent ) ;

		
		result.c.r = r;
		result.c.g = g;
		result.c.b = b;
		
        // set color at pixel
		tx =x +i;
		tx%= StripLights_COLUMNS;
       StripLights_Pixel( tx,0,result.rgb);
    
       BOOT_CHECK();
       
    }
    return tx;
}

uint32 AddColor( led_color c1, led_color c2 )
{
	
	return TweenC1toC2(c1,c2,50);
	
}

#ifdef USE_FLOAT_APPROX
    
// we can go smaller using one of the computed orders
// CMSIS table approx
static const float32_t sinTable[259] = {
  -0.024541229009628296f, 0.000000000000000000f, 0.024541229009628296f,
  0.049067676067352295f, 0.073564566671848297f, 0.098017141222953796f,
  0.122410677373409270f, 0.146730467677116390f,
  0.170961886644363400f, 0.195090323686599730f, 0.219101235270500180f,
  0.242980182170867920f, 0.266712754964828490f, 0.290284663438797000f,
  0.313681751489639280f, 0.336889863014221190f,
  0.359895050525665280f, 0.382683426141738890f, 0.405241310596466060f,
  0.427555084228515630f, 0.449611335992813110f, 0.471396744251251220f,
  0.492898195981979370f, 0.514102756977081300f,
  0.534997642040252690f, 0.555570244789123540f, 0.575808167457580570f,
  0.595699310302734380f, 0.615231573581695560f, 0.634393274784088130f,
  0.653172850608825680f, 0.671558976173400880f,
  0.689540565013885500f, 0.707106769084930420f, 0.724247097969055180f,
  0.740951120853424070f, 0.757208824157714840f, 0.773010432720184330f,
  0.788346409797668460f, 0.803207516670227050f,
  0.817584812641143800f, 0.831469595432281490f, 0.844853579998016360f,
  0.857728600502014160f, 0.870086967945098880f, 0.881921291351318360f,
  0.893224298954010010f, 0.903989315032958980f,
  0.914209783077239990f, 0.923879504203796390f, 0.932992815971374510f,
  0.941544055938720700f, 0.949528157711029050f, 0.956940352916717530f,
  0.963776051998138430f, 0.970031261444091800f,
  0.975702106952667240f, 0.980785250663757320f, 0.985277652740478520f,
  0.989176511764526370f, 0.992479562759399410f, 0.995184719562530520f,
  0.997290432453155520f, 0.998795449733734130f,
  0.999698817729949950f, 1.000000000000000000f, 0.999698817729949950f,
  0.998795449733734130f, 0.997290432453155520f, 0.995184719562530520f,
  0.992479562759399410f, 0.989176511764526370f,
  0.985277652740478520f, 0.980785250663757320f, 0.975702106952667240f,
  0.970031261444091800f, 0.963776051998138430f, 0.956940352916717530f,
  0.949528157711029050f, 0.941544055938720700f,
  0.932992815971374510f, 0.923879504203796390f, 0.914209783077239990f,
  0.903989315032958980f, 0.893224298954010010f, 0.881921291351318360f,
  0.870086967945098880f, 0.857728600502014160f,
  0.844853579998016360f, 0.831469595432281490f, 0.817584812641143800f,
  0.803207516670227050f, 0.788346409797668460f, 0.773010432720184330f,
  0.757208824157714840f, 0.740951120853424070f,
  0.724247097969055180f, 0.707106769084930420f, 0.689540565013885500f,
  0.671558976173400880f, 0.653172850608825680f, 0.634393274784088130f,
  0.615231573581695560f, 0.595699310302734380f,
  0.575808167457580570f, 0.555570244789123540f, 0.534997642040252690f,
  0.514102756977081300f, 0.492898195981979370f, 0.471396744251251220f,
  0.449611335992813110f, 0.427555084228515630f,
  0.405241310596466060f, 0.382683426141738890f, 0.359895050525665280f,
  0.336889863014221190f, 0.313681751489639280f, 0.290284663438797000f,
  0.266712754964828490f, 0.242980182170867920f,
  0.219101235270500180f, 0.195090323686599730f, 0.170961886644363400f,
  0.146730467677116390f, 0.122410677373409270f, 0.098017141222953796f,
  0.073564566671848297f, 0.049067676067352295f,
  0.024541229009628296f, 0.000000000000000122f, -0.024541229009628296f,
  -0.049067676067352295f, -0.073564566671848297f, -0.098017141222953796f,
  -0.122410677373409270f, -0.146730467677116390f,
  -0.170961886644363400f, -0.195090323686599730f, -0.219101235270500180f,
  -0.242980182170867920f, -0.266712754964828490f, -0.290284663438797000f,
  -0.313681751489639280f, -0.336889863014221190f,
  -0.359895050525665280f, -0.382683426141738890f, -0.405241310596466060f,
  -0.427555084228515630f, -0.449611335992813110f, -0.471396744251251220f,
  -0.492898195981979370f, -0.514102756977081300f,
  -0.534997642040252690f, -0.555570244789123540f, -0.575808167457580570f,
  -0.595699310302734380f, -0.615231573581695560f, -0.634393274784088130f,
  -0.653172850608825680f, -0.671558976173400880f,
  -0.689540565013885500f, -0.707106769084930420f, -0.724247097969055180f,
  -0.740951120853424070f, -0.757208824157714840f, -0.773010432720184330f,
  -0.788346409797668460f, -0.803207516670227050f,
  -0.817584812641143800f, -0.831469595432281490f, -0.844853579998016360f,
  -0.857728600502014160f, -0.870086967945098880f, -0.881921291351318360f,
  -0.893224298954010010f, -0.903989315032958980f,
  -0.914209783077239990f, -0.923879504203796390f, -0.932992815971374510f,
  -0.941544055938720700f, -0.949528157711029050f, -0.956940352916717530f,
  -0.963776051998138430f, -0.970031261444091800f,
  -0.975702106952667240f, -0.980785250663757320f, -0.985277652740478520f,
  -0.989176511764526370f, -0.992479562759399410f, -0.995184719562530520f,
  -0.997290432453155520f, -0.998795449733734130f,
  -0.999698817729949950f, -1.000000000000000000f, -0.999698817729949950f,
  -0.998795449733734130f, -0.997290432453155520f, -0.995184719562530520f,
  -0.992479562759399410f, -0.989176511764526370f,
  -0.985277652740478520f, -0.980785250663757320f, -0.975702106952667240f,
  -0.970031261444091800f, -0.963776051998138430f, -0.956940352916717530f,
  -0.949528157711029050f, -0.941544055938720700f,
  -0.932992815971374510f, -0.923879504203796390f, -0.914209783077239990f,
  -0.903989315032958980f, -0.893224298954010010f, -0.881921291351318360f,
  -0.870086967945098880f, -0.857728600502014160f,
  -0.844853579998016360f, -0.831469595432281490f, -0.817584812641143800f,
  -0.803207516670227050f, -0.788346409797668460f, -0.773010432720184330f,
  -0.757208824157714840f, -0.740951120853424070f,
  -0.724247097969055180f, -0.707106769084930420f, -0.689540565013885500f,
  -0.671558976173400880f, -0.653172850608825680f, -0.634393274784088130f,
  -0.615231573581695560f, -0.595699310302734380f,
  -0.575808167457580570f, -0.555570244789123540f, -0.534997642040252690f,
  -0.514102756977081300f, -0.492898195981979370f, -0.471396744251251220f,
  -0.449611335992813110f, -0.427555084228515630f,
  -0.405241310596466060f, -0.382683426141738890f, -0.359895050525665280f,
  -0.336889863014221190f, -0.313681751489639280f, -0.290284663438797000f,
  -0.266712754964828490f, -0.242980182170867920f,
  -0.219101235270500180f, -0.195090323686599730f, -0.170961886644363400f,
  -0.146730467677116390f, -0.122410677373409270f, -0.098017141222953796f,
  -0.073564566671848297f, -0.049067676067352295f,
  -0.024541229009628296f, -0.000000000000000245f, 0.024541229009628296f
};


/**
 * @brief  Fast approximation to the trigonometric sine function for floating-point data.
 * @param[in] x input value in radians.
 * @return  sin(x).
 */

float32_t arm_sin_f32( float32_t x)
{
  float32_t sinVal, fract, in;                   /* Temporary variables for input, output */
  int32_t index;                                 /* Index variable */
  uint32_t tableSize = (uint32_t) TABLE_SIZE;    /* Initialise tablesize */
  float32_t wa, wb, wc, wd;                      /* Cubic interpolation coefficients */
  float32_t a, b, c, d;                          /* Four nearest output values */
  float32_t *tablePtr;                           /* Pointer to table */
  int32_t n;
  float32_t fractsq, fractby2, fractby6, fractby3, fractsqby2;
  float32_t oneminusfractby2;
  float32_t frby2xfrsq, frby6xfrsq;

  /* input x is in radians */
  /* Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi */
  in = x * 0.159154943092f;

  /* Calculation of floor value of input */
  n = (int32_t) in;

  /* Make negative values towards -infinity */
  if(x < 0.0f)
  {
    n = n - 1;
  }

  /* Map input value to [0 1] */
  in = in - (float32_t) n;

  /* Calculation of index of the table */
  index = (uint32_t) (tableSize * in);

  /* fractional value calculation */
  fract = ((float32_t) tableSize * in) - (float32_t) index;

  /* Checking min and max index of table */
  if(index < 0)
  {
    index = 0;
  }
  else if(index > 256)
  {
    index = 256;
  }

  /* Initialise table pointer */
  tablePtr = (float32_t *) & sinTable[index];

  /* Read four nearest values of input value from the sin table */
  a = tablePtr[0];
  b = tablePtr[1];
  c = tablePtr[2];
  d = tablePtr[3];

  /* Cubic interpolation process */
  fractsq = fract * fract;
  fractby2 = fract * 0.5f;
  fractby6 = fract * 0.166666667f;
  fractby3 = fract * 0.3333333333333f;
  fractsqby2 = fractsq * 0.5f;
  frby2xfrsq = (fractby2) * fractsq;
  frby6xfrsq = (fractby6) * fractsq;
  oneminusfractby2 = 1.0f - fractby2;
  wb = fractsqby2 - fractby3;
  wc = (fractsqby2 + fract);
  wa = wb - frby6xfrsq;
  wb = frby2xfrsq - fractsq;
  sinVal = wa * a;
  wc = wc - frby2xfrsq;
  wd = (frby6xfrsq) - fractby6;
  wb = wb + oneminusfractby2;

  /* Calculate sin value */
  sinVal = (sinVal + (b * wb)) + ((c * wc) + (d * wd));

  /* Return the output value */
  return (sinVal);

}

#else
    
//fxmath
    
#define  fix16_min ( 0x80000000)

    
static inline fix16_t fix16_from_float(float a)
{
    float temp = a * fix16_one;
    return (fix16_t)temp;
}

/* 32-bit implementation of fix16_mul. Potentially fast on 16-bit processors,
 * and this is a relatively good compromise for compilers that do not support
 * uint64_t. Uses 16*16->32bit multiplications.
 */

fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1)
{
  // Each argument is divided to 16-bit parts.
  //          AB
  //      *   CD
  // -----------
  //          BD  16 * 16 -> 32 bit products
  //         CB
  //         AD
  //        AC
  //       |----| 64 bit product
  int32_t A = (inArg0 >> 16), C = (inArg1 >> 16);
  uint32_t B = (inArg0 & 0xFFFF), D = (inArg1 & 0xFFFF);
  
  int32_t AC = A*C;
  int32_t AD_CB = A*D + C*B;
  const uint32_t BD = B*D;
  
  int32_t product_hi = AC + (AD_CB >> 16);
  
  // Handle carry from lower 32 bits to upper part of result.
  uint32_t ad_cb_temp = AD_CB << 16;
  uint32_t product_lo = BD + ad_cb_temp;
  if (product_lo < BD)
    product_hi++;

  return (product_hi << 16) | (product_lo >> 16);
}

unsigned char __clz(unsigned int val);

fix16_t fix16_div(fix16_t a, fix16_t b)
{
  // This uses a hardware 32/32 bit division multiple times, until we have
  // computed all the bits in (a<<17)/b. Usually this takes 1-3 iterations.
  
  if (b == 0)
      return fix16_min;
  
  uint32_t remainder = (a >= 0) ? a : (-a);
  uint32_t divider = (b >= 0) ? b : (-b);
  uint32_t quotient = 0;
  int bit_pos = 17;
  
  // Kick-start the division a bit.
  // This improves speed in the worst-case scenarios where N and D are large
  // It gets a lower estimate for the result by N/(D >> 17 + 1).
  if (divider & 0xFFF00000)
  {
    uint32_t shifted_div = ((divider >> 17) + 1);
    quotient = remainder / shifted_div;
    remainder -= ((uint64_t)quotient * divider) >> 17;
  }
  
  // If the divider is divisible by 2^n, take advantage of it.
  while (!(divider & 0xF) && bit_pos >= 4)
  {
    divider >>= 4;
    bit_pos -= 4;
  }
  
  while (remainder && bit_pos >= 0)
  {
    // Shift remainder as much as we can without overflowing
    int shift = __clz(remainder);
    if (shift > bit_pos) shift = bit_pos;
    remainder <<= shift;
    bit_pos -= shift;
    
    uint32_t div = remainder / divider;
    remainder = remainder % divider;
    quotient += div << bit_pos;

    remainder <<= 1;
    bit_pos--;
  }
  

  
  fix16_t result = quotient >> 1;
  
  // Figure out the sign of the result
  if ((a ^ b) & 0x80000000)
  {

    
    result = -result;
  }
  
  return result;
}

fix16_t fix16_sin(fix16_t inAngle) 
{

    fix16_t tempAngle = inAngle % (fix16_pi << 1);


	if(tempAngle > fix16_pi)
		tempAngle -= (fix16_pi << 1);
	else if(tempAngle < -fix16_pi)
		tempAngle += (fix16_pi << 1);

	fix16_t tempAngleSq = fix16_mul(tempAngle, tempAngle);

	fix16_t tempOut;
	tempOut = fix16_mul(-13, tempAngleSq) + 546;
	tempOut = fix16_mul(tempOut, tempAngleSq) - 10923;
	tempOut = fix16_mul(tempOut, tempAngleSq) + 65536;
	tempOut = fix16_mul(tempOut, tempAngle);


	return tempOut;
}
#endif

