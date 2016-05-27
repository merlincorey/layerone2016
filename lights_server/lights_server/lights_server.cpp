// lights_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

WSADATA  w;
SOCKET sd;

// must match the devices.
int port_number = 40002;
sockaddr_in local, server, client;

// 150 * 3 keep this the same
#define BUFFER_SIZE             ( StripLights_COLUMNS * 3 )
//unsigned char buffer[BUFFER_SIZE];

UINT  update_thread ( LPVOID pParam );

volatile unsigned char update[NUMBER_NODES] ;

//
// 209 debug strip in office
//  58 in garage
//
///
//static const unsigned int ip[NUMBER_NODES] = { 209, 5, 237, 206, 4, 217, 175, 65, 60, 58 };

#if NUMBER_NODES != 1
static const unsigned int ip[NUMBER_NODES] = {  217, 238 };
#else
static const unsigned int ip[NUMBER_NODES] = {  255 };
#endif


/*
Get the mac address of a given ip
*/
void GetMacAddress ( unsigned char *mac ,  struct in_addr destip )
{
    DWORD ret;
    struct in_addr srcip;
    ULONG MacAddr[2];
    ULONG PhyAddrLen = 6;  /* default to length of six bytes */
    int i;

    srcip.s_addr = 0;

    //Send an arp packet
    ret = SendARP ( ( IPAddr ) destip.S_un.S_addr , ( IPAddr ) srcip.S_un.S_addr , MacAddr , &PhyAddrLen );

    //Prepare the mac address
    if ( PhyAddrLen ) {
        BYTE *bMacAddr = ( BYTE * ) & MacAddr;

        for ( i = 0; i < ( int ) PhyAddrLen; i++ ) {
            mac[i] = ( char ) bMacAddr[i];
        }
    }
}
void Larson1 ( int count );

int _tmain ( int argc, _TCHAR* argv[] )
{
    /* Open windows connection */
    if ( WSAStartup ( MAKEWORD ( 2, 2 ), &w ) != 0 ) {
        fprintf ( stderr, "Could not open Windows connection.\n" );
        exit ( 0 );
    }

    InitBASS ( GetModuleHandle ( NULL )  );

    /* Open a datagram socket */
    sd = socket ( AF_INET, SOCK_DGRAM, 0 );

    if ( sd == INVALID_SOCKET ) {
        fprintf ( stderr, "Could not create socket.\n" );
        WSACleanup();
        exit ( 0 );
    }

    /* Clear out local struct */
    memset ( ( void * ) &local, '\0', sizeof ( struct sockaddr_in ) );

    /* Set family and port */
    local.sin_family = AF_INET;
    local.sin_port = htons ( port_number );
    local.sin_addr.s_addr = INADDR_ANY;


    // bind to the local address
    if ( bind ( sd, ( sockaddr * ) &local, sizeof ( local )  ) == -1 ) {

        fprintf ( stderr, "Could not bind name to socket. %d\n" , WSAGetLastError() );
        closesocket ( sd );
//		WSACleanup();
        //  exit ( 0 );
    }

    // Start the threads for each of the nodes.
    AfxBeginThread ( update_thread, ( LPVOID ) NUMBER_NODES );

    {
        unsigned char ch;

        do {
            printf ( "ready\n" );
            ch = _getch();
            Select ( ch );
        } while ( ch != 27 );
    }

    while ( 1 ) {
        static int selected, lastselected;
        {

            if ( _kbhit() ) { break; }

            do {
                selected = 'a' + ( rand() % ( 'z' - 'a' ) );

            } while ( selected == lastselected );

            printf ( "%d %d\n", selected, lastselected );

            Select ( selected );

            lastselected = selected;
        }
    }

    StripLights_DisplayClear ( RGB ( 0, 0, 0 ) );
    StripLights_Trigger ( 1 );

    WSACleanup();

    return 0;
}


void StripLights_Trigger ( int j )
{

    int i = 0;

    if ( j > 1 ) {
        static _declspec ( thread )  threaddata * lpvData;

        if ( lpvData == NULL ) {

            lpvData = ( threaddata* ) malloc ( sizeof ( threaddata ) );

            lpvData->sock = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP );


            /* Clear out server struct */
            memset ( ( void * ) &lpvData->dest_ip[i], '\0', sizeof ( struct sockaddr_in ) );

            printf ( "Adding node 192.168.1.%d\n", j );

            /* Set family and port */
            lpvData->dest_ip[i].sin_family = AF_INET;
            lpvData->dest_ip[i].sin_port = htons ( port_number );
            lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b1 = ( unsigned char ) 192;
            lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b2 = ( unsigned char ) 168;
            lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b3 = ( unsigned char ) 1;

        }

        lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b4 = ( unsigned char ) j;

        sendto (
            lpvData->sock,
            ( const char* ) &StripLights_ledArray[0][0], BUFFER_SIZE,
            MSG_DONTROUTE, ( sockaddr * ) &lpvData->dest_ip[i], sizeof ( struct sockaddr_in )
        );

        return;

    }

