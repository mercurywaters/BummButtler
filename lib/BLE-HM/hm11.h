/**
 * @file hm11.h
 *
 * @brief Bluetooth Low Energy v4.0 HM-11 Breakout Library
 *
 * @Author lukasz uszko(luszko@op.pl)
 *
 * Tested on FRDM-KL25Z and STM32NUCLEO
 *
 * Copyright (c) 2014 lukasz uszko
 * Released under the MIT License (see http://mbed.org/license/mit)
 *
 * Documentation regarding the HM-11 Bluetooth Low energy module can be found here:
 * https://www.microduino.cc/wiki/images/f/fc/Bluetooth40_en.pdf
 * http://txyz.info/b10n1c/datasheets/hm-11_bluetooth40_en.pdf
 *
 * HM-11 Device Pinout:
 *  Pin        Name              Description
 *  1          UART_RTS          UART interface
 *  2          UART_TX           UART interface
 *  3          UART_CTS          UART interface
 *  4          UART_RX           UART interface
 *  5          NC                   NC
 *  6          NC                   NC
 *  7          NC                   NC
 *  8          NC                   NC
 *  9          CC                   V3.3
 *  10         NC                   NC or VCC
 *  11         RESETB            Reset if low <100ms
 *  12         GND                 Ground
 *  13         PIO3                 Programmable input/output line
 *  14         PIO2                 Programmable input/output line
 *  15         PIO1                 System LED
 *  16         PIO0                 System KEY
 *  AT Commands
 *  Factory default setting:
 *  Name: HMSoft; Baud: 9600, N, 8, 1; Pin code: 000000; transmit
 *
 * DEMO - HOW TO USE:
 * compiled and built under mbed
 * ---------------------------------------- DEMO: 1st version -simple polling ----------------------------------------
 * @code
 *  #include "mbed.h"
 *  #include "hm11.h"
 *
 *
 *  #define HM11_PIN_TX PTE22 //FRDM-KL25Z UART2
 *  #define HM11_PIN_RX PTE23
 *
 *  int main() {
 *      Serial usbDebug(USBTX, USBRX);
 *      usbDebug.printf("HELLO WORLD !");
 *
 *
 *      HM11* hm11 = new HM11( HM11_PIN_TX, HM11_PIN_RX);
 *      int counter =0;
 *      while(1) {
 *
 *          wait(0.5);
 *          usbDebug.printf("alive ");
 *          wait(0.5);
 *          char buf[2];
 *          snprintf(buf,2,"%d",counter++);
 *
 *          if(counter>9)
 *              counter=0;
 *
 *          hm11->sendDataToDevice(buf);
 *          wait(0.2);
 *
 *          while(hm11->isRxDataAvailable())
 *              usbDebug.printf("data:  %c\r\n",hm11->getDataFromRx());
 *      }
 *  }
 * @endcode
 *
 * Modified version of original hm11 library.  Added char * buffer support and
 * commendted out commands facility to save memory.  Will remove other command
 * utility functions as well, to further reduce memory fottprint
 */

#ifndef HM11_H
#define HM11_H

#include "mbed.h"
#include "BufferedSerial.h"

#define HM11_SERIAL_DEFAULT_BAUD       9600
#define HM11_SERIAL_TIMEOUT            10000
#define HM11_SERIAL_EOL                "\r\n"

class HM11
{
    private:
        BufferedSerial mSerial;

    public:
        /**
         * @param uartTx
         * @param uartRx
         */
        HM11 ( PinName uartTx, PinName uartRx );
        HM11 ( const BufferedSerial & serial );

        int sendDataToDevice ( const char* data );
        int sendDataToDevice ( const uint8_t * byteData, uint8_t dataLength );

        int isRxDataAvailable ();

        int copyAvailableDataToBuf ( char *buf, uint16_t bufLength );
        int copyAvailableDataToBuf ( uint8_t *buf, uint8_t bufLength );
        int copyAvailableDataToBufWithTimeout ( char *buf, uint16_t bufLength, int timeoutMs );

        bool waitForData ( int timeoutMs );

        inline uint8_t getDataFromRx ();
        inline void flushBuffers ();

        inline static bool hexToString ( uint32_t hex, char*str, uint8_t len );
        inline static uint32_t strToHex ( char* const str, uint8_t len );
};

inline uint8_t HM11::getDataFromRx ()
{
    return mSerial.getc ();
}

inline void HM11::flushBuffers ()
{
    mSerial.clearRxBuf ();
    mSerial.clearTxBuf ();
}

inline bool HM11::hexToString ( uint32_t hex, char*str, uint8_t nrOfNibblesInHex )
{
    if ( nrOfNibblesInHex > 8 || nrOfNibblesInHex == 0 || str == NULL )
        return false;
    for ( int i = 0; i < nrOfNibblesInHex; i++ )
    {
        int temp = ( hex >> ( i * 4 ) ) & 0x0F;
        if ( temp >= 0 && temp <= 9 )
        {
            str [ nrOfNibblesInHex - i - 1 ] = temp + '0';
        }
        else
        {
            str [ nrOfNibblesInHex - i - 1 ] = temp + 'A' - 10;
        }
    }
    return true;
}

//returns hex in  reverse direction
inline uint32_t HM11::strToHex ( char* const str, uint8_t len ) //len - nr of nibbles in str
{
    uint32_t ret = 0;
    uint8_t temp;
    if ( len < 1 || len > 8 || str == NULL )
        return 0xFFFFFFFF;

    for ( int i = 0; i < len && str [ i ] != '\0'; i++ )
    {
        if ( str [ i ] >= '0' && str [ i ] <= '9' )
        {
            temp = str [ i ] - '0';
            ret |= temp << ( i * 4 );
        }
        else if ( str [ i ] >= 'A' && str [ i ] <= 'F' )
        {
            temp = str [ i ] - 'A' + 10;
            ret |= temp << ( i * 4 );
        }
        else if ( str [ i ] >= 'a' && str [ i ] <= 'f' )
        {
            temp = str [ i ] - 'a' + 10;
            ret |= temp << ( i * 4 );
        }
        else
            return 0xFFFFFFFF;  //-1 :-)
    }
    return ret;
}

#endif
