#include "ShiftRegister.h"

ShiftRegister::ShiftRegister ( PinName _dataPin, PinName _latchPin, PinName _clockPin, PinName _enablePin, PinName _resetPin, const unsigned int totalPinCount )
        : dataPin ( _dataPin ), latchPin ( _latchPin ), clockPin ( _clockPin ), enablePin ( _enablePin ), resetPin ( _resetPin ), numberOfPins ( totalPinCount )
{
    enablePin = HIGH;   //diable the output
    masterReset ();
}

ShiftRegister::ShiftRegister ( const ShiftRegister & other )
        : dataPin ( D0 ), latchPin ( D0 ), clockPin ( D0 ), enablePin ( D0 ), resetPin ( D0 ), numberOfPins ( 0 )
{
}

ShiftRegister::~ShiftRegister ()
{
}

unsigned int ShiftRegister::setData ()
{
    int highBits = 0;

    latchPin.write ( LOW );

    for ( int i = ( numberOfPins - 1 ); i >= 0; i-- )
    {
        clockPin.write ( LOW );

        if ( testValueAt ( i ) == true )
        {
            dataPin.write ( HIGH );
            highBits++;
        }
        else
        {
            dataPin.write ( LOW );
        }

        clockPin.write ( HIGH );
    }
    latchPin.write ( HIGH );

    return highBits;
}

bool ShiftRegister::masterReset ()
{
    latchPin = LOW;
    resetPin = LOW;
    enablePin = LOW;
    latchPin = HIGH; // Make sure it is high, to capture the rising edge for reset

    enablePin = HIGH;

    resetPin = HIGH;
    enablePin = LOW; // enable the Output, after reset

    return true;
}

bool ShiftRegister::disableOutput ()
{
    enablePin = HIGH;
    return true;
}

bool ShiftRegister::enableOutput ()
{
    enablePin = LOW;
    return true;
}
