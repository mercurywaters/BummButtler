// comment the following define statement to shut the built-in LED off
#define USE_DEBUG_LED // set debug LED
// comment the following define statement to shut the serial debug off
#define SERIAL_DEBUG // set debug mode

#include "mbed.h"
#include "PumpControl.h"
#include "DispenserControl.h"
#include "hm11.h"
#include "Json.h"
#include "ServiceStatus.h"
#include "OrderQueue.h"
#include "OrderManager.h"
#include "USBSerial.h"
#include "string.h"

#ifdef USE_DEBUG_LED
DigitalOut debugLed ( LED1 );
Ticker debugLedTimer;
void atDebugLedTimer ()
{
    if ( debugLed.read () == 0 )
    {
        debugLed = 1;
    }
    else
    {
        debugLed = 0;
    }
}
#define setupDebugLed() do{debugLedTimer.attach_us(atDebugLedTimer,250000);}while(0)
#else
#define setupDebugLed()
#endif

#ifdef SERIAL_DEBUG
USBSerial * SERIAL_DEBUG_OUT = NULL;
//#define error(code,fmt,...) if(SERIAL_DEBUG_OUT!=NULL){SERIAL_DEBUG_OUT->printf("[ERROR] ");SERIAL_DEBUG_OUT->printf(##__VA_ARGS__);SERIAL_DEBUG_OUT->printf("\n\r");}
#define debug(fmt,...) if(SERIAL_DEBUG_OUT!=NULL){SERIAL_DEBUG_OUT->printf("[DEBUG] ");SERIAL_DEBUG_OUT->printf(fmt,##__VA_ARGS__);SERIAL_DEBUG_OUT->printf("\n\r");}
#else
#define debug(fmt,...)
//#define error(code,fmt,...)
#endif

using namespace std;

typedef enum
{
    SUCCESS = 0,
    UNKNOWN_ERROR = 0xFF00,
    ERROR_JSON = 0xC000, // 1100000000000000
    ERROR_JSON_INVALID_OBJECT = ( ERROR_JSON | 0x01 ),
    ERROR_JSON_INVALID_VALUE_TYPE = ( ERROR_JSON | 0x02 ),
    ERROR_JSON_MISSING_ATTRIBUTE = ( ERROR_JSON | 0x03 ),
    ERROR_PUMP = 0xA000, // 1010000000000000
    ERROR_PUMP_BUSY = ( ERROR_PUMP | 0x01 ),
    ERROR_PUMP_PAUSED = ( ERROR_PUMP | 0x02 ),
    ERROR_PUMP_INVALID_ID = ( ERROR_PUMP | 0x03 ),
    ERROR_PUMP_INVALID_DURATION = ( ERROR_PUMP | 0x04 ),
    ERROR_ORDER = 0x9000, // 1001000000000000
    ERROR_ORDER_QUEUE_FULL = ( ERROR_ORDER | 0x01 ),
} StatusCode;

ServiceStatus * executeCommand ( char * jsonCommand, int commandLength, OrderManager * orderManager, PumpControl * pumpControl, HM11 * &ble, OrderQueue * orderQueue );

#define BARVIS_COMMAND_SIZE    1024
#define TOTAL_CUPS             1
#define TOTAL_PUMPS            24

int sendBleATCommand ( HM11 * &ble, const char * command, char * &responseBuffer, const int bufferSize )
{
    ble->sendDataToDevice ( command );
    ble->waitForData ( 1000 );
    int dataLength = ble->copyAvailableDataToBuf ( responseBuffer, bufferSize );
    debug( "Got a response for [%s] as: %s", command, responseBuffer );
    return dataLength;
}

// ------------------- PIN Names and configuration for all the Pins used --------------- //

#define BLE_TX  D1
#define BLE_RX  D0

#define PUMP_CONTROL_DATA           D2  // 75HC595 Pin 14 - Blue
#define PUMP_CONTROL_LATCH          D3  // 75HC595 Pin 12 - Green
#define PUMP_CONTROL_CLOCK          D4  // 75HC595 Pin 11 - Yellow
#define PUMP_CONTROL_ENABLE         D5  // 75HC595 Pin 13 - White
#define PUMP_CONTROL_RESET          D6  // 75HC595 Pin 10 - Grey/Gold
#define PUMP_CONTROL_CUP_DETECTOR   D14 // IR Sensor Pin0

