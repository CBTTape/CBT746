#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parse.h>
#include <syntaxchk.h>
#include <interpret.h>

#define AS400                   'A'
#define UNIX                    'U'
#define WINDOWS                 'W'
#define PLATFORM    WINDOWS             /* set platform-specific flag */

#define TRUE 		1
#define FALSE 		0
#define ERROR		1
#define SUCCESS		0

/*****************************main globals ****************************************/

char NAME[10];
int  SCORE1;
int  SCORE2;

extern int exit010(char token[], int tokenlen, char *errmsg); /* Extract NAME     */
extern int exit020(char token[], int tokenlen, char *errmsg); /* Extract 1st score*/
extern int exit030(char token[], int tokenlen, char *errmsg); /* Extract 2nd score*/

/*****************************Documentation Start***********************************

  NAME: TestINTERPRET.c  - test program to drive parse.c, syntaxchk.c and interpret.c

******************************Documentation End************************************/

main()
{
  FILE *InputFileHandle = NULL;

  char ErrMsg[101];
  char inbuff[MAXSTRINGLEN+1];

#if PLATFORM==WINDOWS
  char INPUTFILE[65] = "TestINTERPRET.dat";
#endif
#if PLATFORM==UNIX
  char INPUTFILE[65] = "TestINTERPRET.dat";
#endif
#if PLATFORM==AS400
  char INPUTFILE[65] = "MQMTOOLS/TESTINPUT(TESTINTRPT)";
#endif

  int  rc;
  int  toklen;
  int  tokloc;
  int  ruleindex;

   char userdelim[33][2] = { /* User-defined delimiters for parse */
      PTPAD,YES,
      PTCOMMA,YES,
      PTEQUAL,YES,
      PTLPAREN,YES,		
      PTRPAREN,YES,		
      PTLT,YES,
      PTGT,YES,
      PTLBRACE,YES,
      PTRBRACE,YES,
      PTDASH,NO,		/* could be used in unix, Windows and AS/400 IFS files */
      PTUBAR,NO,		/* could be used in unix, Windows and AS/400 IFS files */
      PTAND,YES,
      PTPOUND,NO,		/* could be used in file names */
      PTAT,NO,			/* could be used in file names */
      PTPLUS,YES,
      PTSLASH,NO,		/* used in unix pathname */
      PTPERCENT,YES,	
      PTSTAR,YES,
      PTSCOLON,YES,
      PTCOLON,NO,		/* used to denote Windows drive */
      PTSQUOTE,YES,
      PTDQUOTE,YES,
      PTTILDE,NO,		/* could be used in DOS file names */
      PTBAR,YES,
      PTQUEST,YES,
      PTPERIOD,NO,		/* used in all file names, except AS/400 non-IFS */
      PTEXCLAM,YES,
      PTBSLASH,NO,		/* used in Windows pathname */
      PTDOLLAR,NO,		/* could be used in file names */
      PTRVQUOT,YES,
      PTLBRACKET,YES,
      PTRBRACKET,YES,
      PTNULL};

#include <TestSyntbl.h>

 /*****************/
 /* Start of main */
 /*****************/

  if (!(InputFileHandle = fopen(INPUTFILE, "r"))) { /* Open input file */
     printf(">>> File = %s could not be opened\n", INPUTFILE);
     return(ERROR);
  } /* end if */
  
  memset(inbuff, '\0', MAXSTRINGLEN);
  fgets(inbuff, MAXSTRINGLEN-1, InputFileHandle);

  while (!feof(InputFileHandle)) { /* Process all command statements */
     printf("\n+++ cmdlinelen = %d cmdline = %s\n", strlen(inbuff), inbuff);
 
     /* rc = parse(inbuff, DEFAULT_DELIMS); */
     rc = parse(inbuff, userdelim);

     if (rc) { /* Error detected by Parser */ 
        printf(">>> Error detected by Parser.  rc=%d\n", rc);
        return(ERROR);
     } /* end if */

     rc = syntaxchk(inbuff, syntaxtab, &tokloc, &toklen, &ruleindex);

     if (rc) { /* an error was found */
		if (rc == SYNTAX_ERROR)
           printf(">>> Syntax error in column %d token length = %d\n", tokloc + 1, toklen);

		if (rc == SYNTAX_PARM_IS_NULL)
           printf(">>> One,  or more call parameters, is NULL\n");

		if (rc == SYNTAX_NEXTRULE_ZERO)
           printf(">>> NEXTRULE value is zero in rule %d\n", ruleindex);

		if (rc == SYNTAX_NO_STARTRULE)
           printf(">>> STARTRULE is not the first entry in the syntax table\n");

     } /* end if */
     else { /* no syntax error */
        printf("+++ no syntax error\n");

		memset(NAME, '\0', sizeof(NAME));
		SCORE1 = 0;
		SCORE2 = 0;

        rc = interpret(inbuff, syntaxtab, &tokloc, &toklen, ErrMsg);

        if (rc) { /* an error was found */
		   if (rc == INTERPRET_CONTEXT_ERROR) {
              printf(">>> User exit found a problem\n");
			  printf(">>> User exit msg=%s\n", ErrMsg);
		   } /* end if */

           if (rc == INTERPRET_PARM_IS_NULL)
              printf(">>> One,  or more call parameters, is NULL\n");

		} /* end if */
		else {
		   printf("NAME=%s\n", NAME);
		   printf("SCORE1=%d\n", SCORE1);
		   printf("SCORE2=%d\n", SCORE2);
		} /* end else */
     } /* end else */

     memset(inbuff, '\0', sizeof(inbuff));
     fgets(inbuff, MAXSTRINGLEN-1, InputFileHandle);

  } /* end while */

  fclose(InputFileHandle);

  return(SUCCESS);

} /* end of main */


int exit010(char token[], int tokenlen, char *errmsg)
{
  memset(NAME, '\0', sizeof(NAME));
  strcat(NAME, token);

  printf("+++ exit010 called: token=%s, tokenlen=%d +++\n", token, tokenlen);

  return(SUCCESS);

} /* end of exit010 */

int exit020(char token[], int tokenlen, char *errmsg)
{
  printf("+++ exit020 called: token=%s, tokenlen=%d +++\n", token, tokenlen);

  SCORE1 = atoi(token);

  if (SCORE1 == 0) {
	 sprintf(errmsg,">>> 0 is invalid");
	 return(ERROR);
  } /* end if */

  return(SUCCESS);

} /* end of exit020 */

int exit030(char token[], int tokenlen, char *errmsg)
{
  printf("+++ exit030 called: token=%s, tokenlen=%d +++\n", token, tokenlen);

  SCORE2 = atoi(token);

  if (SCORE2 == 0) {
	 sprintf(errmsg,">>> 0 is invalid");
	 return(ERROR);
  } /* end if */

  return(SUCCESS);

} /* end of exit030 */
