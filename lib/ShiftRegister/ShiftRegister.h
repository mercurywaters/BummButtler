#ifndef LIB_SHIFTREGISTER_SHIFTREGISTER_H_
#define LIB_SHIFTREGISTER_SHIFTREGISTER_H_

#include "mbed.h"

/**
 *      74HC595 & 74HCT595 Pinouts
 *      +--------+
 *  Q1--|01 \/ 16|--Vcc
 *  Q2--|02    15|--Q0
 *  Q3--|03    14|--DS/SER/Data
 *  Q4--|04    13|--OE
 *  Q5--|05    12|--STCP/RCLK/Latch
 *  Q6--|06    11|--SHCP/SRCLK/Clock
 *  Q7--|07    10|--MR
 * GND--|08    09|--Q7S/Q7'
 *      +--------+

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
 * | Vcc                            | 16                      | supply voltage                   |
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

class ShiftRegister
{
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

      protected:
          const unsigned int numberOfPins;
          virtual bool testValueAt ( const int &index ) const = 0;

    public:
        ShiftRegister ( PinName _dataPin, PinName _latchPin, PinName _clockPin, PinName _enablePin, PinName _resetPin, const unsigned int totalPinCount = 0 );
        virtual ~ShiftRegister () = 0;

        /**
         * This function sets the Output/Bits of the Shift Register(s).
         * Internally it calls the pure virtual function testValueAt to
         * figure out if the bit at that particular index needs to be
         * HIGH (1) or LOW (0).  Function works in MSB first fashion.
         * @return numberOfHighBits -- Returns the Number of HIGH (1) bits
         * output on the Shift Register Data
         * */
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
