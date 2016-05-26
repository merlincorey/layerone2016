#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <mmiscapi2.h>

static WNDCLASS wc = {0};
void CALLBACK UpdateSpectrum ( UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 );

static HDC specdc = 0;
static HBITMAP specbmp = 0;
static BYTE *specbuf;
DWORD timer = 0;
DWORD chan = 0;

static HWND win;
static char tmp_buffer[256];

int specmode = 0, specpos = 0; // spectrum mode (and marker pos for 3D mode)
VOID CALLBACK TimerProc ( HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime )
{
    SetWindowTextA ( ( HWND ) win, tmp_buffer );
}


UINT  message_pump ( LPVOID pParam )

{
    MSG  msg;

    HWND tmpwnd;
    tmpwnd = CreateWindow (
                 L"Light Server",
                 L"Audio Display",
                 WS_POPUPWINDOW | WS_CAPTION | WS_VISIBLE, 200, 200,
                 SPECWIDTH + 2 * GetSystemMetrics ( SM_CXDLGFRAME ),
                 SPECHEIGHT + GetSystemMetrics ( SM_CYCAPTION ) + 2 * GetSystemMetrics ( SM_CYDLGFRAME ),
                 NULL, NULL, GetModuleHandle ( NULL ), NULL )  ;


    if (  ! tmpwnd  ) {
        printf ( "Can't create window %d\n" , GetLastError() );
        return 0;
    }

    ShowWindow ( win, SW_SHOWNORMAL );

    UINT TimerId = SetTimer ( win, 0, 100, &TimerProc );


    while ( GetMessage ( &msg, win, 0, 0 ) > 0 ) {
        TranslateMessage ( &msg );
        DispatchMessage ( &msg );
    }

    return 0;
}

// window procedure
LRESULT CALLBACK SpectrumWindowProc ( HWND h, UINT m, WPARAM w, LPARAM l )
{
    switch ( m ) {
        case WM_PAINT:
            if ( GetUpdateRect ( h, 0, 0 ) ) {
                PAINTSTRUCT p;
                HDC dc;

                if ( ! ( dc = BeginPaint ( h, &p ) ) ) {
                    return 0;
                }

                BitBlt ( dc, 0, 0, SPECWIDTH, SPECHEIGHT, specdc, 0, 0, SRCCOPY );
                EndPaint ( h, &p );
            }

            return 0;

        case WM_LBUTTONUP:
            specmode = ( specmode + 1 ) % 4; // change spectrum mode
            memset ( specbuf, 0, SPECWIDTH * SPECHEIGHT ); // clear display
            return 0;

        case WM_CREATE:
            win = h;

            // initialize BASS
            if ( !BASS_Init ( -1, 44100, 0, win, NULL ) ) {
                printf ( "Can't initialize device" );
                return -1;
            }

            {
                // create bitmap to draw spectrum in (8 bit for easy updating)
                BYTE data[2000] = {0};
                BITMAPINFOHEADER *bh = ( BITMAPINFOHEADER* ) data;
                RGBQUAD *pal = ( RGBQUAD* ) ( data + sizeof ( *bh ) );
                int a;
                bh->biSize = sizeof ( *bh );
                bh->biWidth = SPECWIDTH;
                bh->biHeight = SPECHEIGHT; // upside down (line 0=bottom)
                bh->biPlanes = 1;
                bh->biBitCount = 8;
                bh->biClrUsed = bh->biClrImportant = 256;

                // setup palette
                for ( a = 1; a < 128; a++ ) {
                    pal[a].rgbGreen = 256 - 2 * a;
                    pal[a].rgbRed = 2 * a;
                }

                for ( a = 0; a < 32; a++ ) {
                    pal[128 + a].rgbBlue = 8 * a;
                    pal[128 + 32 + a].rgbBlue = 255;
                    pal[128 + 32 + a].rgbRed = 8 * a;
                    pal[128 + 64 + a].rgbRed = 255;
                    pal[128 + 64 + a].rgbBlue = 8 * ( 31 - a );
                    pal[128 + 64 + a].rgbGreen = 8 * a;
                    pal[128 + 96 + a].rgbRed = 255;
                    pal[128 + 96 + a].rgbGreen = 255;
                    pal[128 + 96 + a].rgbBlue = 8 * a;
                }

                // create the bitmap
                specbmp = CreateDIBSection ( 0, ( BITMAPINFO* ) bh, DIB_RGB_COLORS, ( void** ) &specbuf, NULL, 0 );
                specdc = CreateCompatibleDC ( 0 );
                SelectObject ( specdc, specbmp );
            }

            // start update timer (40hz)
            timer = timeSetEvent ( 25, 25, ( LPTIMECALLBACK ) &UpdateSpectrum, 0, TIME_PERIODIC );
            break;

        case WM_DESTROY:
            if ( timer ) {
                timeKillEvent ( timer );
            }

            BASS_Free();

            if ( specdc ) {
                DeleteDC ( specdc );
            }

            if ( specbmp ) {
                DeleteObject ( specbmp );
            }

            PostQuitMessage ( 0 );
            break;
    }

    return DefWindowProc ( h, m, w, l );
}

