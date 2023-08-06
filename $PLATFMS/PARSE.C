#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PARSE       1
#include <parse.h>

#define TRUE        1
#define FALSE       0
#define ERROR       1
#define SUCCESS     0

/*****************************Documentation Start***********************************

  NAME: parse.c  - Parse string into tokens


  DESCRIPTION:

  This program parses a string into tokens, based upon a set of delimiters that the
  caller specifies.  The set of delimiters can be customized by the user, or it can
  specify a default set. 

  The set of tokens that are created by this program can be used as input to 
  syntaxchk.c, which, given a set of syntax rules, can determine the syntactical
  correctness of the string.  In addition, once the string has been proven to be 
  syntactically correct, the set of tokens can then be used as input to interpret.c.
  This program effectively "interprets" the string, allowing the user to gain access
  to the tokens via user exits.


  ENTRY: From caller

  EXIT:  Return to caller


  INPUT:    char cmdline[]      - character array containing the string
            char delim[]        - character array containing delimiters

  OUTPUT:   struct parsework    - stack containing token descriptors

                         ------------------------------
                         | token | token  | location  |
                         | type  | length | in string |
                         ------------------------------


  RETURNS:  0 = Success
            1 = Parameter is NULL
            2 = String has invalid length


  *--PROPRIETARY STATEMENT--------------------------------------------------------*
  *   This program is the sole property of Richard Tsujimoto, Inc.                *
  *-------------------------------------------------------------------------------*


  HISTORY:

  Date        Who Description
  ----------- --- -----------------------------------------------------------------
  2005-Oct-17 RXT Following changes made:
                  1. Check for TOGGLE flag in a given delimiter: if TOGGLE found, 
                     turn on local toggle flag
                  2. If delimter is PTPAD (e.g. blank), the decision to tokenize it
                     is dependent on the local toggle flag
                  3. Store EOS token with all attributes set, e.g. token type, 
                     token size, offset of EOS in input string
  2005-Jul-08 RXT Check if any parameter is NULL; assign specific return code for
                  given error type
  2004-May-24 RXT Created

******************************Documentation End************************************/

/***************************** global vars ****************************************/

#include <dfltdelim.h>

struct parsework {  /* Stack of parsed tokens */
   int tokentype;   /* token type flag        */
   int tokenlen;    /* token length           */
   int tokenloc;    /* token offset in string */
} tokenstack[MAXSTRINGLEN+1];

/***************************** end of global vars *********************************/


