#ifndef DISPENSER_CONTROL_H
#define DISPENSER_CONTROL_H

#include "mbed.h"

class DispenserControlListener
{
    public:
        virtual ~DispenserControlListener ()
        {
        }
        virtual void reachedToPosition ( const unsigned int position, const unsigned int stepsTaken ) = 0;
};

class DispenserControl
{
    private:
        enum Direction
        {
            HOME = 0,
            END = 1
        };

        enum PinState
        {
            OFF = 0,
            ON = 1
        };

        DigitalIn homeSwitchPin;
        DigitalIn endSwitchPin;
        DigitalOut dispenserStepPin;
        DigitalOut dispenserDirPin;

        bool inMotion;
        bool atEnd;
        bool atHome;
        unsigned int maximumNumberOfSteps;
        int dispenserCurrentPosition;

        DispenserControlListener * listener;

        void testLimitSwitches ();
        unsigned int moveDispenserHead ( const unsigned int steps, Direction direction );

        unsigned int goToHome ();
        unsigned int goToEnd ();

    public:
        DispenserControl ( const PinName homePin, const PinName endPin, const PinName motorStepPin, const PinName motorDirPin );
        virtual ~DispenserControl ();

        unsigned int moveToPosition ( const unsigned int position );
        void setListener ( DispenserControlListener * &_listener );

        inline bool isInMotion () const;
        inline bool isAtHome () const;
        inline bool isAtEnd () const;
//        inline unsigned int getMaxPosition () const;
        inline unsigned int getCurrentPosition () const;
};

inline bool DispenserControl::isInMotion () const
{
    return inMotion;
}

inline bool DispenserControl::isAtHome () const
{
    return atHome;
}

inline bool DispenserControl::isAtEnd () const
{
    return atEnd;
}

//inline unsigned int DispenserControl::getMaxPosition () const
//{
//    return maximumNumberOfSteps;
//}
//
inline unsigned int DispenserControl::getCurrentPosition () const
{
    return dispenserCurrentPosition;
}


#endif
