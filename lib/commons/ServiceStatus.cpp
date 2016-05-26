#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "ServiceStatus.h"

const int ServiceStatus::MAX_STATUS_MESSAGE_LENGTH = 100;

ServiceStatus::ServiceStatus ( const int code, const char* format ... )
{
    statusCode = code;
    message = new char [ MAX_STATUS_MESSAGE_LENGTH ];

    va_list argList;
    va_start ( argList, format );
    vsnprintf ( message, MAX_STATUS_MESSAGE_LENGTH, format, argList );
    va_end ( argList);
}

ServiceStatus::ServiceStatus ( const ServiceStatus &other )
{
    statusCode = other.statusCode;
    message = new char [ MAX_STATUS_MESSAGE_LENGTH ];
    strcpy ( message, other.message );
}

ServiceStatus::~ServiceStatus ()
{
    delete [] message;
}


ServiceStatus * ServiceStatus::status ( const int code, const char * format ... )
{
    statusCode = code;
    va_list argList;
    va_start ( argList, format );
    vsnprintf ( message, MAX_STATUS_MESSAGE_LENGTH, format, argList );
    va_end ( argList);

    return this;
}
