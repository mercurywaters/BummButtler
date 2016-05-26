#include "PumpControl.h"

PumpControl::PumpControl ( PinName _dataPin, PinName _latchPin, PinName _clockPin, PinName _enablePin, PinName _resetPin, const unsigned int pumpCount )
        : ShiftRegister ( _dataPin, _latchPin, _clockPin, _enablePin, _resetPin, pumpCount )
{
    isExecuteSemaphoreLock = false;
    pumpTimer = new Ticker ();
    pumpRunningTime = new unsigned int [ numberOfPins ];
    resetPumps ();
    pumpTimer->attach_us ( this, &PumpControl::atPumpTimer, 1000000 );
}

PumpControl::PumpControl ( const PumpControl & other )
        : ShiftRegister ( D0, D0, D0, D0, D0, 0 )
{
    isExecuteSemaphoreLock = false;
    pumpTimer = NULL;
    pumpRunningTime = NULL;
}

PumpControl::~PumpControl ()
{
    pumpTimer->detach ();

    delete pumpTimer;
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
    for ( unsigned int i = 0; i < numberOfPins; i++ )
    {
        pumpRunningTime [ i ] = 0;
    }
    pumpControllerState = Idle;
    masterReset ();
}

void PumpControl::atPumpTimer ()
{
    if ( pumpControllerState == Executing )
    {
        bool stateChanged = false;
        for ( unsigned int i = 0; i < numberOfPins; i++ )
        {
            if ( pumpRunningTime [ i ] > 0 )
            {
                pumpRunningTime [ i ]--;
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

        unsigned int highBits = setData ();

        if ( highBits == 0 )
        {
            pumpControllerState = Idle;
        }
        else
        {
            pumpControllerState = Executing;
        }

        isExecuteSemaphoreLock = false;
    }
}

bool PumpControl::runPumpsFor ( unsigned int * durations )
{
    if ( pumpControllerState == Idle )
    {

        for ( unsigned int i = 0; i < numberOfPins; i++ )
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
        disableOutput ();
        pumpControllerState = Paused; // Explicitly set it to Paused
    }
}

void PumpControl::resumePumps ()
{
    if ( pumpControllerState == Paused )
    {
        enableOutput ();
        pumpControllerState = Executing;
    }
}

bool PumpControl::testValueAt ( const int &index ) const
{
    return ( pumpRunningTime [ index ] > 0 );
}
