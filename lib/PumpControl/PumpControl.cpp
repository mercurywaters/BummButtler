#include "PumpControl.h"

//int SER_Pin = dataPin;    //pin 14 on the 75HC595 --> BLUE (BLACK FOR DAISY-CHAINING)
//int RCLK_Pin = latchPin;  //pin 12 on the 75HC595 --> GREEN
//int SRCLK_Pin = clockPin; //pin 11 on the 75HC595 --> YELLOW
//int OE = Output Enable;   //pin 13 on the 75HC595 --> WHITE
//int MR = Master Reset;    //pin 10 on the 75HC595 --> GOLD/GREY

PumpControl::PumpControl ( PinName _dataPin, PinName _latchPin, PinName _clockPin, PinName _enablePin, PinName _resetPin, const unsigned int pumpCount )
        : dataPin(_dataPin), latchPin(_latchPin), clockPin(_clockPin), enablePin(_enablePin), resetPin(_resetPin), connectedPumpCount (pumpCount)
{
    enablePin = HIGH; //diable the output

    isExecuteSemaphoreLock = false;

    pumpTimer = new Ticker ();

    pumpRunningTime = new unsigned int [ connectedPumpCount ];
//    backupRunningTime = new unsigned int [ connectedPumpCount ];

    resetPumps ();

    pumpTimer->attach_us ( this, &PumpControl::atPumpTimer, 1000000 );
}

PumpControl::PumpControl ( const PumpControl & other )
        : dataPin(D0), latchPin(D0), clockPin(D0), enablePin(D0), resetPin(D0), connectedPumpCount(0)
{
    isExecuteSemaphoreLock = false;
    pumpTimer = NULL;
    pumpRunningTime = NULL;
//    backupRunningTime = NULL;
}

PumpControl::~PumpControl ()
{
    pumpTimer -> detach();

    delete pumpTimer;
//    delete [] backupRunningTime;
    delete [] pumpRunningTime;
}

void PumpControl::pinStateChanged ( const PinName pin, const int pinId, const bool pinValue )
{
    if ( pinValue == true )
    {
        resumePumps ();
    }
    else
    {
        pausePumps ();
    }
}

void PumpControl::resetPumps ()
{
    resetPin = LOW; // Pulling Reset Pin to LOW will clear the shift register

    for ( unsigned int i = 0; i < connectedPumpCount; i++ )
    {
        pumpRunningTime [ i ] = 0;
    }

//    executePumpTimers ();
    pumpControllerState = Idle;
    resetPin = HIGH; // Pull it high up again to make it working for next data transactions
    enablePin = LOW; // enable the output
}

void PumpControl::atPumpTimer ()
{
    if ( pumpControllerState == Executing )
    {
        bool stateChanged = false;
        for ( unsigned int i = 0; i < connectedPumpCount; i++ )
        {
            if ( pumpRunningTime [ i ] > 0 )
            {
                pumpRunningTime [ i ] --;
                stateChanged = true;
            }
        }

        if ( stateChanged )
        {
            executePumpTimers ();
        }
    }
}

void PumpControl::executePumpTimers ()
{
    if ( !isExecuteSemaphoreLock )
    {
        isExecuteSemaphoreLock = true;

        unsigned long currentStatus = 0;

        latchPin.write ( LOW );

        for ( int i = ( connectedPumpCount - 1 ); i >= 0; i-- )
        {
            clockPin.write ( LOW );

            if ( pumpRunningTime [ i ] > 0 )
            {
                dataPin.write ( HIGH );
                currentStatus |= 0x01;
            }
            else
            {
                dataPin.write ( LOW );
            }

            clockPin.write ( HIGH );

            currentStatus = currentStatus << 1;
        }

        if ( currentStatus == 0 )
        {
            pumpControllerState = Idle;
        }
        else
        {
            pumpControllerState = Executing;
        }

        latchPin.write ( HIGH );

        isExecuteSemaphoreLock = false;
    }
}

bool PumpControl::runPumpsFor ( unsigned int * durations )
{
    if ( pumpControllerState == Idle )
    {

        for ( unsigned int i = 0; i < connectedPumpCount; i++ )
        {
            pumpRunningTime [ i ] = durations [ i ];
        }

        executePumpTimers ();
        return true;
    }
    return false;
}

void PumpControl::pausePumps ()
{
    if ( pumpControllerState == Executing )
    {
//        // backup current runningTimes
//        for ( unsigned int i = 0; i < connectedPumpCount; i++ )
//        {
//            backupRunningTime [ i ] = pumpRunningTime [ i ];
//            pumpRunningTime [ i ] = 0;
//        }
//        executePumpTimers ();

        enablePin = HIGH;

        pumpControllerState = Paused; // Explicitly set it to Paused
    }
}

void PumpControl::resumePumps ()
{
    if ( pumpControllerState == Paused )
    {
//        // restore the runningTimes
//        for ( unsigned int i = 0; i < connectedPumpCount; i++ )
//        {
//            pumpRunningTime [ i ] = backupRunningTime [ i ];
//        }
        enablePin = LOW;
        pumpControllerState = Executing;
    }
}
