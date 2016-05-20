#include "DispenserControl.h"

#define NUMBER_OF_STEPS_PER_REV         200
#define STEPPER_MOTOR_DELAY_MICRO_SECS  250

DispenserControl::DispenserControl ( const PinName homePin, const PinName endPin, const PinName motorStepPin, const PinName motorDirPin )
        : homeSwitchPin ( homePin ), endSwitchPin ( endPin ), dispenserStepPin ( motorStepPin ), dispenserDirPin ( motorDirPin )

{
    inMotion = false;
    atEnd = false;
    atHome = false;
    maximumNumberOfSteps = 0;
    dispenserCurrentPosition = 0;
    listener = NULL;

    goToHome ();

    goToEnd ();
    moveDispenserHead ( (maximumNumberOfSteps / 2), HOME );
}

DispenserControl::~DispenserControl ()
{
}

unsigned int DispenserControl::goToHome ()
{
    unsigned int stepsTaken = 0;
    while ( !isAtHome () )
    {
        stepsTaken += moveDispenserHead ( NUMBER_OF_STEPS_PER_REV, HOME );
    }
    return stepsTaken;
}

unsigned int DispenserControl::goToEnd ()
{
    unsigned int stepsTaken = 0;
    while ( !isAtEnd () )
    {
        stepsTaken += moveDispenserHead ( NUMBER_OF_STEPS_PER_REV, END );
    }
    return stepsTaken;
}

unsigned int DispenserControl::moveDispenserHead ( unsigned int steps, Direction direction )
{
    int delta = 1;
    if ( direction == HOME )
    {
        delta = -delta; // reduce steps moving towards Home
    }

    dispenserDirPin = direction;

    bool stopMotorMovement = false;
    unsigned int stepsTaken = 0;

    inMotion = true;
    while ( ( !stopMotorMovement ) && ( stepsTaken < steps ) )
    {
        testLimitSwitches ();

        if ( isAtHome () && ( direction == HOME ) )
        {
            stopMotorMovement = true;
        }
        else if ( isAtEnd () && ( direction == END ) )
        {
            stopMotorMovement = true;
        }

        if ( !stopMotorMovement )
        {
            dispenserStepPin = ON;
            dispenserStepPin = OFF;
            stepsTaken++;
            dispenserCurrentPosition += delta;
            wait_us ( STEPPER_MOTOR_DELAY_MICRO_SECS ); // To prevent motor from stalling
        }
    }
    inMotion = false;

    return stepsTaken;
}

void DispenserControl::testLimitSwitches ()
{
    if ( homeSwitchPin.read () == ON )
    { // Home Button Pressed
        atHome = true;
        dispenserCurrentPosition = 0;
    }
    else
    {
        atHome = false;
    }

    if ( endSwitchPin.read () == ON )
    { // End Button Pressed
        atEnd = true;
        maximumNumberOfSteps = dispenserCurrentPosition;
    }
    else
    {
        atEnd = false;
    }
}

unsigned int DispenserControl::moveToPosition ( const unsigned int position )
{
    unsigned int stepsTaken = 0;
    if ( !isInMotion () )
    {
        goToHome ();
        stepsTaken = moveDispenserHead ( position, END );

        if ( listener != NULL ){
            listener -> reachedToPosition ( position, stepsTaken );
        }
    }

    return stepsTaken;
}

void DispenserControl::setListener ( DispenserControlListener * &_listener )
{
    listener = _listener;
}

