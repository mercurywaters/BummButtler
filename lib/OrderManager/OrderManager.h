#include "mbed.h"
#include "OrderQueue.h"
#include "PumpControl.h"
#include "DispenserControl.h"

class OrderManager
{
    private:
        const int pumpCount;
        const int cupCount;

        OrderQueue * orderQueue;
        PumpControl * pumpControl;
        DispenserControl * dispenserControl;

        unsigned int * durations;
        int currCupIndex;

        Ticker * orderProcessingTimer;
        void atOrderProcessingTimer ();

        volatile bool semaphoreLock;

        OrderManager ( OrderManager & other );

        void executeNextOrder ();

    public:
        OrderManager ( int _CUP_COUNT, int _PUMP_COUNT, OrderQueue * _orderQueue, PumpControl * _pumpControl, DispenserControl * _dispenserControl );
        virtual ~OrderManager ();

        inline void lock ()
        {
            semaphoreLock = true;
        }
        ;
        inline void release ()
        {
            semaphoreLock = false;
        }
        ;
};