DWORD CALLBACK WasapiProc ( void *buffer, DWORD length, void *user )
{
    return 1; // continue
}
int devnum = -1;

int InitBASS ( HINSTANCE hInstance )
{

    // register window class and create the window
    wc.lpfnWndProc = SpectrumWindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor ( NULL, IDC_ARROW );
    wc.lpszClassName = L"light server";

    if ( !RegisterClass ( &wc ) ) {
        printf ( "Can't register class %d\n" , GetLastError() );
        return 0;

    }

    AfxBeginThread ( message_pump , 0 );

    // find the loopback device for the default output

    BASS_WASAPI_DEVICEINFO info;

    for ( int a = 0; BASS_WASAPI_GetDeviceInfo ( a, &info ); a++ ) {
        if ( ! ( info.flags & BASS_DEVICE_INPUT ) // found an output device (not input)
                && ( info.flags & BASS_DEVICE_DEFAULT ) ) { // and it is the default
            devnum = a + 1; // use it (+1 because the next device is the corresponding loopback device)
            break;
        }
    }

    if ( devnum >= 0 ) {
        BASS_WASAPI_Init ( devnum, 0, 0, BASS_WASAPI_BUFFER, 1.0, 0, WasapiProc, NULL ); // initialize the device with buffering enabled
    }

    BASS_WASAPI_Start();



    // do something with the FFT data
    return 1;
}
//	KillTimer ( NULL, TimerId );
// select a file to play, and play it
DWORD PlayFile ( void )
{

    TCHAR file[MAX_PATH] = L"";
    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof ( ofn );
    ofn.hwndOwner = NULL;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFile = file;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
    ofn.lpstrTitle = L"Select a file to play";
    ofn.lpstrFilter = L"playable files\0*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.umx;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif\0All files\0*.*\0\0";

    if ( chan ) {
        BASS_ChannelStop ( chan );
    }

    if ( !GetOpenFileName ( &ofn ) ) {
        return FALSE;
    }

    if ( ! ( chan = BASS_StreamCreateFile ( FALSE, file, 0, 0, BASS_SAMPLE_LOOP ) )
            && ! ( chan = BASS_MusicLoad ( FALSE, file, 0, 0, BASS_MUSIC_RAMP | BASS_SAMPLE_LOOP, 1 ) ) ) {
        return 0; // Can't load the file
    }

    BASS_ChannelPlay ( chan, FALSE );

    return chan;

}

static void audio_ColorWheel()
{
    static int i = 0xAAA, x;
    static uint32_t color;
    static uint32_t startColor;
    int count = 20;

    if ( i >= count ) {
        i = 0;
    }

    if ( i == 0xaaa ) {
        i = 0;
        color = 0;
        startColor = 0;
    }

    //for ( ; i < count; i++ )
    {
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

    }
    i++;
}


DWORD PlayFile ( const char *file )
{
    DWORD chan;

    if ( ! ( chan = BASS_StreamCreateFile ( FALSE, file, 0, 0, BASS_SAMPLE_LOOP ) )
            && ! ( chan = BASS_MusicLoad ( FALSE, file, 0, 0, BASS_MUSIC_RAMP | BASS_SAMPLE_LOOP, 1 ) ) ) {
        return 0; // Can't load the file
    }

    BASS_ChannelPlay ( chan, FALSE );

    return chan;
}


