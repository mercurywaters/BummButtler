#include "hm11.h"
#include "string.h"

#define WAIT_FOR_DATA_TIMEOUT_MS 1000
#define DEFAULT_WAIT_FOR_DATA_TIMEOUT_MS 50  //required to get correct data

HM11::HM11 ( PinName uartTx, PinName uartRx )
        : mSerial ( uartTx, uartRx )
{

    mSerial.baud ( HM11_SERIAL_DEFAULT_BAUD );
}

bool HM11::waitForData ( int timeoutMs )
{
    int endTime;
    int startTime; // sometimes not needed 50ms
    Timer timer;
    timer.start ();
    startTime = timer.read_ms ();
    endTime = startTime + timeoutMs;
    while ( ( timer.read_ms () ) < endTime )
    {
        if ( isRxDataAvailable () && ( timer.read_ms () - startTime ) > DEFAULT_WAIT_FOR_DATA_TIMEOUT_MS )
            return true;
    }
    return false;
}

int HM11::sendDataToDevice ( const char* data )
{
    return mSerial.printf ( data );
}

int HM11::sendDataToDevice ( const uint8_t * byteData, uint8_t dataLength )
{
    return mSerial.write ( byteData, dataLength );
}

int HM11::isRxDataAvailable ()
{
    return mSerial.readable ();
}

int HM11::copyAvailableDataToBuf ( uint8_t *buf, uint8_t bufLength )
{
    int lenCounter = 0;
    if ( buf == NULL || bufLength < 1 )
        return -1;
    while ( isRxDataAvailable () && lenCounter < bufLength )
    {
        buf [ lenCounter++ ] = getDataFromRx ();
    }
    return lenCounter;
}

int HM11::copyAvailableDataToBuf ( char *buf, uint16_t bufLength )
{
    int lenCounter = 0;
    if ( buf == NULL || bufLength < 1 )
        return -1;
    while ( isRxDataAvailable () && lenCounter < bufLength )
    {
        buf [ lenCounter++ ] = (char) getDataFromRx ();
    }
    buf [ lenCounter ] = 0;
    return lenCounter;
}

int HM11::copyAvailableDataToBufWithTimeout ( char *buf, uint16_t bufLength, int timeoutMs )
{
    int lenCounter = 0;
    if ( buf == NULL || bufLength < 1 )
        return -1;
    while ( isRxDataAvailable () && lenCounter < bufLength )
    {
        buf [ lenCounter++ ] = (char) getDataFromRx ();
        waitForData ( timeoutMs );
    }
    buf [ lenCounter ] = 0;
    return lenCounter;
}

