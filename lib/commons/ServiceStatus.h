#ifndef COMMONS_SERVICESTATUS_H_
#define COMMONS_SERVICESTATUS_H_

class ServiceStatus
{
    private:
        static const int MAX_STATUS_MESSAGE_LENGTH;
        int statusCode;
        char * message;

        ServiceStatus ( const ServiceStatus &other );

    public:
        ServiceStatus ( const int code, const char* format ... );
        virtual ~ServiceStatus ();

        ServiceStatus * status ( const int code, const char * format ... );
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
