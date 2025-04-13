
/*****************************Documentation Start***********************************

  NAME: parse.h  - Header file for parser.c and user code that invokes parser.c


  DESCRIPTION:

  This file contains definitions used by parser.c, and callers of parser.c


  NOTE: parser.c must specify #define PARSE  1 to ensure the global variables it
        uses are exposed.


  HISTORY:

  Date        Who Description
  ----------- --- -----------------------------------------------------------------
  2004-Jun-01 RXT Created
  2005-Jul-08 RXT Add specific return codes for each error type

******************************Documentation End************************************/

#define PARSE_PARM_IS_NULL          1
#define PARSE_STRING_LEN_INVALID    2

#define YES             'Y'
#define NO              'N'
#define TOGGLE			'T'
#define TOGGLE_ON       1
#define DEFAULT_DELIMS  dfltdelim
#define MAXSTRINGLEN    100
#define MAX_KEYWORD_LEN 100

#define PTDATA          100
#define PTNUM           101
#define PEOS            255
#define PTNULL          '\0'
#define PTPAD           ' '
#define PTCOMMA         ','
#define PTEQUAL         '='
#define PTLPAREN        '('
#define PTRPAREN        ')'
#define PTLT            '<'
#define PTGT            '>'
#define PTLBRACE        '{'
#define PTRBRACE        '}'
#define PTDASH          '-'
#define PTUBAR          '_'
#define PTAND           '&'
#define PTPOUND         '#'
#define PTAT            '@'
#define PTPLUS          '+'
#define PTSLASH         '/'
#define PTPERCENT       '%'
#define PTSTAR          '*'
#define PTSCOLON        ';'
#define PTCOLON         ':'
#define PTSQUOTE        '\''
#define PTDQUOTE        '"'
#define PTTILDE         '~'
#define PTBAR           '|'
#define PTQUEST         '?'
#define PTPERIOD        '.'
#define PTEXCLAM        '!'
#define PTBSLASH        '\\'
#define PTDOLLAR        '$'
#define PTRVQUOT        '`'
#define PTLBRACKET      '['
#define PTRBRACKET      ']'

#if PARSE != 1
extern char dfltdelim[33][2];

extern struct parsework {  /* Stack of parsed tokens */
          int tokentype;   /* token type flag        */
          int tokenlen;    /* token length           */
          int tokenloc;    /* token offset in string */
} tokenstack[MAXSTRINGLEN+1];
#endif

extern int parse(char   cmdline[],                   /* String to be parsed */
                 char   delimiterlist[][2]);         /* List of delimiters  */
