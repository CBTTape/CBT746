
/*****************************Documentation Start***********************************

  NAME: syntblgen.h  - Header file for syntblgen.c


  DESCRIPTION:

  This file contains global constants.



  HISTORY:

  Date        Who Description
  ----------- --- -----------------------------------------------------------------
  2005-Dec-02 RXT Created

******************************Documentation End************************************/


#define AS400                   'A'
#define UNIX                    'U'
#define WINDOWS                 'W'

#define TRUE                    1
#define FALSE                   0
#define ERROR                   1
#define SUCCESS                 0
#define ON                      1
#define OFF                     0
#define YES                     'Y'
#define NO                      'N'
#define BLNK                    ' '

#define FILENAME_LEN            80
#define MAXSTRINGLEN			100
#define MAXTOKENTYPELEN			17
#define MAXEXITNAMELEN			30
#define MAXCOMMENTLEN			40
#define MAX_RULES	            1000
#define MaxSwitches             3  /* Switch and data pairs */

int DEBUG_FLAG = OFF;           /* Initialized to OFF */

struct tokentypetab {	/* table of macro-tokentype and equivalent C-version type */
   char *macro_tokentype; /* macro-tokentype */
   char *C_tokentype;	/* equivalent C-version tokentype */
};

