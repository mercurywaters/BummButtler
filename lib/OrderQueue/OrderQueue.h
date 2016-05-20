#ifndef BARVIS_ORDER_QUEUE_H_
#define BARVIS_ORDER_QUEUE_H_

#include "mbed.h"

class OrderQueue
{
    private:
        const unsigned int capacity;
        const unsigned int PUMP_OPERATION_SIZE;
        unsigned int * * queue;

        unsigned int head;
        unsigned int tail;
        unsigned int currSize;

        OrderQueue ( OrderQueue & other );

    public:
        OrderQueue ( unsigned int _capacity, unsigned int _pumpCount );
        virtual ~OrderQueue ();

        int addOrder ( unsigned int * runPumpsFor );
        int removeOrder ( unsigned int * runPumpsFor );
        int deleteNextOrder ();

        inline bool isEmpty () const;
        inline bool isFull () const;
        inline int size () const;
        inline int getCapacity () const;

        void print ( char * buffer ) const;
};

inline bool OrderQueue::isEmpty () const
{
    return ( currSize == 0 );
}

inline bool OrderQueue::isFull () const
{
    return ( currSize == capacity );
}

inline int OrderQueue::size () const
{
    return currSize;
}

inline int OrderQueue::getCapacity () const
{
    return capacity;
}

#endif
