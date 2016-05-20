#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "ServiceStatus.h"

ServiceStatus::ServiceStatus ( const int code, const char* format... )
{
    statusCode = code;
    va_list argList;
    va_start ( argList, format );
    vsnprintf ( message, MAX_STATUS_MESSAGE_LENGTH, format, argList );
    va_end ( argList);
}

ServiceStatus::ServiceStatus ( const ServiceStatus &other )
{
    statusCode = other.statusCode;
    strcpy ( message, other.message );
}