int parse(char   cmdline[],                      /* String to be parsed */
          char   delim[][2])                     /* List of delimiters  */
{
  char *pcmd;       /* Pointer to input string          */

  int strttoken;    /* Location of the start of a token */
  int delimitermatch;   /* Token is/is not a delimiter  */
  int numbermatch;  /* Token is/is not a number         */
  int endofstring;  /* Token is/is not a NULL           */
  int toggle_flag;  /* Toggle flag                      */

  int i;
  int j;
  int k;
  int m;
  int n;

  /*******************/
  /* Start of parser */
  /*******************/

  toggle_flag = 0;  /* Initialize toggle flag */

  if ((cmdline == NULL) || (delim == NULL)) /* Any parameters set to NULL? */
     return(PARSE_PARM_IS_NULL);

  /* Validate input string lengths */
  if ((strlen(cmdline) < 1) || (strlen(cmdline) > MAXSTRINGLEN))
     return(PARSE_STRING_LEN_INVALID);

  /* Initialize token stack */
  for (i = 0; i < MAXSTRINGLEN; ++i) {
     tokenstack[i].tokentype  = 0;
     tokenstack[i].tokenlen   = 0;
     tokenstack[i].tokenloc   = 0;
  } /* end for */
  tokenstack[i+1].tokentype = PEOS; /* Store End-of-Stack flag */

  /* Parse string into tokens */
  pcmd = cmdline; /* start-of-input pointer        */ 
  strttoken = 0;  /* start-of-(sub)string index    */
  j = 0;          /* forward char index            */
  k = 0;          /* token stack entry index       */
  endofstring = FALSE;

  while (endofstring == FALSE) { /* Keep scanning till end-of-string */
     i = 0; /* initialize index into delimiter list */
     delimitermatch = TRUE;

     while ((cmdline[j] != delim[i][0]) && (delim[i][0] != PTNULL)) { /* Look for delimiter */
        i++; /* Bump index to next delimiter in list */
        if (delim[i][0] == PTNULL) { /* At the end of the delimiter list? */
           delimitermatch = FALSE;
        } /* end if */
     } /* end while */

     if ((cmdline[j] == '\0') || (cmdline[j] == '\n')) { /* we reached the end of the string */
        endofstring = TRUE;
     } /* end if */

     if ((delimitermatch) && (delim[i][1] == NO)) { /* check if the delimiter is to be ignored */
        delimitermatch = FALSE;
     } /* end if */

     if ((delimitermatch) || (endofstring)) { /* (Sub)string is: a delimiter, NULL, or NL */ 
        if (strttoken == j) { /* Is current (sub)string a potential delimiter? */
           if (endofstring == FALSE) { /* If current delimiter is NULL or NL, skip it */
              if (delim[i][1] == TOGGLE) { /* Toggle request? */
                 toggle_flag = toggle_flag ^ TOGGLE_ON;
              } /* end if */
              if (((delim[i][0] == PTPAD) && (toggle_flag == TOGGLE_ON)) || 
                  ((delim[i][0] != PTPAD))) { /* Should we tokenize the delimiter? */
                 tokenstack[k].tokentype = i;  /* Store delimiter type               */ 
                 tokenstack[k].tokenlen  = 1;  /* Store delimiter length = 1         */ 
                 tokenstack[k].tokenloc  = j;  /* Store delimiter location in string */ 
                 k++; /* Bump token stack index to next free stack entry */
              } /* end if */
              j++; /* Bunp index to next char in string */
              strttoken = j; /* Reset start-of-(sub)string index */
              pcmd++; /* Point to next char in input string */
           } /* end if */
        } /* end if */
        else { /* Store prior token first, then the following delimiter */
           numbermatch = TRUE; 
           m = strttoken; /* Point to first char of (sub)string */
           n = m + (j - strttoken); /* Compute length of (sub)string */

           while ((m < n) && (numbermatch)) { /* See if (sub)string is a number */
              if (!(isdigit(cmdline[m]))) { 
                 numbermatch = FALSE;  /* (sub)string is not a number */
              } /* end if */
              m++;
           } /* end while */

           if (numbermatch) {
              tokenstack[k].tokentype = PTNUM;   /* Mark (sub)string as a number  */
           } /* end if */
           else {
              tokenstack[k].tokentype = PTDATA;  /* Mark (sub)string as data      */ 
           } /* end else */

           tokenstack[k].tokenlen  = j - strttoken;  /* Store (sub)string length  */
           tokenstack[k].tokenloc  = strttoken;  /* Store (sub)string offset in input */ 

           k++;  /* Bump token stack index to next free token stack entry */

           if (endofstring == FALSE) { /* Tokenize the delimiter */
              if (delim[i][1] == TOGGLE) { /* Toggle request? */
                 toggle_flag = toggle_flag ^ TOGGLE_ON;
              } /* end if */
              if (((delim[i][0] == PTPAD) && (toggle_flag == TOGGLE_ON)) || 
                   (delim[i][0] != PTPAD)) { /* Should we tokenize the delimiter? */
                 tokenstack[k].tokentype = i;  /* Store delimiter type               */ 
                 tokenstack[k].tokenlen  = 1;  /* Store delimiter length = 1         */ 
                 tokenstack[k].tokenloc  = j;  /* Store delimiter location in string */ 
                 k++; /* Bump token stack index to next free token stack entry */
              } /* end if */
              j++; /* Bump index to next char after delimiter */
              strttoken = j; /* Reset start-of-(sub)string index */
              pcmd = pcmd + 2; /* Point to next char after delimiter */
           } /* end if */

        } /* end else - Store prior token first, then the following delimiter */

     } /* end if - String is: a delimiter, NULL, or NL */
     else { /* No delimiter match found yet */ 
        j++; /* Bump index to next next char in string */
     } /* end else */
     
  } /* end while - Keep scanning till end-of-string */

  tokenstack[k].tokentype = PEOS; /* Store End-Of-Stack flag             */
  tokenstack[k].tokenlen  = 1;    /* Store End-Of-Stack length = 1       */

  if (k != 0) { /* Is EOS not the only token on the stack? */
     m = tokenstack[k-1].tokenloc; /* Offset of prior token in input string */
     j = m + tokenstack[k-1].tokenlen;  /* Offset of EOS is after prior token */
  } /* end if */
     
  tokenstack[k].tokenloc  = j;    /* Store End-Of-Stack offset in string */

  return(SUCCESS);

} /* end parse */