#define DISPENSER_CONTROL_HOME  D8
#define DISPENSER_CONTROL_END   D9
#define DISPENSER_MOTOR_STEP    D12
#define DISPENSER_MOTOR_DIR     D11

void pumpDurationsDebugString ( char * buffer, unsigned int * durations );

void increment ( unsigned int * &array, const int index )
{
    if ( index >= 0 && index < TOTAL_PUMPS )
    {
        if ( array [ index ] == 0 )
        {
            array [ index ] = 1;
        }
        else
        {
            array [ index ] = 0;
            increment ( array, index + 1 );
        }
    }
}

int main ()
{
    setupDebugLed()
    ;

    USBSerial usbSerial ( USBTX, USBRX );
    SERIAL_DEBUG_OUT = &usbSerial;

//    PinName             irSensorPins [ TOTAL_CUPS ] = { D14, D15, D16, D17 };

    PumpControl * pumpControl = new PumpControl ( PUMP_CONTROL_DATA, PUMP_CONTROL_LATCH, PUMP_CONTROL_CLOCK, PUMP_CONTROL_ENABLE, PUMP_CONTROL_RESET, TOTAL_PUMPS );
    DispenserControl * dispenserControl = new DispenserControl ( DISPENSER_CONTROL_HOME, DISPENSER_CONTROL_END, DISPENSER_MOTOR_STEP, DISPENSER_MOTOR_DIR );
    OrderQueue * orderQueue = new OrderQueue ( TOTAL_CUPS, TOTAL_PUMPS );
    HM11 * ble = new HM11 ( BLE_TX, BLE_RX );
    IrSensorPin * cupDetectorPin = new IrSensorPin ( PUMP_CONTROL_CUP_DETECTOR, 0, pumpControl );

    char * commandBuffer = new char [ BARVIS_COMMAND_SIZE ];

    sendBleATCommand ( ble, "AT", commandBuffer, BARVIS_COMMAND_SIZE );
    sendBleATCommand ( ble, "AT+NOTI0", commandBuffer, BARVIS_COMMAND_SIZE );
    sendBleATCommand ( ble, "AT+ROLE0", commandBuffer, BARVIS_COMMAND_SIZE );
    sendBleATCommand ( ble, "AT+RESET", commandBuffer, BARVIS_COMMAND_SIZE );
    sendBleATCommand ( ble, "AT+SHOW1", commandBuffer, BARVIS_COMMAND_SIZE );
    sendBleATCommand ( ble, "AT+IMME1", commandBuffer, BARVIS_COMMAND_SIZE );
    sendBleATCommand ( ble, "AT+NAMEBummButtler", commandBuffer, BARVIS_COMMAND_SIZE );

    OrderManager * orderManager = new OrderManager ( TOTAL_CUPS, TOTAL_PUMPS, orderQueue, pumpControl, dispenserControl );

    ServiceStatus * status = NULL;

//    unsigned int * testDurations = new unsigned int [ TOTAL_PUMPS ];
//    for ( int i = 0; i < TOTAL_PUMPS; i ++ )
//    {
//        testDurations [ i ] = 0;
//    }
//    testDurations [ 7 ] = 1;
//    testDurations [ 6 ] = 1;
//    testDurations [ 5 ] = 1;
//    testDurations [ 4 ] = 1;
//    testDurations [ 3 ] = 1;
//    testDurations [ 2 ] = 1;

    char debugBuffer [ 256 ];

    int count = 0;
    while ( true )
    {

        if ( ble->isRxDataAvailable () )
        {
            int length = ble->copyAvailableDataToBufWithTimeout ( commandBuffer, BARVIS_COMMAND_SIZE, 10 );
            status = executeCommand ( commandBuffer, length, orderManager, pumpControl, ble, orderQueue );
            status -> toJsonString ( commandBuffer );
            debug( commandBuffer );
            ble->sendDataToDevice ( commandBuffer );
        }
        else if ( usbSerial.available () )
        {
            usbSerial.gets ( commandBuffer, BARVIS_COMMAND_SIZE );
            status = executeCommand ( commandBuffer, strlen ( commandBuffer ), orderManager, pumpControl, ble, orderQueue );
            status -> toJsonString ( commandBuffer );
            debug( commandBuffer );
            ble->sendDataToDevice ( commandBuffer );
        }

        wait_us ( 100000 );

        /*
         {"type":"PUMP","run_pumps":[{"id":1,"for":40},{"id":2,"for":60}]}
         {\"type\":\"PUMP\",\"run_pumps\":[{\"id\":1,\"for\":40},{\"id\":2,\"for\":60}]}
         {"type":"CLEAR"}
         {\"type\":\"CLEAR\"}
         {"type":"PAUSE"}
         {\"type\":\"PAUSE\"}
         {"type":"RESUME"}
         {\"type\":\"RESUME\"}
         */

        {
            wait ( 2 );
            strcpy ( commandBuffer, "{\"type\":\"PUMP\",\"run_pumps\":[{\"id\":1,\"for\":40},{\"id\":2,\"for\":60}]}" );
            status = executeCommand ( commandBuffer, strlen ( commandBuffer ), orderManager, pumpControl, ble, orderQueue );
            status -> toJsonString ( commandBuffer );
            debug( commandBuffer );
        }
        {
            wait ( 2 );
            strcpy ( commandBuffer, "{\"type\":\"PAUSE\"}" );
            status = executeCommand ( commandBuffer, strlen ( commandBuffer ), orderManager, pumpControl, ble, orderQueue );
            status -> toJsonString ( commandBuffer );
            debug( commandBuffer );
        }
        {
            wait ( 2 );
            strcpy ( commandBuffer, "{\"type\":\"RESUME\"}" );
            status = executeCommand ( commandBuffer, strlen ( commandBuffer ), orderManager, pumpControl, ble, orderQueue );
            status -> toJsonString ( commandBuffer );
            debug( commandBuffer );
        }
        {
            wait ( 2 );
            strcpy ( commandBuffer, "{\"type\":\"CLEAR\"}" );
            status = executeCommand ( commandBuffer, strlen ( commandBuffer ), orderManager, pumpControl, ble, orderQueue );
            status -> toJsonString ( commandBuffer );
            debug( commandBuffer );
        }
        wait ( 5 );
    }
}

