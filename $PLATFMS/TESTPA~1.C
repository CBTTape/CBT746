#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parse.h>
#include <syntaxchk.h>

#define AS400                   'A'
#define UNIX                    'U'
#define WINDOWS                 'W'
#define PLATFORM    WINDOWS             /* set platform-specific flag */

#define TRUE 		1
#define FALSE 		0
#define ERROR		1
#define SUCCESS		0


/*****************************Documentation Start***********************************

  NAME: TestPARSE.c  - test program to drive parse.c

******************************Documentation End************************************/

main()
{
  FILE *InputFileHandle = NULL;

  char inbuff[MAXSTRINGLEN+1];

#if PLATFORM==WINDOWS
  char INPUTFILE[65] = "TestPARSE.dat";
#endif
#if PLATFORM==UNIX
  char INPUTFILE[65] = "TestPARSE.dat";
#endif
#if PLATFORM==AS400
  char INPUTFILE[65] = "MQMTOOLS/TESTINPUT(TESTPARSE)";
#endif

  char QUITLOOP;

  char tok[20];
  int  rc;
  int  i;
  int  j;
  int  k;
  int  toktype;
  int  toklen;
  int  tokloc;

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
      PTDQUOTE,TOGGLE,  /* Make sure blanks are tokenized within double quotes */ 
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

     /* Print out parsing results */
     printf("+++ print out tokens\n");
     i = 0;
	 QUITLOOP = NO;

     while (QUITLOOP == NO) {
        if (tokenstack[i].tokentype == PEOS)
		   QUITLOOP = YES;
        toktype = tokenstack[i].tokentype;
        memset(tok, '\0', sizeof(tok));
        toklen = tokenstack[i].tokenlen;
        tokloc = tokenstack[i].tokenloc;
        j = tokloc;

        for (k = 0; k < toklen; ++k) {
           tok[k] = inbuff[j];
           j++;
        } /* end for */

        printf("token=%s  tokentype=%d  tokenloc=%d tokenleng=%d\n", tok, toktype, tokloc, toklen);
        i++;
     } /* end while */

     printf("+++ end print out tokens\n");
     printf("\n");

     memset(inbuff, '\0', sizeof(inbuff));
     fgets(inbuff, MAXSTRINGLEN-1, InputFileHandle);

  } /* end while */

  fclose(InputFileHandle);

  return(SUCCESS);

} /* end of main */
