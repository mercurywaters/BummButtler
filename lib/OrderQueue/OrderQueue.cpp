#include "OrderQueue.h"

OrderQueue::OrderQueue ( unsigned int _capacity, unsigned int _pumpCount )
        : capacity ( _capacity ), PUMP_OPERATION_SIZE ( _pumpCount )
{
    queue = new unsigned int * [ capacity ];

    for ( unsigned int i = 0; i < capacity; i++ )
    {
        queue [ i ] = new unsigned int [ PUMP_OPERATION_SIZE ];
    }

    head = 0;
    tail = 0;
    currSize = 0;
}

OrderQueue::OrderQueue ( OrderQueue & other )
        : capacity ( 0 ), PUMP_OPERATION_SIZE ( 0 )
{
    head = 0;
    tail = 0;
    currSize = 0;
    queue = NULL;
}

OrderQueue::~OrderQueue ()
{
    for ( unsigned int i = 0; i < capacity; i++ )
    {
        delete [] queue [ i ];
    }
    delete [] queue;
}

int OrderQueue::addOrder ( unsigned int * runPumpsFor )
{
    if ( !isFull () )
    {

        tail = ( tail + 1 ) % capacity;

        for ( unsigned int i = 0; i < PUMP_OPERATION_SIZE; i++ )
        {
            queue [ tail ] [ i ] = runPumpsFor [ i ];
        }
        currSize++;
    }
    return currSize;
}

int OrderQueue::removeOrder ( unsigned int * runPumpsFor )
{
    if ( !isEmpty () )
    {

        for ( unsigned int i = 0; i < PUMP_OPERATION_SIZE; i++ )
        {
            runPumpsFor [ i ] = queue [ head ] [ i ];
        }

        head = ( head + 1 ) % capacity;
        currSize--;
    }

    return currSize;
}

int OrderQueue::deleteNextOrder ()
{
    if ( !isEmpty () )
    {

        head = ( head + 1 ) % capacity;
        currSize--;
    }

    return currSize;
}

void OrderQueue::print ( char * buffer ) const
{
    int length = 0;
    buffer [ length++ ] = '{';
    int index = tail;
    for ( unsigned int i = 0; i < currSize; i++ )
    {
        buffer [ length++ ] = '[';
        for ( unsigned int j = 0; j < PUMP_OPERATION_SIZE; j++ )
        {
            sprintf ( buffer + length, "(%3d)", queue [ index ] [ j ] );
            length += 5;
        }
        buffer [ length++ ] = ']';
        index = ( index + 1 ) % capacity;
    }
    buffer [ length++ ] = '}';
    buffer [ length ] = '\0';
}
