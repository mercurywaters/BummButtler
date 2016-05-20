#ifndef COMMONS_SERVICESTATUS_H_
#define COMMONS_SERVICESTATUS_H_

#define MAX_STATUS_MESSAGE_LENGTH   100

class ServiceStatus
{
    private:
        int statusCode;
        char message [ MAX_STATUS_MESSAGE_LENGTH ];

    public:
        ServiceStatus ( const int code, const char* format ... );

        ServiceStatus ( const ServiceStatus &other );

        inline const char * getMessage () const;
        inline int getCode () const;
        inline char * toJsonString ( char * buffer ) const;
};

inline const char * ServiceStatus::getMessage () const
{
    return message;
}

inline int ServiceStatus::getCode () const
{
    return statusCode;
}

inline char * ServiceStatus::toJsonString ( char * buffer ) const
{
    sprintf ( buffer, "{\"status\":%d,\"message\":\"%s\"}", statusCode, message );
    return buffer;
}

#endif
