#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parse.h>
#include <syntaxchk.h>

#define TRUE        1
#define FALSE       0
#define ERROR       1
#define SUCCESS     0

/*****************************Documentation Start***********************************

  NAME: syntaxchk.c  - Perform syntax check of tokens created by parse.c


  DESCRIPTION:
 
  This program performs a syntax check of a string, which is represented by a set  
  of tokens creatd by parse.c, by using the tokens as input to a set of "rules"
  that the user provides.  The "rules" contain very basic logic,  such as checking 
  the token type, comparing the token with user-specified character strings,
  branching to other "rules", and determining if the ultimate outcome of the 
  processing is a success or failure.  In other words, was the string found to be
  syntactically correct or not.

  The "rules" are read by syntaxchk.c and are interpreted, similar in the way a
  Basic program is read and interpreted (e.g. executed). And, as with any program,
  if the code, or "rules" in this case, contain faulty logic, the interpretation
  process could result in unpredictable results, such as a program fault, loop,
  and so forth.  In other words, the successful processing by syntaxchk.c is highly
  dependent upon the correctness of the user-provided "rules".


  ENTRY: From caller

  EXIT:  Return to caller


  INPUT:    char cmdline[]      - character array containing the string
            struct syntax_table *syntaxtab - set of user-provided "rules"
            int *errtokenloc    - int variable provided by caller
            int *errtokenlen    - int variable provided by caller

                The basic format of a "rule" is as follows:

                ----------------------------------
                | token | next | keyword  | user |
                | type  | rule | to match | exit |
                ----------------------------------

                For detailed information regarding syntax rules and how to
                to construct them, see:

                  Parsing, Syntax Checking and Interpreting - User's Guide


  OUTPUT:   int *errtokenloc        - index value of bad token within string
            int *errtokenlen        - length of bad token
            int *errindex           - index into syntax_table with zero "Next Rule"


  RETURNS:  0 = Success
            1 = Syntax error detected
            2 = Parameter is null
            3 = "Next Rule" index is zero
            4 = STARTRULE not found as first table entry


  *--PROPRIETARY STATEMENT--------------------------------------------------------*
  *   This program is the sole property of Richard Tsujimoto, Inc.                *
  *-------------------------------------------------------------------------------*


  HISTORY:

  Date        Who Description
  ----------- --- -----------------------------------------------------------------
  2005-Oct-17 RXT For syntax error, report offset of EOS as the char following the
                  substring/delimiter associated with the prior token entry; if EOS
                  is the one-and-only token on the stack, the offset is 0.
  2005-Jul-08 RXT Add support for token type STARTRULE
  2005-Jul-07 RXT Check if addr of command string is NULL
                  Check if addr of syntax table is NULL
                  Support token type GOTORULE
                  Check if GOTORULE "next rule" index is zero
                  Fix logic in CONTRULE test - do not point to next token
                  Ignore token type DUMMYRULE
                  Use SYSTEM_TOKENS to restructure logic testing
  2004-May-27 RXT Created

******************************Documentation End************************************/

/***************************** global vars ****************************************/

int contrule_loc = -1;  /* Loc of syntax rule to execute due to statement 
                           continuation */

/***************************** end of global vars *********************************/


int syntaxchk(char cmdline[],                  /* String to be checked     */
              struct syntax_table *syntaxtab,  /* Table of syntax rules    */
              int *errtokenloc,                /* Index value of bad token */
              int *errtokenlen,                /* Length of bad token      */
              int *errindex)                   /* Index into syntax_table  */
{
  char token[MAX_KEYWORD_LEN];  /* extracted token from cmdline      */

  int i;
  int j;
  int k;
  int m;

  /**********************/
  /* Start of syntaxchk */
  /**********************/

  if ((cmdline == NULL) || (syntaxtab == NULL) || /* Any parameters set to NULL? */
      (errtokenloc == NULL) || (errtokenlen == NULL) || (errindex == NULL))
     return(SYNTAX_PARM_IS_NULL);

  if (syntaxtab[0].tokentype != STARTRULE) /* Is STARTRULE the 1st table entry? */
     return(SYNTAX_NO_STARTRULE);

  i = 0; /* Point to start of tokenstack */
  j = 0; /* Point to start of syntax table */

  if (contrule_loc != -1) { /* We're in the midst of a continued statement */
     j = contrule_loc; /* Point to rule to resume syntax checking          */
     contrule_loc = -1;
  } /* end if */

  /* Check syntax of input string */
  while ((syntaxtab[j].tokentype  != LASTRULE) &&
         (syntaxtab[j].tokentype  != SYNTAXERR)) {

     if (syntaxtab[j].tokentype > SYSTEM_TOKENS ) { /* Process internal type token */

         switch (syntaxtab[j].tokentype) { /* Internal token type */

            case DUMMYRULE: /* Ignore and get next rule */
               j++;  /* Point to next syntax rule */
               break;

            case STARTRULE: /* Ignore and get next rule */
               j++;  /* Point to next syntax rule */
               break;

            case CONTRULE:  /* Process statement continuation */
               if (syntaxtab[j].nextrule == 0) { /* Bad syntax rule */
                  contrule_loc = -1;  /* Reset location of continuation rule */
                  *errindex = j;  /* Store location of bad rule */
                  return(SYNTAX_NEXTRULE_ZERO);
               } /* end if */

               contrule_loc = syntaxtab[j].nextrule;  /* Save index of continuation rule */
               j++;  /* Point to next syntax rule */
               break;

            case GOTORULE:  /* Process GOTO rule */
               if (syntaxtab[j].nextrule == 0) { /* Bad syntax rule */
                  contrule_loc = -1;  /* Reset location of continuation rule */
                  *errindex = j;  /* Store location of bad rule */
                  return(SYNTAX_NEXTRULE_ZERO);
               } /* end if */

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
                 i++; /* Point to the next tokenstack entry */

                 if (syntaxtab[j].nextrule == 0) { /* Bad syntax rule */
                    contrule_loc = -1;  /* Reset location of continuation rule */
                    *errindex = j;  /* Store location of bad rule */
                    return(SYNTAX_NEXTRULE_ZERO);
                 } /* end if */

                 j = syntaxtab[j].nextrule; /* Point to the next syntax rule */

              } /* end if */
              else { /* Keywords do not match */
                 j++; /* Point to the next syntax rule */
              } /* end else */
           
           } /* end if */
           else { /* No keyword match required */
              i++; /* Point to the next tokenstack entry */

              if (syntaxtab[j].nextrule == 0) { /* Bad syntax rule */
                 contrule_loc = -1;  /* Reset location of continuation rule */
                 *errindex = j;  /* Store location of bad rule */
                 return(SYNTAX_NEXTRULE_ZERO);
              } /* end if */

              j = syntaxtab[j].nextrule; /* Point to the next syntax rule */

           } /* end else */

        } /* end if */
        else { /* tokentypes do not match */
           j++; /* Point to the next syntax rule */
        } /* end else */
     } /* end else - Process delimiter type tokens */

  } /* end while */

  if (syntaxtab[j].tokentype == SYNTAXERR) { /* Syntax error found                  */
     *errtokenloc = tokenstack[i].tokenloc;  /* Store indexed location of bad token */
     *errtokenlen = tokenstack[i].tokenlen;  /* Store length of bad token           */
     contrule_loc = -1; /* Reset location of continuation rule */
     return(SYNTAX_ERROR);
  } /* end if */

  return(SUCCESS);

} /* end of syntaxchk */
