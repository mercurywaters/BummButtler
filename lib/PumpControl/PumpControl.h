#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include "IrSensorPin.h"
#include "mbed.h"

#define __PUMPCONTROL_DURATION_MAX_SECS__   300

// ------------- PUMP Control Functions Start ---------------- //
enum PumpControllerState
{
    Idle,
    Paused,
    Executing
};

class PumpControl: public IrSensorListener
{
    private:
        DigitalOut dataPin;     // 75HC595 Pin 14 - Blue
        DigitalOut latchPin;    // 75HC595 Pin 12 - Green
        DigitalOut clockPin;    // 75HC595 Pin 11 - Yellow
        DigitalOut enablePin;   // 75HC595 Pin 13 - White
        DigitalOut resetPin;    // 75HC595 Pin 10 - Grey/Gold

        const unsigned int connectedPumpCount;

        volatile PumpControllerState pumpControllerState;
        volatile bool isExecuteSemaphoreLock;

        unsigned int * pumpRunningTime;
//        unsigned int * backupRunningTime;

        enum PinState {
          LOW = 0,
          HIGH = 1,
        };

        Ticker * pumpTimer;
        void atPumpTimer ();

        void executePumpTimers ();

        PumpControl ( const PumpControl &other );

    public:
        PumpControl ( PinName _dataPin, PinName _latchPin, PinName _clockPin, PinName _enablePin, PinName _resetPin, const unsigned int pumpCount = 0 );
        virtual ~PumpControl ();

        virtual bool runPumpsFor ( unsigned int * durations );
        virtual void pausePumps ();
        virtual void resumePumps ();
        virtual void resetPumps ();

        virtual void pinStateChanged ( const PinName pin, const int pinId, const bool pinValue );

        inline PumpControllerState getState () const;
        inline int getPumpCount () const;
        inline bool isValidId ( const unsigned int id ) const;
        inline bool isValidDuration ( const int duration ) const;
};

inline PumpControllerState PumpControl::getState () const
{
    return pumpControllerState;
}

inline int PumpControl::getPumpCount () const
{
    return connectedPumpCount;
}

inline bool PumpControl::isValidId ( const unsigned int id ) const
{
    return ( id < connectedPumpCount );
}

inline bool PumpControl::isValidDuration ( const int duration ) const
{
    return ( ( duration >= 0 ) && ( duration <= __PUMPCONTROL_DURATION_MAX_SECS__ ) );
}

#endif
