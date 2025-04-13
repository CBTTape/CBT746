#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parse.h>
#include <syntaxchk.h>
#include <interpret.h>

#define TRUE        1
#define FALSE       0
#define ERROR       1
#define SUCCESS     0

/*****************************Documentation Start***********************************

  NAME: interpret.c  - Interpret tokens created by parse.c


  DESCRIPTION:
 
  This program "inteprets" the tokens created by parse.c, by following the path
  dictated by the syntax rules.  But, instead checking the syntax of the input 
  string, interpret.c looks for user-specified exits to call.  When an user-exit is
  found in the syntax table, a function call is made to it, passing the location 
  and length of the associated token.  This provides a means to extract a parsed
  value to user code, so that it can perform additional operations on it, e.g.
  range checking, context checking, storing the data for later, and so forth.

  NOTE: it is assumed that the input string was syntactically checked by 
        syntaxchk.c before invoking interpret.c


  ENTRY: From caller

  EXIT:  Return to caller


  INPUT:    char cmdline[]      - character array containing the string
        struct syntax_table *syntaxtab - set of user-provided "rules"

                The basic format of a "rule" is as follows:

                ----------------------------------
                | token | next | keyword  | user |
                | type  | rule | to match | exit |
                ----------------------------------

                For detailed information regarding syntax rules and how to
                to construct them, see:

                  Parsing, Syntax Checking and Interpreting - User's Guide


  OUTPUT:   none


  RETURNS:  0 = Success
            1 = Context error detected by user exit
            2 = Parameter is null


  *--PROPRIETARY STATEMENT--------------------------------------------------------*
  *   This program is the sole property of Richard Tsujimoto, Inc.                *
  *-------------------------------------------------------------------------------*


  HISTORY:

  Date        Who Description
  ----------- --- -----------------------------------------------------------------
  2004-May-28 RXT Created
  2005-Mar-03 RXT Corrected reference to address of user exit in section of code
                  that finds a keyword value match, e.g. keywd1=ABC, where ABC is
                  defined as a specified value to match in the syntax table
  2005-Jul-08 RXT Add support for token types STARTRULE and GOTORULE
                  Add support for error message buffer
                  Check if any parameter is NULL

******************************Documentation End************************************/

/***************************** global vars ****************************************/

int fail_rc;            /* Error rc returned from user exit */
int contrule2_loc = -1; /* Loc of syntax rule to execute due to statement 
                           continuation */

/***************************** end of global vars *********************************/


int interpret(char cmdline[],                  /* String to be checked     */
              struct syntax_table *syntaxtab,  /* Table of syntax rules    */
              int *errtokenloc,                /* Index value of bad token */
              int *errtokenlen,                /* Length of bad token      */
              char errmsg[101])
{
  char token[MAX_KEYWORD_LEN];  /* extracted token from cmdline      */

  int ERR_FLAG;
  int tokenlen;
  int i;
  int j;
  int k;
  int m;
  int rc;

  int (*fp) (char token[], int tokenlen, char *errmsg);   /* function ptr to call user exit */

  /**********************/
  /* Start of interpret */
  /**********************/

  if ((cmdline == NULL) || (syntaxtab == NULL)   || /* Any parms set to NULL? */
      (errmsg == NULL)  || (errtokenloc == NULL) || (errtokenlen == NULL))
     return(INTERPRET_PARM_IS_NULL);

  ERR_FLAG = FALSE; /* Reset error flag */
  i = 0; /* Point to start of tokenstack */
  j = 0; /* Point to start of syntax table */

  if (contrule2_loc != -1) { /* We're in the midst of a continued statement */
     j = contrule2_loc; /* Point to rule to resume interpretation           */
     contrule2_loc = -1;
  } /* end if */

  /* Interpret input string */
  while ((syntaxtab[j].tokentype != LASTRULE) && (ERR_FLAG == FALSE)) {

     if (syntaxtab[j].tokentype > SYSTEM_TOKENS ) { /* Process internal type token */

         switch (syntaxtab[j].tokentype) { /* Internal token type */

            case DUMMYRULE: /* Ignore and get next rule */
               j++;  /* Point to next syntax rule */
               break;

            case STARTRULE: /* Ignore and get next rule */
               j++;  /* Point to next syntax rule */
               break;

            case CONTRULE:  /* Process statement continuation */
               contrule2_loc = syntaxtab[j].nextrule;  /* Save index of continuation rule */
               j++;  /* Point to next syntax rule */
               break;

            case GOTORULE:  /* Process GOTO rule */
               j = syntaxtab[j].nextrule;  /* Point to "next rule" */
               break;

         } /* end switch */

     } /* end if */
     else { /* Process delimiter type token */

        if (tokenstack[i].tokentype == syntaxtab[j].tokentype) { /* Do tokentypes match? */

           if (syntaxtab[j].keyword != NULL) { /* Is there a keyword to be matched? */
              memset(token, '\0', sizeof(token));
              k = tokenstack[i].tokenloc;

              for (m = 0; m < tokenstack[i].tokenlen; ++m) { /* Extract token from cmdline */
                 token[m] = cmdline[k];
                 k++;
              } /* end for */

              if (!(strcmp(token, syntaxtab[j].keyword))) { /* Do we have a keyword match? */

                 if (syntaxtab[j].userexit != NULL) { /* Is there a user exit? */
                    tokenlen = tokenstack[i].tokenlen;
                    memset(errmsg, '\0', MSG_BUFF_LEN);

                    fp = syntaxtab[j].userexit; /* Get address of user exit */

                    rc = (*fp)(token, tokenlen, errmsg); /* Call user exit */

                    if (rc) { /* Save error rc */
                       *errtokenloc = tokenstack[i].tokenloc; /* Store offset of bad token */
                       *errtokenlen = tokenlen;               /* Store length of bad token */
                       ERR_FLAG = TRUE;
                    } /* end if */

                 } /* end if */

                 i++; /* Point to the next tokenstack entry */
                 j = syntaxtab[j].nextrule; /* Point to the next syntax rule */

              } /* end if */
              else { /* Keywords do not match */
                 j++; /* Point to the next syntax rule */
              } /* end else */
           
           } /* end if */
           else { /* No keyword match required */

              if (syntaxtab[j].userexit != NULL) { /* Is there a user exit to invoke? */
                 memset(token, '\0', sizeof(token));
                 k = tokenstack[i].tokenloc;

                 for (m = 0; m < tokenstack[i].tokenlen; ++m) { /* Extract token from cmdline */
                    token[m] = cmdline[k];
                    k++;
                 } /* end for */

                 tokenlen = tokenstack[i].tokenlen;
                 memset(errmsg, '\0', MSG_BUFF_LEN);

                 fp = syntaxtab[j].userexit; /* Get address of user exit */

                 rc = (*fp)(token, tokenlen, errmsg); /* Call user exit */

                 if (rc) { /* Save error rc */
                    *errtokenloc = tokenstack[i].tokenloc; /* Store offset of bad token */
                    *errtokenlen = tokenlen;               /* Store length of bad token */
                    ERR_FLAG = TRUE;
                 } /* end if */

              } /* end if */

              i++; /* Point to the next tokenstack entry */
              j = syntaxtab[j].nextrule; /* Point to the next syntax rule */

           } /* end else */

        } /* end if */
        else { /* tokentypes do not match */
           j++; /* Point to the next syntax rule */
        } /* end else */

     } /* end else - Process delimiter type tokens */

  } /* end while */

  if (ERR_FLAG) { /* User exit discovered a context error */
     return(INTERPRET_CONTEXT_ERROR);
  } /* end if */

  return(SUCCESS);

} /* end of interpret */
