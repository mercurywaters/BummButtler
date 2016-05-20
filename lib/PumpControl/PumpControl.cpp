#include "PumpControl.h"

//int SER_Pin = dataPin;   //pin 14 on the 75HC595
//int RCLK_Pin = latchPin;  //pin 12 on the 75HC595
//int SRCLK_Pin = clockPin; //pin 11 on the 75HC595

PumpControl::PumpControl ( PinName latch, PinName clock, PinName data, const unsigned int pumpCount )
        : connectedPumpCount ( pumpCount ), latchPin ( latch, 0 ), clockPin ( clock, 0 ), dataPin ( data, 0 )
{
    isExecuteSemaphoreLock = false;

    pumpTimer = new Ticker ();

    pumpRunningTime = new unsigned int [ connectedPumpCount ];
    backupRunningTime = new unsigned int [ connectedPumpCount ];

    resetPumps ();

    pumpTimer->attach_us ( this, &PumpControl::atPumpTimer, 1000000 );
}

PumpControl::~PumpControl ()
{
    pumpTimer -> detach();

    delete pumpTimer;
    delete [] backupRunningTime;
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
    for ( unsigned int i = 0; i < connectedPumpCount; i++ )
    {
        pumpRunningTime [ i ] = 0;
    }

    executePumpTimers ();
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

        latchPin.write ( 0 );

        for ( int i = ( connectedPumpCount - 1 ); i >= 0; i-- )
        {
            clockPin.write ( 0 );

            if ( pumpRunningTime [ i ] > 0 )
            {
                dataPin.write ( 0 );
                currentStatus |= 0x01;
            }
            else
            {
                dataPin.write ( 1 );
            }

            clockPin.write ( 1 );

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

        latchPin.write ( 1 );

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
        // backup current runningTimes
        for ( unsigned int i = 0; i < connectedPumpCount; i++ )
        {
            backupRunningTime [ i ] = pumpRunningTime [ i ];
            pumpRunningTime [ i ] = 0;
        }
        executePumpTimers ();
        pumpControllerState = Paused; // Explicitly set it to Paused
    }
}

void PumpControl::resumePumps ()
{
    if ( pumpControllerState == Paused )
    {
        // restore the runningTimes
        for ( unsigned int i = 0; i < connectedPumpCount; i++ )
        {
            pumpRunningTime [ i ] = backupRunningTime [ i ];
        }
        executePumpTimers ();
    }
}
