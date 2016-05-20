#include "jsmn.h"

/**
 * Allocates a fresh unused token from the token pull.
 */
static jsmntok_t *jsmn_alloc_token ( jsmn_parser *parser, jsmntok_t *tokens, size_t num_tokens )
{
    jsmntok_t *tok;
    if ( parser->toknext >= num_tokens )
    {
        return NULL ;
    }
    tok = &tokens [ parser->toknext++ ];
    tok->start = tok->end = -1;
    tok->childCount = 0;
    tok->parent = -1;
    return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token ( jsmntok_t *token, jsmntype_t type, int start, int end )
{
    token->type = type;
    token->start = start;
    token->end = end;
    token->childCount = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive ( jsmn_parser *parser, const char *js, size_t len, jsmntok_t *tokens, size_t num_tokens )
{
    jsmntok_t *token;
    int start;

    start = parser->pos;

    for ( ; parser->pos < len && js [ parser->pos ] != '\0'; parser->pos++ )
    {
        switch ( js [ parser->pos ] )
        {
            case '\t':
            case '\r':
            case '\n':
            case ' ':
            case ',':
            case ']':
            case '}':
                goto found;
        }
        if ( js [ parser->pos ] < 32 || js [ parser->pos ] >= 127 )
        {
            parser->pos = start;
            return JSMN_ERROR_INVAL;
        }
    }
    /* primitive must be followed by a comma/object/array */
    parser->pos = start;
    return JSMN_ERROR_PART;

    found: if ( tokens == NULL )
    {
        parser->pos--;
        return 0;
    }
    token = jsmn_alloc_token ( parser, tokens, num_tokens );
    if ( token == NULL )
    {
        parser->pos = start;
        return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token ( token, JSMN_PRIMITIVE, start, parser->pos );
    token->parent = parser->toksuper;
    parser->pos--;
    return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string ( jsmn_parser *parser, const char *js, size_t len, jsmntok_t *tokens, size_t num_tokens, unsigned char isKey )
{
    jsmntok_t *token;

    int start = parser->pos;

    parser->pos++;

    /* Skip starting quote */
    for ( ; parser->pos < len && js [ parser->pos ] != '\0'; parser->pos++ )
    {
        char c = js [ parser->pos ];

        /* Quote: end of string */
        if ( c == '\"' )
        {
            if ( tokens == NULL )
            {
                return 0;
            }
            token = jsmn_alloc_token ( parser, tokens, num_tokens );
            if ( token == NULL )
            {
                parser->pos = start;
                return JSMN_ERROR_NOMEM;
            }
            if ( isKey == 1 )
            {
                jsmn_fill_token ( token, JSMN_KEY, start + 1, parser->pos );
            }
            else
            {
                jsmn_fill_token ( token, JSMN_STRING, start + 1, parser->pos );
            }
            token->parent = parser->toksuper;
            return 0;
        }

        /* Backslash: Quoted symbol expected */
        if ( c == '\\' && parser->pos + 1 < len )
        {
            int i;
            parser->pos++;
            switch ( js [ parser->pos ] )
            {
                /* Allowed escaped symbols */
                case '\"':
                case '/':
                case '\\':
                case 'b':
                case 'f':
                case 'r':
                case 'n':
                case 't':
                    break;
                    /* Allows escaped symbol \uXXXX */
                case 'u':
                    parser->pos++;
                    for ( i = 0; i < 4 && parser->pos < len && js [ parser->pos ] != '\0'; i++ )
                    {
                        /* If it isn't a hex character we have an error */
                        if ( ! ( ( js [ parser->pos ] >= 48 && js [ parser->pos ] <= 57 ) || /* 0-9 */
                        ( js [ parser->pos ] >= 65 && js [ parser->pos ] <= 70 )
                                 || /* A-F */
                                 ( js [ parser->pos ] >= 97 && js [ parser->pos ] <= 102 ) ) )
                        { /* a-f */
                            parser->pos = start;
                            return JSMN_ERROR_INVAL;
                        }
                        parser->pos++;
                    }
                    parser->pos--;
                    break;
                    /* Unexpected symbol */
                default:
                    parser->pos = start;
                    return JSMN_ERROR_INVAL;
            }
        }
    }
    parser->pos = start;
    return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int jsmn_parse ( jsmn_parser *parser, const char *js, size_t len, jsmntok_t *tokens, unsigned int num_tokens )
{
    int r;
    int i;
    jsmntok_t *token;
    int count = parser->toknext;

    unsigned char isKey = 1;

    for ( ; parser->pos < len && js [ parser->pos ] != '\0'; parser->pos++ )
    {
        char c;
        jsmntype_t type;

        c = js [ parser->pos ];
        switch ( c )
        {
            case '{':
            case '[':
                count++;
                if ( tokens == NULL )
                {
                    break;
                }
                token = jsmn_alloc_token ( parser, tokens, num_tokens );
                if ( token == NULL )
                    return JSMN_ERROR_NOMEM;
                if ( parser->toksuper != -1 )
                {
                    tokens [ parser->toksuper ].childCount++;
                    token->parent = parser->toksuper;
                }
                token->type = ( c == '{' ? JSMN_OBJECT : JSMN_ARRAY );
                token->start = parser->pos;
                parser->toksuper = parser->toknext - 1;
                if ( token->type == JSMN_OBJECT )
                {
                    isKey = 1;
                }
                break;
            case '}':
            case ']':
                if ( tokens == NULL )
                    break;
                type = ( c == '}' ? JSMN_OBJECT : JSMN_ARRAY );
                if ( parser->toknext < 1 )
                {
                    return JSMN_ERROR_INVAL;
                }
                token = &tokens [ parser->toknext - 1 ];
                for ( ;; )
                {
                    if ( token->start != -1 && token->end == -1 )
                    {
                        if ( token->type != type )
                        {
                            return JSMN_ERROR_INVAL;
                        }
                        token->end = parser->pos + 1;
                        parser->toksuper = token->parent;
                        break;
                    }
                    if ( token->parent == -1 )
                    {
                        break;
                    }
                    token = &tokens [ token->parent ];
                }
                break;
            case '\"':
                r = jsmn_parse_string ( parser, js, len, tokens, num_tokens, isKey );
                if ( r < 0 )
                    return r;
                count++;
                if ( parser->toksuper != -1 && tokens != NULL )
                    tokens [ parser->toksuper ].childCount++;
                break;
            case '\t':
            case '\r':
            case '\n':
            case ' ':
                break;
            case ':':
                isKey = 0;
                parser->toksuper = parser->toknext - 1;
                break;
            case ',':
                if ( tokens != NULL && parser->toksuper != -1 && tokens [ parser->toksuper ].type != JSMN_ARRAY && tokens [ parser->toksuper ].type != JSMN_OBJECT )
                {
                    parser->toksuper = tokens [ parser->toksuper ].parent;
                }
                isKey = 1;
                break;
                /* In strict mode primitives are: numbers and booleans */
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 't':
            case 'f':
            case 'n':
                /* And they must not be keys of the object */
                if ( tokens != NULL && parser->toksuper != -1 )
                {
                    jsmntok_t *t = &tokens [ parser->toksuper ];
                    if ( t->type == JSMN_OBJECT || ( t->type == JSMN_STRING && t->childCount != 0 ) )
                    {
                        return JSMN_ERROR_INVAL;
                    }
                }
                r = jsmn_parse_primitive ( parser, js, len, tokens, num_tokens );
                if ( r < 0 )
                    return r;
                count++;
                if ( parser->toksuper != -1 && tokens != NULL )
                    tokens [ parser->toksuper ].childCount++;
                break;

                /* Unexpected char in strict mode */
            default:
                return JSMN_ERROR_INVAL;
        }
    }

    if ( tokens != NULL )
    {
        for ( i = parser->toknext - 1; i >= 0; i-- )
        {
            /* Unmatched opened object or array */
            if ( tokens [ i ].start != -1 && tokens [ i ].end == -1 )
            {
                return JSMN_ERROR_PART;
            }
        }
    }

    return count;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void jsmn_init ( jsmn_parser *parser )
{
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
}