//	ret     = sendto ( s, ( const char* ) &StripLights_ledArray[0][0], BUFFER_SIZE, 0, ( sockaddr * ) &dest[1], sizeof ( struct sockaddr_in ) );


    for ( int i = 0 ; i < NUMBER_NODES; i++ ) {
        update[i]  = TRUE ;
    }

    for ( int i = 0 ; i < NUMBER_NODES; i++ ) {
        while ( update[i] == TRUE ) { Sleep ( 10 ); }

    }




}

class CTimer
{
    private:
#     if defined(_MSC_VER)
        LARGE_INTEGER m_depart;
#     else
        timeval m_depart;
#     endif

    public:
        inline void start()
        {
#        if defined(_MSC_VER)
            QueryPerformanceCounter ( &m_depart );
#        else
            gettimeofday ( &m_depart, 0 );
#        endif
        };

        inline float GetSeconds() const
        {
#        if defined(_MSC_VER)
            LARGE_INTEGER now;
            LARGE_INTEGER freq;

            QueryPerformanceCounter ( &now );
            QueryPerformanceFrequency ( &freq );

            return ( now.QuadPart - m_depart.QuadPart ) / static_cast<float> ( freq.QuadPart );
#        else
            timeval now;
            gettimeofday ( &now, 0 );

            return now.tv_sec - m_depart.tv_sec + ( now.tv_usec - m_depart.tv_usec ) / 1000000.0f;
#        endif
        };
};

DWORD dwTlsIndex;

UINT  update_thread ( LPVOID pParam )
{
    unsigned int sleep_counter, same_counter;
    static _declspec ( thread )  threaddata * lpvData;

    lpvData = ( threaddata* ) LocalAlloc ( LPTR, sizeof ( threaddata ) );

    lpvData->sock = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP );


    for ( int i = 0; i < ( int ) pParam; i ++ ) {

        /* Clear out server struct */
        memset ( ( void * ) &lpvData->dest_ip[i], '\0', sizeof ( struct sockaddr_in ) );

        printf ( "Adding node 192.168.1.%d\n", ip[i] );

        /* Set family and port */
        lpvData->dest_ip[i].sin_family = AF_INET;
        lpvData->dest_ip[i].sin_port = htons ( port_number );
        lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b1 = ( unsigned char ) 192;
        lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b2 = ( unsigned char ) 168;
        lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b3 = ( unsigned char ) 1;
        lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b4 = ( unsigned char ) ip[i];

        unsigned char tmp_buffer[128];
        memset ( tmp_buffer, 0, sizeof ( tmp_buffer ) );

        GetMacAddress ( &tmp_buffer[0], lpvData->dest_ip[i].sin_addr );

        printf ( "MAC Address of A %d.%d.%d.%d = ",
                 lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b1,
                 lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b2,
                 lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b3,
                 lpvData->dest_ip[i].sin_addr.S_un.S_un_b.s_b4
               );

        for ( int i = 0 ; i < 6 ; i++ ) {
            printf ( ":%02x", tmp_buffer[i] );
        }

        printf ( "\n" );
    }

    QueryPerformanceCounter ( &lpvData->m_depart );

    same_counter = 0;

    do  {
        {


            // lock to a fixed rate
            QueryPerformanceCounter ( &lpvData->now );
            QueryPerformanceFrequency ( &lpvData->freq );

            lpvData->before_time = ( lpvData->now.QuadPart - lpvData->m_depart.QuadPart ) / static_cast<float> ( lpvData->freq.QuadPart );

            lpvData->before_time += ( ( 1000.0 / 50.0 ) / 1000.0 );


            for ( int i = 0; i < ( int ) pParam; i++ ) {

                if ( update[i] == TRUE ) {


                    if ( 0 && memcmp (
                                last_StripLights_ledArray,
                                StripLights_ledArray, sizeof ( StripLights_ledArray )
                            ) == 0 ) {

                        same_counter++;

                    } else {

                        uint8_t *ptr;
                        ptr = &StripLights_ledArray[0][i * BUFFER_SIZE];
                        sendto (
                            lpvData->sock,
                            ( const char* ) ptr, BUFFER_SIZE,
                            MSG_DONTROUTE, ( sockaddr * ) &lpvData->dest_ip[i], sizeof ( struct sockaddr_in )
                        );

                        memcpy (
                            last_StripLights_ledArray,
                            StripLights_ledArray, sizeof ( StripLights_ledArray )
                        );
                    }

                    // allow the host to go on ( do we do this before or after the delay )
                    update[i] = FALSE;

                }
            }

            sleep_counter = 0;

            while ( 1 ) {

                QueryPerformanceCounter ( &lpvData->now );
                QueryPerformanceFrequency ( &lpvData->freq );

                lpvData->after_time = ( lpvData->now.QuadPart - lpvData->m_depart.QuadPart ) / static_cast<float> ( lpvData->freq.QuadPart );

                if ( lpvData->before_time < lpvData->after_time ) {
                    break;
                }

                Sleep ( 1 );

                sleep_counter++;

            }

            if ( same_counter || sleep_counter ) {
                _RPT2 ( _CRT_WARN, "same counter = %d, sleep = %d\n", same_counter, sleep_counter );
            }
        }

        Sleep ( 1 );

    } while ( 1 );

    if ( lpvData != 0 ) {
        LocalFree ( ( HLOCAL ) lpvData );
    }

    closesocket ( lpvData->sock ) ;

    return 0;

}


