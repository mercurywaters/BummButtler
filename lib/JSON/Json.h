#ifndef __JSON_LIB_CLASS_H_
#define __JSON_LIB_CLASS_H_

#include "jsmn.h"
#include <stdlib.h>
#include <string.h>

/*
 JSON wrapper over JSMN lib
 */

class Json
{
    private:
        const char * source;
        const size_t sourceLength;
        jsmntok_t * tokens;
        int tokenCount;
        Json ( const Json & other );

    public:
        Json ( const char * jsonString, size_t length );
        virtual ~Json ();

        int findKeyIndexIn ( const char * key, const int &parentIndex ) const;
        int findChildIndexOf ( const int &parentIndex, const int &startingAt ) const;
        bool matches ( const int & tokenIndex, const char * value ) const;

        inline bool isValidJson () const;
        inline jsmntype_t type ( const int tokenIndex ) const;
        inline int parent ( const int tokenIndex ) const;
        inline int childCount ( const int tokenIndex ) const;
        inline int tokenLength ( const int tokenIndex ) const;
        inline const char * tokenAddress ( const int tokenIndex ) const;

        int tokenIntegerValue ( const int tokenIndex ) const;
        float tokenNumberValue ( const int tokenIndex ) const;

        inline bool tokenBooleanValue ( const int tokenIndex ) const;

        // void print () const;
};

inline bool Json::isValidJson () const
{
    return ( tokenCount >= 1 );
}

inline jsmntype_t Json::type ( const int tokenIndex ) const
{
    return tokens [ tokenIndex ].type;
}

inline int Json::parent ( const int tokenIndex ) const
{
    return tokens [ tokenIndex ].parent;
}

inline int Json::childCount ( const int tokenIndex ) const
{
    return tokens [ tokenIndex ].childCount;
}

inline int Json::tokenLength ( const int tokenIndex ) const
{
    return tokens [ tokenIndex ].end - tokens [ tokenIndex ].start;
}

inline const char * Json::tokenAddress ( const int tokenIndex ) const
{
    return source + tokens [ tokenIndex ].start;
}

#endif
