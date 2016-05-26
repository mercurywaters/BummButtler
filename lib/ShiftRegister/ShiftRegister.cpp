#include "ShiftRegister.h"

/**
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * | Symbol                         | Pin                     | Description                      |
 * |________________________________|_________________________|__________________________________|
 * | Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7 | 15, 1, 2, 3, 4, 5, 6, 7 | parallel data output             |
 * | GND                            | 8                       | ground (0 V)                     |
 * | Q7S (Q7')                      | 9                       | serial data output (Daisy Chain) |
 * | MR (SRCLR)                     | 10                      | master reset (active LOW)        |
 * | SHCP (SRCLK) (clock)           | 11                      | shift register clock input       |
 * | STCP (RCLK) (latch)            | 12                      | storage register clock input     |
 * | OE                             | 13                      | output enable input (active LOW) |
 * | DS (SER) (data)                | 14                      | serial data input (dataPin)      |
 * | Q0                             | 15                      | parallel data output 0           |
 * | VCC                            | 16                      | supply voltage                   |
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * | CONTROL               | INPUT | OUTPUT    | Function                                           |
 * |-----------------------|-------|-----------|                                                    |
 * | SHCP | STCP | OE | MR | DS    | Q7S | Qn  |                                                    |
 * |______|______|____|____|_______|_____|_____|____________________________________________________|
 * | X    | X    | L  | L  | X     | L   | NC  | a LOW-level on MR only affects the shift registers |
 * | X    | ^    | L  | L  | X     | L   | L   | empty shift register loaded into storage register  |
 * | X    | X    | H  | L  | X     | L   | Z   | shift register clear; parallel outputs in high-    |
 * |      |      |    |    |       |     |     | impedance OFF-state                                |
 * | ^    | X    | L  | H  | H     | Q6s | NC  | logic HIGH-level shifted into shift register stage |
 * |      |      |    |    |       |     |     | 0. Contents of all shift register stages shifted   |
 * |      |      |    |    |       |     |     | through, e.g. previous state of stage 6 (internal  |
 * |      |      |    |    |       |     |     | Q6S) appears on the serial output (Q7S).           |
 * | X    | ^    | L  | H  | X     | NC  | Qns | contents of shift register stages (internal QnS)   |
 * |      |      |    |    |       |     |     | are transferred to the storage register and        |
 * |      |      |    |    |       |     |     | parallel output stages                             |
 * | ^    | ^    | L  | H  | X     | Q6s | Qns | contents of shift register shifted through;        |
 * |      |      |    |    |       |     |     | previous contents of the shift register is         |
 * |      |      |    |    |       |     |     | transferred to the storage register and the        |
 * |      |      |    |    |       |     |     | parallel output stages                             |
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

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
}

bool ShiftRegister::disableOutput ()
{
    enablePin = HIGH;
}

bool ShiftRegister::enableOutput ()
{
    enablePin = LOW;
}

