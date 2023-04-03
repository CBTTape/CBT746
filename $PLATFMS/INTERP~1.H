
/*****************************Documentation Start***********************************

  NAME: interpret.h  - Header file for interpret.c and user code that invokes
                       interpret.c


  DESCRIPTION:

  This file contains definitions used by interpret.c, and callers of interpret.c



  HISTORY:

  Date        Who Description
  ----------- --- -----------------------------------------------------------------
  2004-Jun-01 RXT Created
  2005-Jul-08 RXT Add additional parameters - pointer to token, token length and
                  error message buffer

******************************Documentation End************************************/

#define INTERPRET_CONTEXT_ERROR 1
#define INTERPRET_PARM_IS_NULL  2

#define MSG_BUFF_LEN            101

extern int interpret(char cmdline[],                  /* String to be interpreted */
                     struct syntax_table *syntaxtab,  /* Table of syntax rules    */
                     int *tokenloc,                   /* Index value of bad token */
                     int *tokenlen,                   /* Length of bad token      */
                     char errmsg[101]);               /* Error message            */

