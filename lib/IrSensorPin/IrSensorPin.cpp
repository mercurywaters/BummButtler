#include "IrSensorPin.h"
#include "PinNames.h"

IrSensorPin::IrSensorPin ( const PinName _pinName, const int _pinId, IrSensorListener * &_listener )
        : InterruptIn ( _pinName ), pinName ( _pinName ), pinId ( _pinId )
{
    listener = _listener;
    mode( PullUp );

//    rise ( this, &IrSensorPin::pinStateChanged );
    fall ( this, &IrSensorPin::pinStateChanged );
}

IrSensorPin::IrSensorPin ( const IrSensorPin &other )
    : InterruptIn ( D1 ), pinName(D1), pinId(-1)
{
    listener = NULL;
}

IrSensorPin::~IrSensorPin ()
{
}

void IrSensorPin::pinStateChanged ()
{
    bool pinValue = ( read () == 1 );
    listener->pinStateChanged ( pinName, pinId, pinValue );
}

/*
 template<typename T> void rise(T* tptr, void (T::*mptr)(void)) {
 _rise.attach(tptr, mptr);
 gpio_irq_set(&gpio_irq, IRQ_RISE, 1);
 }
 */