// update the spectrum display - the interesting bit :)
void CALLBACK UpdateSpectrum ( UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 )
{
    HDC dc;
    int x;
    long y, y1;
    float level;

    float fft[1024];



    if ( chan ) {



        if ( BASS_ChannelIsActive ( chan ) != BASS_ACTIVE_PLAYING ) { return; }

        if ( specmode == 3 ) { // waveform
            int c;
            float *buf;
            BASS_CHANNELINFO ci;
            memset ( specbuf, 0, SPECWIDTH * SPECHEIGHT );
            BASS_ChannelGetInfo ( chan, &ci ); // get number of channels
            buf = ( float* ) alloca ( ci.chans * SPECWIDTH * sizeof ( float ) ); // allocate buffer for data
            BASS_ChannelGetData ( chan, buf, ( ci.chans * SPECWIDTH * sizeof ( float ) ) | BASS_DATA_FLOAT ); // get the sample data (floating-point to avoid 8 & 16 bit processing)

            for ( c = 0; c < ci.chans; c++ ) {
                for ( x = 0; x < SPECWIDTH; x++ ) {
                    int v = ( 1 - buf[x * ci.chans + c] ) * SPECHEIGHT / 2; // invert and scale to fit display

                    if ( v < 0 ) {
                        v = 0;
                    }

                    else
                        if ( v >= SPECHEIGHT ) {
                            v = SPECHEIGHT - 1;
                        }

                    if ( !x ) {
                        y = v;
                    }

                    do { // draw line from previous sample...
                        if ( y < v ) {
                            y++;
                        }

                        else
                            if ( y > v ) {
                                y--;
                            }

                        specbuf[y * SPECWIDTH + x] = c & 1 ? 127 : 1; // left=green, right=red (could add more colours to palette for more chans)
                    } while ( y != v );
                }
            }

        } else {

            if ( chan ) {
                BASS_ChannelGetData ( chan, fft, BASS_DATA_FFT2048 );
            } // get the FFT data

            if ( !specmode ) {
                // "normal" FFT
                memset ( specbuf, 0, SPECWIDTH * SPECHEIGHT );

                for ( x = 0; x < SPECWIDTH / 2; x++ ) {
#if 0
                    y = sqrt ( fft[x + 1] ) * 3 * SPECHEIGHT - 4; // scale it (sqrt to make low values more visible)
#else
                    y = fft[x + 1] * 10 * SPECHEIGHT; // scale it (linearly)
#endif

                    if ( y > SPECHEIGHT ) {
                        y = SPECHEIGHT;    // cap it
                    }

                    if ( x && ( y1 = ( y + y1 ) / 2 ) ) // interpolate from previous to make the display smoother
                        while ( --y1 >= 0 ) {
                            specbuf[y1 * SPECWIDTH + x * 2 - 1] = y1 + 1;
                        }

                    y1 = y;

                    while ( --y >= 0 ) {
                        int yo;
                        yo = y * SPECWIDTH + x * 2;

                        if ( yo >= 0 && yo < SPECWIDTH * SPECHEIGHT )
                        { specbuf[yo] = y + 1; }    // draw level
                    }
                }

            } else
                if ( specmode == 1 ) { // logarithmic, combine bins
                    int b0 = 0;
                    // one frame of colour wheel
                    {
                        static int counter = 0;

                        if ( counter++ == 3 ) {
                            audio_ColorWheel();
                            counter = 0;
                        }
                    }

                    memset ( specbuf, 0, SPECWIDTH * SPECHEIGHT );

#define BANDS ( 20 )

                    for ( x = 0; x < BANDS; x++ ) {

                        float peak = 0, intensity;

                        int b1 = pow ( 2, x * 10.0 / ( BANDS - 1 ) );

                        if ( b1 <= b0 ) {
                            b1 = b0 + 1;    // make sure it uses at least 1 FFT bin
                        }

                        if ( b1 > 1023 ) {
                            b1 = 1023;
                        }

                        for ( ; b0 < b1; b0++ )
                            if ( peak < fft[1 + b0] ) {
                                peak = fft[1 + b0];
                            }

                        y = sqrt ( peak ) * 3 * SPECHEIGHT - 4; // scale it (sqrt to make low values more visible)

                        if ( y > SPECHEIGHT ) {
                            y = SPECHEIGHT;    // cap it
                        }

                        intensity = y;

                        while ( --y >= 0 ) {
                            memset ( specbuf + y * SPECWIDTH + x * ( SPECWIDTH / BANDS ), y + 1, SPECWIDTH / BANDS - 2 ); // draw bar
                        }

                        // clip it
                        if ( intensity < 0 ) {
                            intensity = 0;
                        }

                        hsv_color  src;
                        led_color from;

                        // whats the current colour
                        from.rgb = StripLights_GetPixel ( x, 0 );

                        // convert it to hsv
                        src = rgb_to_hsv ( from );

                        // scale it
                        src.h.v = ( intensity * 1.24 );

                        // back to RGB
                        from = hsv_to_rgb ( src );

                        // and set it.
                        StripLights_Pixel ( x, 0, from.rgb );



                        if ( intensity < 10 ) {
                            //  StripLights_Pixel ( x, 0, StripLights_BLACK );
                            //FadeLED ( x, 0, 10 );

                        }

                    }

                } else {   // "3D"
                    for ( x = 0; x < SPECHEIGHT; x++ ) {
                        y = sqrt ( fft[x + 1] ) * 3 * 127; // scale it (sqrt to make low values more visible)

                        if ( y > 127 ) {
                            y = 127;    // cap it
                        }

                        specbuf[x * SPECWIDTH + specpos] = 128 + y; // plot it
                    }

                    // move marker onto next position
                    specpos = ( specpos + 1 ) % SPECWIDTH;

                    for ( x = 0; x < SPECHEIGHT; x++ ) {
                        specbuf[x * SPECWIDTH + specpos] = 255;
                    }
                }
        }

        // update the display
        dc = GetDC ( ( HWND ) win );
        BitBlt ( dc, 0, 0, SPECWIDTH, SPECHEIGHT, specdc, 0, 0, SRCCOPY );
        ReleaseDC ( ( HWND ) win, dc );

        double pos;
        QWORD bpos = BASS_ChannelGetPosition ( chan, BASS_POS_BYTE );

        pos = BASS_ChannelBytes2Seconds ( chan, bpos );

        int seconds;


        seconds = pos * 10;

        level = LOWORD ( BASS_ChannelGetLevel ( chan ) ) / 2;


        {
            sprintf ( tmp_buffer, "Audio Window - seconds in %4.04g, level = %d", pos, level );
        }

    } else {

        BASS_WASAPI_GetData ( fft, BASS_DATA_FFT2048 ); // get FFT data from the WASAPI device

    }

    StripLights_Trigger ( 0 );

}