void pumpDurationsDebugString ( char * buffer, unsigned int * durations )
{
    int length = 0;
    buffer [ length++ ] = '{';
    buffer [ length++ ] = '[';
    for ( int i = ( TOTAL_PUMPS - 1 ); i >= 0; i-- )
    {
        sprintf ( buffer + length, "(%3d)", durations [ i ] );
        length += 5;
    }
    buffer [ length++ ] = ']';
    buffer [ length++ ] = '}';
    buffer [ length ] = '\0';
}

/*
 JSON Structure for Barvis Commands
 {
 "type" : { "AT" | "PUMP" | "SET" | "CLEAR" | "PING" },
 "at_cmd" : "<ATCMD>",
 "run_pumps" : [ { "id" : <pumpID>, "for" : <runForUnits> }, ...  ]
 "set" : [ { "key" : "value" }, { "key2" : "value2" } ... ]
 }
 */

#define JSON_ROOT_INDEX         0
#define JSON_KEY_TYPE           "type"
#define JSON_ENUM_TYPE_PING     "PING"
#define JSON_ENUM_TYPE_PUMP     "PUMP"
#define JSON_ENUM_TYPE_SET      "SET"
#define JSON_ENUM_TYPE_CLEAR    "CLEAR"
#define JSON_ENUM_TYPE_PAUSE    "PAUSE"
#define JSON_ENUM_TYPE_RESUME   "RESUME"
#define JSON_ENUM_TYPE_AT       "AT"
#define JSON_KEY_RUN_PUMPS      "run_pumps"
#define JSON_KEY_RUN_PUMPS_ID   "id"
#define JSON_KEY_RUN_PUMPS_FOR  "for"
#define JSON_KEY_AT_CMD         "at_cmd"
#define JSON_KEY_SET            "set"

