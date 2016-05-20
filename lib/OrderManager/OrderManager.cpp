#include "OrderManager.h"

OrderManager::OrderManager ( int _CUP_COUNT, int _PUMP_COUNT, OrderQueue * _orderQueue, PumpControl * _pumpControl, DispenserControl * _dispenserControl )
        : pumpCount ( _PUMP_COUNT ), cupCount ( _CUP_COUNT )
{
    orderQueue = _orderQueue;
    pumpControl = _pumpControl;
    dispenserControl = _dispenserControl;
    semaphoreLock = false;
    currCupIndex = -1;
    durations = new unsigned int [ pumpCount ];

    orderProcessingTimer = new Ticker ();
    orderProcessingTimer->attach_us ( this, &OrderManager::atOrderProcessingTimer, 250000 );
}

OrderManager::OrderManager ( OrderManager & other )
        : pumpCount ( 0 ), cupCount ( 0 )
{
    orderQueue = NULL;
    pumpControl = NULL;
    dispenserControl = NULL;
    currCupIndex = -1;
    durations = NULL;
    orderProcessingTimer = NULL;
}

OrderManager::~OrderManager ()
{
    orderProcessingTimer->detach ();
    delete [] durations;
    delete orderProcessingTimer;
}

void OrderManager::atOrderProcessingTimer ()
{
    if ( semaphoreLock == false )
    {
        orderProcessingTimer->detach ();
        executeNextOrder ();
        orderProcessingTimer->attach_us ( this, &OrderManager::atOrderProcessingTimer, 250000 );
    }
}

void OrderManager::executeNextOrder ()
{
    if ( !orderQueue->isEmpty () )
    {
        if ( pumpControl->getState () == Idle )
        {
            int nextCupIndex = ( currCupIndex + 1 ) % ( cupCount );
//            if ( dispenserControl -> moveToCup ( nextCupIndex ) ) {
//            if ( dispenserControl->isCupPresent ( nextCupIndex ) )
//            {
                orderQueue->removeOrder ( durations );
                pumpControl->runPumpsFor ( durations );
                currCupIndex = nextCupIndex;
//            }
//            }
        }
    }
}
