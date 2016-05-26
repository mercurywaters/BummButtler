#ifndef LIB_SHIFTREGISTER_SHIFTREGISTER_H_
#define LIB_SHIFTREGISTER_SHIFTREGISTER_H_

#include "mbed.h"

class ShiftRegister
{
    protected:
        const unsigned int numberOfPins;

        virtual bool testValueAt ( const int &index ) const = 0;

    private:
        DigitalOut dataPin;     // 75HC595 Pin 14
        DigitalOut latchPin;    // 75HC595 Pin 12
        DigitalOut clockPin;    // 75HC595 Pin 11
        DigitalOut enablePin;   // 75HC595 Pin 13
        DigitalOut resetPin;    // 75HC595 Pin 10

        enum PinState
        {
            LOW = 0,
            HIGH = 1,
        };

        ShiftRegister ( const ShiftRegister &other ); // Don't allow pass-by-value

    public:
        ShiftRegister ( PinName _dataPin, PinName _latchPin, PinName _clockPin, PinName _enablePin, PinName _resetPin, const unsigned int totalPinCount = 0 );
        virtual ~ShiftRegister () = 0;

        unsigned int setData ();
        bool masterReset ();
        bool disableOutput ();
        bool enableOutput ();

        inline int getNumberOfPins () const;
};

inline int ShiftRegister::getNumberOfPins () const
{
    return numberOfPins;
}

#endif /* LIB_SHIFTREGISTER_SHIFTREGISTER_H_ */
