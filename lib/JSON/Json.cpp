#include "Json.h"

Json::Json ( const char * jsonString, size_t length )
        : source ( jsonString ), sourceLength ( length )
{
    jsmn_parser parser;
    int count = 100; //jsmn_parse ( &parser, jsonString, length, NULL, 16384 );
    tokens = new jsmntok_t [ count ];

    jsmn_init ( &parser );
    tokenCount = jsmn_parse ( &parser, jsonString, length, tokens, count );
}

Json::Json ( const Json & other )
        : source ( NULL ), sourceLength ( 0 )
{
    tokenCount = 0;
    tokens = NULL;
}

Json::~Json ()
{
    delete [] tokens;
}

int Json::findKeyIndexIn ( const char * key, const int &parentIndex ) const
{
    int retVal = -1;

    if ( parentIndex != -1 && parentIndex < tokenCount )
    {

        for ( int i = parentIndex + 1; i < tokenCount; i++ )
        {

            jsmntok_t t = tokens [ i ];

            if ( t.end >= tokens [ parentIndex ].end )
            {
                break;
            }

            if ( ( t.type == JSMN_KEY ) && ( t.parent == parentIndex ) )
            {
                size_t keyLength = (size_t) ( t.end - t.start );
                if ( ( strlen ( key ) == keyLength ) && ( strncmp ( source + t.start, key, keyLength ) == 0 ) )
                {
                    retVal = i;
                    break;
                }
            }
        }
    }

    return retVal;
}

int Json::findChildIndexOf ( const int &parentIndex, const int &startingAt ) const
{
    int retVal = -1;

    if ( parentIndex != -1 && parentIndex < tokenCount )
    {

        jsmntype_t type = tokens [ parentIndex ].type;
        if ( ( type == JSMN_KEY ) || ( type == JSMN_OBJECT ) || ( type == JSMN_ARRAY ) )
        {
            int i = startingAt + 1;
            if ( startingAt < 0 )
            {
                i = 0;
            }

            for ( ; i < tokenCount; i++ )
            {
                if ( tokens [ i ].parent == parentIndex )
                {
                    retVal = i;
                    break;
                }
            }
        }
    }

    return retVal;
}

bool Json::matches ( const int & tokenIndex, const char * value ) const
{
    jsmntok_t token = tokens [ tokenIndex ];
    return ( strncmp ( source + token.start, value, ( token.end - token.start ) ) == 0 );
}

int Json::tokenIntegerValue ( const int tokenIndex ) const
{
    if ( type ( tokenIndex ) == JSMN_PRIMITIVE )
    {
        int len = tokenLength ( tokenIndex );
        char * tok = new char [ len + 1 ];
        strncpy ( tok, tokenAddress ( tokenIndex ), len );
        tok [ len ] = 0;
        int retVal = atoi ( tok );
        delete [] tok;
        return retVal;
    }
    return -1;
}

float Json::tokenNumberValue ( const int tokenIndex ) const
{
    if ( type ( tokenIndex ) == JSMN_PRIMITIVE )
    {
        int len = tokenLength ( tokenIndex );
        char * tok = new char [ len + 1 ];
        strncpy ( tok, tokenAddress ( tokenIndex ), len );
        tok [ len ] = 0;
        float retVal = atof ( tok );
        delete [] tok;
        return retVal;
    }
    return -1;
}

inline bool Json::tokenBooleanValue ( const int tokenIndex ) const
{
    if ( type ( tokenIndex ) == JSMN_PRIMITIVE )
    {
        return matches ( tokenIndex, "true" );
    }
    return false;
}


// void Json::print () const
// {
//     #ifdef SOFTWARE_DEBUG
//         const char * TYPES [] = {
//             "UNDEFINED",
//             "OBJECT   ",
//             "ARRAY    ",
//             "STRING   ",
//             "PRIMITIVE",
//             "KEY      "
//         };
//
//         for ( int i = 0; i < tokenCount; i ++ ) {
//             debug ( "Index: %3d, Type:%d(%s), Indices: (%3d to %3d), ParentIndex: %3d, ChildCount: %3d    Data: %.*s", i, tokens [ i ].type, TYPES [ tokens [ i ].type ], tokens [ i ].start, tokens [ i ].end, tokens [ i ].parent, tokens [ i ].childCount, tokens [ i ].end - tokens [ i ].start, source + tokens [ i ].start );
//         }
//         debug ( "" );
//     #endif
// }
