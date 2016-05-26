#ifndef __IR_SENSOR_PIN_H
#define __IR_SENSOR_PIN_H

#include "mbed.h"

class IrSensorListener
{
    public:
        virtual ~IrSensorListener ()
        {
        }
        virtual void pinStateChanged ( const PinName pin, const int pinId, const bool pinValue ) = 0;
};

class IrSensorPin : private InterruptIn
{
    private:
        const PinName pinName;
        const int pinId;
        IrSensorListener * listener;

        IrSensorPin ( const IrSensorPin &other ); // Don't allow copying at all

        void pinStateChanged ();

    public:
        IrSensorPin ( const PinName _pinName, const int _pinId, IrSensorListener * _listener = NULL );
        virtual ~IrSensorPin ();
};

#endif