ServiceStatus * executeCommand ( char * jsonCommand, int commandLength, OrderManager * orderManager, PumpControl * pumpControl, HM11 * &ble, OrderQueue * orderQueue )
{
    static ServiceStatus * serviceStatus = new ServiceStatus ( SUCCESS, "Nothing executed and no error occurred" );

    debug( "Executing %s", jsonCommand );

    Json json ( jsonCommand, commandLength );

    if ( !json.isValidJson () )
    {
        return serviceStatus -> status ( ERROR_JSON_INVALID_OBJECT, "Invalid JSON ... structure is not a JSON Object" );
    }

    if ( json.type ( JSON_ROOT_INDEX ) != JSMN_OBJECT )
    { // outher object
        return serviceStatus -> status ( ERROR_JSON_INVALID_OBJECT, "Invalid JSON ... structure is not a JSON Object" );
    }

    int typeIndex = json.findKeyIndexIn ( JSON_KEY_TYPE, JSON_ROOT_INDEX );
    if ( typeIndex == -1 )
    {
        return serviceStatus -> status ( ERROR_JSON_MISSING_ATTRIBUTE, "Invalid JSON ... '%s' does not exist as root attribute", JSON_KEY_TYPE );
    }

    // compare with the 'value' of the Key ... i.e. ( typeIndex + 1 )
    int typeValueIndex = json.findChildIndexOf ( typeIndex, -1 );
    if ( typeValueIndex == -1 )
    {
        return serviceStatus -> status ( ERROR_JSON_MISSING_ATTRIBUTE, "Invalid JSON ... '%s' should have a value", JSON_KEY_TYPE );
    }

    if ( json.matches ( typeValueIndex, JSON_ENUM_TYPE_PING ) )
    {
        return serviceStatus -> status ( SUCCESS, "PONG" );
    }
    else if ( json.matches ( typeValueIndex, JSON_ENUM_TYPE_PUMP ) )
    {

        int runPumpsIndex = json.findKeyIndexIn ( JSON_KEY_RUN_PUMPS, JSON_ROOT_INDEX );
        if ( runPumpsIndex == -1 )
        {
            return serviceStatus -> status ( ERROR_JSON_INVALID_OBJECT, "Invalid JSON ... '%s' should exist", JSON_KEY_RUN_PUMPS );
        }

        int runPumpsArrayIndex = json.findChildIndexOf ( runPumpsIndex, 0 ); // get the first child i.e. value of the "run_pumps" KEY
        if ( runPumpsArrayIndex == -1 )
        {
            return serviceStatus -> status ( ERROR_JSON_MISSING_ATTRIBUTE, "Invalid JSON ... '%s' should have an array element", JSON_KEY_RUN_PUMPS );
        }
        if ( json.type ( runPumpsArrayIndex ) != JSMN_ARRAY )
        {
            return serviceStatus -> status ( ERROR_JSON_INVALID_VALUE_TYPE, "Invalid JSON ... '%s' arry should be none other than array type", JSON_KEY_RUN_PUMPS );
        }

        const int existingQueueSize = orderQueue->size ();
        int numberOfInstructions = json.childCount ( runPumpsArrayIndex );
        int childIndex = -1; // start iterating from the begining of the array
        unsigned int runPumpsFor [ TOTAL_PUMPS ] = { 0 };

        for ( int i = 0; i < numberOfInstructions; i++ )
        {

            childIndex = json.findChildIndexOf ( runPumpsArrayIndex, childIndex );
            if ( json.type ( childIndex ) != JSMN_OBJECT )
            {
                return serviceStatus -> status ( ERROR_JSON_INVALID_VALUE_TYPE, "Invalid JSON ... '%s' array should have all 'object' elements", JSON_KEY_RUN_PUMPS );
            }

            int idIndex = json.findKeyIndexIn ( JSON_KEY_RUN_PUMPS_ID, childIndex );
            if ( idIndex == -1 )
            {
                return serviceStatus -> status ( ERROR_JSON_MISSING_ATTRIBUTE, "Invalid JSON ... object at %d is missing '%s' key", i, JSON_KEY_RUN_PUMPS_ID );
            }

            int durationIndex = json.findKeyIndexIn ( JSON_KEY_RUN_PUMPS_FOR, childIndex );
            if ( durationIndex == -1 )
            {
                return serviceStatus -> status ( ERROR_JSON_MISSING_ATTRIBUTE, "Invalid JSON ... object at %d is missing '%s' key", i, JSON_KEY_RUN_PUMPS_FOR );
            }
            // Now as we have both the keys of 'id' and 'for', get the values out

            int idValueIndex = json.findChildIndexOf ( idIndex, 0 );
            if ( idValueIndex == -1 )
            {
                return serviceStatus -> status ( ERROR_JSON_INVALID_VALUE_TYPE, "Invalid JSON ... object at %d: '&s' value is missing", i, JSON_KEY_RUN_PUMPS_ID );
            }
            if ( ( json.type ( idValueIndex ) != JSMN_PRIMITIVE ) )
            {
                return serviceStatus -> status ( ERROR_JSON_INVALID_VALUE_TYPE, "Invalid JSON ... object at %d: '%s' should have integer value", i, JSON_KEY_RUN_PUMPS_ID );
            }

            int durationValueIndex = json.findChildIndexOf ( durationIndex, 0 );
            if ( durationValueIndex == -1 )
            {
                return serviceStatus -> status ( ERROR_JSON_INVALID_VALUE_TYPE, "Invalid JSON ... object at %d: '&s' value is missing", i, JSON_KEY_RUN_PUMPS_FOR );
            }
            if ( json.type ( durationValueIndex ) != JSMN_PRIMITIVE )
            {
                return serviceStatus -> status ( ERROR_JSON_INVALID_VALUE_TYPE, "Invalid JSON ... object at %d: '%s' should have integer value", i, JSON_KEY_RUN_PUMPS_FOR );
            }

            int pumpId = json.tokenIntegerValue ( idValueIndex );
            if ( !pumpControl->isValidId ( pumpId ) )
            {
                return serviceStatus -> status ( ERROR_PUMP_INVALID_ID, "Invalid ID: %d provided for Instruction: %d", pumpId, i );
            }

            int duration = json.tokenIntegerValue ( durationValueIndex );
            if ( !pumpControl->isValidDuration ( duration ) )
            {
                return serviceStatus -> status ( ERROR_PUMP_INVALID_DURATION, "Invalid Duration: %d provided for Instruction: %d", duration, i );
            }

            runPumpsFor [ pumpId ] = (unsigned int) duration;
        }

//        pumpControl -> runPumpsFor ( runPumpsFor );

        orderManager->lock ();
        // Queue the Pump Operation now
        int currSize = orderQueue->addOrder ( runPumpsFor );
        char debugBuffer [ 250 ];
        orderQueue->print ( debugBuffer );
        debug( debugBuffer );
        orderManager->release ();

        if ( currSize > existingQueueSize )
        {
            return serviceStatus -> status ( SUCCESS, "Command queued at %d of %d", currSize, orderQueue->getCapacity () );
        }
        else
        {
            return serviceStatus -> status ( ERROR_ORDER_QUEUE_FULL, "Command NOT accepted. Orders exist %d of %d", currSize, orderQueue->getCapacity () );
        }
    }
    else if ( json.matches ( typeValueIndex, JSON_ENUM_TYPE_CLEAR ) )
    {
        pumpControl->resetPumps ();
        return serviceStatus -> status ( SUCCESS, "All Pumps reset" );
    }
    else if ( json.matches ( typeValueIndex, JSON_ENUM_TYPE_PAUSE ) )
    {
        pumpControl->pausePumps ();
        return serviceStatus -> status ( SUCCESS, "Pumps Paused" );
    }
    else if ( json.matches ( typeValueIndex, JSON_ENUM_TYPE_RESUME ) )
    {
        pumpControl->resumePumps ();
        return serviceStatus -> status ( SUCCESS, "Pumps Resumed" );
    }
    else if ( json.matches ( typeValueIndex, JSON_ENUM_TYPE_SET ) )
    {

    }
    else if ( json.matches ( typeValueIndex, JSON_ENUM_TYPE_AT ) )
    {
        int atCmdIndex = json.findKeyIndexIn ( JSON_KEY_AT_CMD, JSON_ROOT_INDEX );
        if ( atCmdIndex == -1 )
        {
            return serviceStatus -> status ( ERROR_JSON_INVALID_OBJECT, "Invalid JSON ... '%s' should exist", JSON_KEY_AT_CMD );
        }

        int atCmdValueIndex = json.findChildIndexOf ( atCmdIndex, -1 );
        if ( atCmdValueIndex == -1 )
        {
            return serviceStatus -> status ( ERROR_JSON_MISSING_ATTRIBUTE, "Invalid JSON ... '%s' can't have empty value", JSON_KEY_AT_CMD );
        }

        int cmdLen = json.tokenLength ( atCmdValueIndex );
        char * atCommand = new char [ cmdLen + 1 ];
        strncpy ( atCommand, json.tokenAddress ( atCmdValueIndex ), cmdLen );
        atCommand [ cmdLen ] = 0;
        char * atResponse = new char [ 32 ];
        sendBleATCommand ( ble, atCommand, atResponse, BARVIS_COMMAND_SIZE );
        serviceStatus -> status ( SUCCESS, "[%s] response: [%s]", atCommand, atResponse );
        delete [] atCommand;
        delete [] atResponse;
        return serviceStatus;
    }

    return serviceStatus;
}
