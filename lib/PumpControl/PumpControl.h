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
        const unsigned int connectedPumpCount;
        unsigned int * pumpRunningTime;
        unsigned int * backupRunningTime;
        DigitalOut latchPin;
        DigitalOut clockPin;
        DigitalOut dataPin;

        volatile PumpControllerState pumpControllerState;
        volatile bool isExecuteSemaphoreLock;

        Ticker * pumpTimer;
        void atPumpTimer ();

        void executePumpTimers ();

    public:
        PumpControl ( PinName latch, PinName clock, PinName data, const unsigned int pumpCount = 0 );
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

//int SER_Pin = dataPin;   //pin 14 on the 75HC595
//int RCLK_Pin = latchPin;  //pin 12 on the 75HC595
//int SRCLK_Pin = clockPin; //pin 11 on the 75HC595
