#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include "mbed.h"
#include "IrSensorPin.h"
#include "ShiftRegister.h"

#define __PUMPCONTROL_DURATION_MAX_SECS__   300

// ------------- PUMP Control Functions Start ---------------- //
enum PumpControllerState
{
    Idle,
    Paused,
    Executing
};

class PumpControl: private ShiftRegister, public IrSensorListener
{
    private:

        volatile PumpControllerState pumpControllerState;
        volatile bool isExecuteSemaphoreLock;

        unsigned int * pumpRunningTime;

        Ticker * pumpTimer;
        void atPumpTimer ();

        void executePumpTimers ();

        PumpControl ( const PumpControl &other );

        bool testValueAt ( const int &index ) const;

    public:
        PumpControl ( PinName _dataPin, PinName _latchPin, PinName _clockPin, PinName _enablePin, PinName _resetPin, const unsigned int pumpCount = 0 );
        virtual ~PumpControl ();

        virtual bool runPumpsFor ( unsigned int * durations );
        virtual void pausePumps ();
        virtual void resumePumps ();
        virtual void resetPumps ();

        virtual void pinStateChanged ( const PinName pin, const int pinId, const bool pinValue );

        inline PumpControllerState getState () const;
        inline unsigned int getPumpCount () const;
        inline bool isValidId ( const unsigned int id ) const;
        inline bool isValidDuration ( const int duration ) const;
};

inline PumpControllerState PumpControl::getState () const
{
    return pumpControllerState;
}

inline unsigned int PumpControl::getPumpCount () const
{
    return numberOfPins;
}

inline bool PumpControl::isValidId ( const unsigned int id ) const
{
    return ( id < getPumpCount () );
}

inline bool PumpControl::isValidDuration ( const int duration ) const
{
    return ( ( duration >= 0 ) && ( duration <= __PUMPCONTROL_DURATION_MAX_SECS__ ) );
}

#endif
