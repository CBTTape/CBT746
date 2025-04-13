#define PLATFORM    WINDOWS             /* set platform-specific flag */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>          
#include <errno.h>    

#include <parse.h>
#include <syntaxchk.h>
#include <interpret.h>

#include <syntblgen.h>

/*****************************Documentation Start***********************************

  NAME: syntblgen.c  - Generate syntax table from macro-form syntax rules


  DESCRIPTION:
 
  This program reads in syntax rules that closely model the MVS syntax rules based
  on macro statements and generates the C-language array elements that comprise the
  syntax rules.

  This tool makes it easier for the C programmer to create and maintain the syntax
  rules by allowing the rules to be specified as relocatable code vs. hard-coded
  array elements.


  ENTRY: From caller

  EXIT:  Return to caller


  INPUT: 

     Switches:

           -i fname         input file name
           -o fname         output file name
           -d ON            debug mode (optional)


  OUTPUT:  

     File: 
     
           standard output - messages
           
           output fname - that contains the array elements that comprise the 
                          syntax rules that are useable by C programs.


  RETURNS:  0 = Success
            1 = Failure


  *--PROPRIETARY STATEMENT--------------------------------------------------------*
  *   This program is the sole property of Richard Tsujimoto, Inc.                *
  *-------------------------------------------------------------------------------*


  HISTORY:

  Date        Who Description
  ----------- --- -----------------------------------------------------------------
  2005-Nov-16 RXT Created
  2006-Apr-27 RXT Add conditional code to refrain putting NL for AS/400

******************************Documentation End************************************/


int  EndOfFile      = FALSE;
int  NewRule        = FALSE; /* Used to denote the start of a new syntax rule statement */
int  INITIAL_flg    = FALSE; /* TYPE=INITIAL presence */
int  FINAL_flg      = FALSE; /* TYPE=FINAL presence */

int  ixG            = 0;    /* index into goto_table */
int  ixR            = -1;   /* index into rule_table */
int  ixT            = 0;    /* index into tokentype_table */

char InputFileName[FILENAME_LEN+1];  /* Input file name (including path name) */
char OutputFileName[FILENAME_LEN+1]; /* Output file name (including path name) */
char CtlCardBuff[MAXSTRINGLEN+1];    /* Input file buffer */
char temp_label[9]  = {""};
char comment[MAXCOMMENTLEN+1];
char OpenAttr[30];              /* AS/400 non-IFS open attributes      */

#if PLATFORM==AS400
char LF             = '\x25'; /* AS/400 and MVS LF */
#else
char LF             = '\x0a'; /* Unix and Windows LF */
#endif

FILE *fpInputFile   = NULL; /* Input file pointer */
FILE *fpOutputFile  = NULL; /* Output file pointer */

struct ruletab {        /* table of rules and sub-fields */
   char label[9];       /* label name of current rule or NULL */
   char tokentype[MAXTOKENTYPELEN+1]; /* macro-version of tokentype */
   char nextlabel[9];   /* NEXT=label name */
   int  nextlabel_idx;  /* array index value that correlates to NEXT=label name */
   char string[MAXSTRINGLEN+1]; /* string used for data comparison, or NULL */
   char exitname[MAXEXITNAMELEN+1]; /* user's exit name invoked during interpretation */
   char comment[MAXCOMMENTLEN+1];   /* user's comment */
} rule_table[MAX_RULES+1];

struct gototab {        /* table of labels and associated array index value */
   char label[9];           /* label name */
   int  ruleindex;      /* associated array index value */
} goto_table[MAX_RULES+1];

struct tokentypetab tokentype_table[] = {
    {"TOK_IS_DATA",         "TokIsData"},
    {"TOK_IS_NUM",          "TokIsNum"},
    {"TOK_IS_EOS",          "TokIsEOS"},
    {"TOK_IS_BLANK",        "TokIsBlank"},
    {"TOK_IS_COMMA",        "TokIsComma"},
    {"TOK_IS_EQUAL",        "TokIsEqual"},
    {"TOK_IS_LPAREN",       "TokIsLeftParen"},
    {"TOK_IS_RPAREN",       "TokIsRightParen"},
    {"TOK_IS_LT",           "TokIsLessThan"},
    {"TOK_IS_GT",           "TokIsGreaterThan"},
    {"TOK_IS_LBRACE",       "TokIsLeftBrace"},
    {"TOK_IS_RBRACE",       "TokIsRightBrace"},
    {"TOK_IS_DASH",         "TokIsDash"},
    {"TOK_IS_UBAR",         "TokIsUnderBar"},
    {"TOK_IS_AND",          "TokIsAnd"},
    {"TOK_IS_POUND",        "TokIsPound"},
    {"TOK_IS_AT",           "TokIsAt"},
    {"TOK_IS_PLUS",         "TokIsPlus"},
    {"TOK_IS_SLASH",        "TokIsSlash"},
    {"TOK_IS_PERCENT",      "TokIsPercent"},
    {"TOK_IS_STAR",         "TokIsAsterisk"},
    {"TOK_IS_SCOLON",       "TokIsSemiColon"},
    {"TOK_IS_COLON",        "TokIsColon"},
    {"TOK_IS_SQUOTE",       "TokIsSingleQuote"},
    {"TOK_IS_DQUOTE",       "TokIsDoubleQuote"},
    {"TOK_IS_TILDE",        "TokIsTilde"},
    {"TOK_IS_BAR",          "TokIsBar"},
    {"TOK_IS_QUEST",        "TokIsQuestion"},
    {"TOK_IS_PERIOD",       "TokIsPeriod"},
    {"TOK_IS_EXCLAM",       "TokIsExclamation"},
    {"TOK_IS_BSLASH",       "TokIsBackSlash"},
    {"TOK_IS_DOLLAR",       "TokIsDollar"},
    {"TOK_IS_RVQUOTE",      "TokIsReverseQuote"},
    {"TOK_IS_LBRACKET",     "TokIsLeftBracket"},
    {"TOK_IS_RBRACKET",     "TokIsRightBracket"},
    {NULL,NULL}, /* mark end of table */
};

/***************************** function prototypes ********************************/

int CheckSwitches(int,          /* No. of switches                  */
                  char **);     /* Char ptr to list of switch data  */

int DoPassOne();

int DoPassTwo();

int DoPassThree();

int Init();

int OpenFiles();

int PrtGoToTable();

int PrtRuleTable();

extern int exit000(char token[], int tokenlen, char *errmsg); /* label            */
extern int exit005(char token[], int tokenlen, char *errmsg); /* @RULE            */
extern int exit010(char token[], int tokenlen, char *errmsg); /* TYPE=INITIAL|FINA*/
extern int exit020(char token[], int tokenlen, char *errmsg); /* SYNTAXERR|LASTRUL*/
extern int exit030(char token[], int tokenlen, char *errmsg); /* CONTRULE|GOTORULE*/
extern int exit040(char token[], int tokenlen, char *errmsg); /* label (for above)*/
extern int exit050(char token[], int tokenlen, char *errmsg); /* tokentype        */
extern int exit060(char token[], int tokenlen, char *errmsg); /* NEXT= (for above)*/
extern int exit070(char token[], int tokenlen, char *errmsg); /* STRING,EXIT,COMME*/
extern int exit080(char token[], int tokenlen, char *errmsg); /* STRING=s...s     */
extern int exit090(char token[], int tokenlen, char *errmsg); /* EXIT=x...x       */
extern int exit095(char token[], int tokenlen, char *errmsg); /* COMMENT="        */
extern int exit100(char token[], int tokenlen, char *errmsg); /* c...c (for above)*/
extern int exit105(char token[], int tokenlen, char *errmsg); /* " (for above)    */

/***************************** end of function prototypes *************************/


int main(int argc, char **argv)
{

   /******************/
   /* Start of main  */
   /******************/


   Init(); /* Initialize vars */

   printf("\n+++ SYNTBLGEN STARTING\n\n");

   if (CheckSwitches(argc, argv)) { /* Validate switches */
      return(ERROR);
   } /* end if */

   if (OpenFiles()) { /* Open input and output files */
      return(ERROR);
   } /* end if */

   if (DoPassOne()) { /* Read input and populate arrays */
      return(ERROR);
   } /* end if */

   if (DoPassTwo()) { /* Check for valid branch labels */
      return(ERROR);
   } /* end if */

   if (DoPassThree()) { /* Generate array elements */
      return(ERROR);
   } /* end if */

   printf("\n+++ SYNTBLGEN ENDING\n");

   return(SUCCESS); 

} /* end main */


/*****************************Documentation Start***********************************

  NAME: Init - Initialize vars, workareas, etc.

******************************Documentation End************************************/
 
int Init()
{
   int i;

   memset(InputFileName,  '\0', sizeof(InputFileName));
   memset(OutputFileName, '\0', sizeof(OutputFileName));

   for (i = 0; i < MAX_RULES; i++) { /* initialize rule_table */
      memset(rule_table[i].label,     '\0', sizeof(rule_table[0].label));
      memset(rule_table[i].tokentype, '\0', sizeof(rule_table[0].tokentype));
      memset(rule_table[i].nextlabel, '\0', sizeof(rule_table[0].nextlabel));
      rule_table[i].nextlabel_idx = -1;
      memset(rule_table[i].string,    '\0', sizeof(rule_table[0].string));
      memset(rule_table[i].exitname,  '\0', sizeof(rule_table[0].exitname));
      memset(rule_table[i].comment,   '\0', sizeof(rule_table[0].comment));
   } /* end of for */

   for (i = 0; i < MAX_RULES; i++) { /* initialize goto_table */
      memset(goto_table[i].label,     '\0', strlen(goto_table[i].label));
      goto_table[i].ruleindex = -1;
   } /* end of for */

   return(SUCCESS);

} /* end Init */


/*****************************Documentation Start***********************************

  NAME: CheckSwitches - Validate program switches

******************************Documentation End************************************/
 
int CheckSwitches(int argc, char **argv)
{
   int i;
   int k;
 
   char switchflg[3];
   char switchval[100];
   char switchline[100]; /* line containing both switch and data */

   if (argc == 1) { /* no switches entered */
      printf(">>> NO SWITCHES SPECIFIED\n");
      return(ERROR);
   } /* end if */

   i = (MaxSwitches * 2) + 1; /* max switches and data */

   if (!(argc - ((argc/2) * 2)) || (argc > i)) { /* check for valid no. of switches */
      printf(">>> INVALID NUMBER OF SWITCH FLAGS AND DATA\n");
      return(ERROR);
   } /* end if */

   i = 1; /* skip first switch (program name) */
   k = 0;

   while (i < argc) { /* process switches */
      if ((strlen(argv[i]) != 2) || (argv[i][0] != '-')) { /* validate switch flag */
         printf(">>> INVALID SWITCH FLAG\n");
         return(ERROR);
      } /* end if */

      /* reconstruct each switch and its data as a single line and save it */
      memset(switchflg, '\0', sizeof(switchflg));
      strncpy(switchflg, argv[i], strlen(argv[i]));
      memset(switchline, '\0', sizeof(switchline));
      strcat(switchline, switchflg);
      strcat(switchline, " ");

      if (strlen(argv[i+1]) > MAXSTRINGLEN) {
         printf(">>> SWITCH VALUE EXCEEDS %d CHARACERS\n", MAXSTRINGLEN);
         return(ERROR);
      } /* end if */

      strcat(switchline, argv[i+1]);

      printf("%s\n", switchline); /* echo switch and data */

      memset(switchval, '\0', sizeof(switchval));
      strncpy(switchval, argv[i+1], strlen(argv[i+1]));

      switch (switchflg[1]) { /* flag the presence of each switch flag coded */

         case 'd': /* -d ON */
            if (DEBUG_FLAG == ON) {
               printf(">>> DUPLICATE SWITCH SPECIFIED: -d\n");
               return(ERROR);
            } /* end if */

            if (strcmp(switchval, "ON")) {
               printf(">>> INVALID SWITCH VALUE SPECIFIED: -d\n");
               return(ERROR);
            } /* end if */

            DEBUG_FLAG = ON;
            break;

         case 'i': /* -i InputFileName */
            if ((strlen(InputFileName)) != 0) {
               printf(">>> DUPLICATE SWITCH SPECIFIED: -i\n");
               return(ERROR);
            } /* end if */

            strncpy(InputFileName, switchval, strlen(switchval));
            break;

         case 'o': /* -o OutputFileName */
            if ((strlen(OutputFileName)) != 0) {
               printf(">>> DUPLICATE SWITCH SPECIFIED: -o\n");
               return(ERROR);
            } /* end if */

            strncpy(OutputFileName, switchval, strlen(switchval));
            break;

         default:
            printf(">>> UNDEFINED SWITCH FLAG\n");
            return(ERROR);
            break;

      } /* end switch */

      i = i + 2; /* bump index to next switch flag */
      k++;

   } /* end while - process switches */

   if (((strlen(InputFileName) == 0) || (strlen(OutputFileName) == 0))) {
      printf(">>> MANDATORY SWITCH -i or -o MISSING\n");
      return(ERROR);
   } /* end if */

   return(SUCCESS); 

} /* end CheckSwitches */


/*****************************Documentation Start***********************************

  NAME: OpenFiles - Open input and output file

******************************Documentation End************************************/
 
int OpenFiles()
{

   if ((fpInputFile = fopen(InputFileName, "r")) == NULL) { /* Open input file */
      printf(">>> OPEN FAILED FOR INPUT FILE=%s: %s\n", InputFileName, strerror(errno));
      return(ERROR);
   } /* end if */

#if PLATFORM==AS400
   strcat(OpenAttr, "wb,type=record");

   if ((fpOutputFile = fopen(OutputFileName, OpenAttr)) == NULL) { /* Open target file */
      printf(">>> OPEN FAILED FOR OUTPUT FILE=%s: %s\n", OutputFileName, strerror(errno));
      return(ERROR);
   } /* end if */
#else
   if ((fpOutputFile = fopen(OutputFileName, "w")) == NULL) { /* Open target file */
      printf(">>> OPEN FAILED FOR OUTPUT FILE=%s: %s\n", OutputFileName, strerror(errno));
      return(ERROR);
   } /* end if */
#endif

   return(SUCCESS);

} /* end of OpenFiles */


/*****************************Documentation Start***********************************

  NAME: DoPassOne - Validate "macro" syntax statements

******************************Documentation End************************************/
 
int DoPassOne()
{
   char userdelim[33][2] = { /* User-defined delimiters for parse */
      PTPAD,YES,
      PTCOMMA,YES,
      PTEQUAL,YES,
      PTLPAREN,NO,
      PTRPAREN,NO,
      PTLT,NO,
      PTGT,NO,
      PTLBRACE,NO,
      PTRBRACE,NO,
      PTDASH,NO,
      PTUBAR,NO,
      PTAND,NO,
      PTPOUND,NO,
      PTAT,NO,   
      PTPLUS,YES, 
      PTSLASH,NO,
      PTPERCENT,NO, 
      PTSTAR,NO,
      PTSCOLON,NO,
      PTCOLON,NO,
      PTSQUOTE,YES,
      PTDQUOTE,TOGGLE,  /* make sure blanks get tokenized */
      PTTILDE,NO,
      PTBAR,NO,
      PTQUEST,NO,
      PTPERIOD,NO,
      PTEXCLAM,NO,
      PTBSLASH,NO,
      PTDOLLAR,NO,
      PTRVQUOT,NO,
      PTLBRACKET,NO,
      PTRBRACKET,NO,
      PTNULL};

   struct syntax_table syntaxtab[MAX_RULES] = { /* Syntax rules */
      /* rule-0000 */ {STARTRULE},                 
      /* rule-0001 */ {TokIsData,        GoTo6, "@RULE", &exit005},
      /* rule-0002 */ {TokIsData,        GoTo4, NULL, &exit000},
      /* rule-0003 */ {SYNTAXERR},                 
      /* rule-0004 */ {TokIsData,        GoTo6, "@RULE", &exit005},
      /* rule-0005 */ {SYNTAXERR},                 
      /* rule-0006 */ {TokIsData,        GoTo49, "TYPE"},
      /* rule-0007 */ {TokIsData,        GoTo62, "SYNTAXERR", &exit020},
      /* rule-0008 */ {TokIsData,        GoTo62, "LASTRULE", &exit020},
      /* rule-0009 */ {TokIsData,        GoTo54, "CONTRULE", &exit030},
      /* rule-0010 */ {TokIsData,        GoTo54, "GOTORULE", &exit030},
      /* rule-0011 */ {TokIsData,        GoTo65, "TOK_IS_DATA", &exit050},
      /* rule-0012 */ {TokIsData,        GoTo65, "TOK_IS_NUM", &exit050},
      /* rule-0013 */ {TokIsData,        GoTo65, "TOK_IS_EOS", &exit050},
      /* rule-0014 */ {TokIsData,        GoTo65, "TOK_IS_BLANK", &exit050},
      /* rule-0015 */ {TokIsData,        GoTo65, "TOK_IS_COMMA", &exit050},
      /* rule-0016 */ {TokIsData,        GoTo65, "TOK_IS_EQUAL", &exit050},
      /* rule-0017 */ {TokIsData,        GoTo65, "TOK_IS_LPAREN", &exit050},
      /* rule-0018 */ {TokIsData,        GoTo65, "TOK_IS_RPAREN", &exit050},
      /* rule-0019 */ {TokIsData,        GoTo65, "TOK_IS_LT", &exit050},
      /* rule-0020 */ {TokIsData,        GoTo65, "TOK_IS_GT", &exit050},
      /* rule-0021 */ {TokIsData,        GoTo65, "TOK_IS_LBRACE", &exit050},
      /* rule-0022 */ {TokIsData,        GoTo65, "TOK_IS_RBRACE", &exit050},
      /* rule-0023 */ {TokIsData,        GoTo65, "TOK_IS_DASH", &exit050},
      /* rule-0024 */ {TokIsData,        GoTo65, "TOK_IS_UBAR", &exit050},
      /* rule-0025 */ {TokIsData,        GoTo65, "TOK_IS_AND", &exit050},
      /* rule-0026 */ {TokIsData,        GoTo65, "TOK_IS_POUND", &exit050},
      /* rule-0027 */ {TokIsData,        GoTo65, "TOK_IS_AT", &exit050},
      /* rule-0028 */ {TokIsData,        GoTo65, "TOK_IS_PLUS", &exit050},
      /* rule-0029 */ {TokIsData,        GoTo65, "TOK_IS_SLASH", &exit050},
      /* rule-0030 */ {TokIsData,        GoTo65, "TOK_IS_PERCENT", &exit050},
      /* rule-0031 */ {TokIsData,        GoTo65, "TOK_IS_STAR", &exit050},
      /* rule-0032 */ {TokIsData,        GoTo65, "TOK_IS_SCOLON", &exit050},
      /* rule-0033 */ {TokIsData,        GoTo65, "TOK_IS_COLON", &exit050},
      /* rule-0034 */ {TokIsData,        GoTo65, "TOK_IS_SQUOTE", &exit050},
      /* rule-0035 */ {TokIsData,        GoTo65, "TOK_IS_DQUOTE", &exit050},
      /* rule-0036 */ {TokIsData,        GoTo65, "TOK_IS_TILDE", &exit050},
      /* rule-0037 */ {TokIsData,        GoTo65, "TOK_IS_BAR", &exit050},
      /* rule-0038 */ {TokIsData,        GoTo65, "TOK_IS_QUEST", &exit050},
      /* rule-0039 */ {TokIsData,        GoTo65, "TOK_IS_PERIOD", &exit050},
      /* rule-0040 */ {TokIsData,        GoTo65, "TOK_IS_EXCLAM", &exit050},
      /* rule-0041 */ {TokIsData,        GoTo65, "TOK_IS_BSLASH", &exit050},
      /* rule-0042 */ {TokIsData,        GoTo65, "TOK_IS_DOLLAR", &exit050},
      /* rule-0043 */ {TokIsData,        GoTo65, "TOK_IS_RVQUOTE", &exit050},
      /* rule-0044 */ {TokIsData,        GoTo65, "TOK_IS_LBRACKET", &exit050},
      /* rule-0045 */ {TokIsData,        GoTo65, "TOK_IS_RBRACKET", &exit050},
      /* rule-0046 */ {TokIsEOS,         GoTo48}, /* Flush rest of input line */
      /* rule-0047 */ {SYNTAXERR},                 
      /* rule-0048 */ {LASTRULE},                  
      /* rule-0049 */ {TokIsEqual,       GoTo51}, /* Start: TYPE=INITIAL|FINAL */
      /* rule-0050 */ {SYNTAXERR},                 
      /* rule-0051 */ {TokIsData,        GoTo46, "INITIAL", &exit010},
      /* rule-0052 */ {TokIsData,        GoTo120, "FINAL", &exit010},
      /* rule-0053 */ {SYNTAXERR},                /* End:   TYPE=INITIAL|FINAL */
      /* rule-0054 */ {TokIsComma,       GoTo56}, /* Start: CONTRULE|GOTORULE */
      /* rule-0055 */ {SYNTAXERR},                 
      /* rule-0056 */ {TokIsData,        GoTo58, "NEXT"},
      /* rule-0057 */ {SYNTAXERR},                 
      /* rule-0058 */ {TokIsEqual,       GoTo60},  
      /* rule-0059 */ {SYNTAXERR},                 
      /* rule-0060 */ {TokIsData,        GoTo62, NULL, &exit040},
      /* rule-0061 */ {SYNTAXERR},                /* End:   CONTRULE|GOTORULE */
      /* rule-0062 */ {TokIsEOS,         GoTo48}, /* Check if COMMENT=... coded */
      /* rule-0063 */ {TokIsComma,       GoTo79},  
      /* rule-0064 */ {SYNTAXERR},                 
      /* rule-0065 */ {TokIsComma,       GoTo67}, /* Start: NEXT=label */
      /* rule-0066 */ {SYNTAXERR},                 
      /* rule-0067 */ {TokIsData,        GoTo69, "NEXT"},
      /* rule-0068 */ {SYNTAXERR},                 
      /* rule-0069 */ {TokIsEqual,       GoTo71},  
      /* rule-0070 */ {SYNTAXERR},                 
      /* rule-0071 */ {TokIsData,        GoTo73, NULL, &exit060},
      /* rule-0072 */ {SYNTAXERR},                /* End:   NEXT=label */
      /* rule-0073 */ {TokIsComma,       GoTo76}, /* Start: Process STRING, EXIT, COMMENT */
      /* rule-0074 */ {TokIsEOS,         GoTo48},  
      /* rule-0075 */ {SYNTAXERR},                 
      /* rule-0076 */ {TokIsPlus,        GoTo81}, /* Continuation delimiter */
      /* rule-0077 */ {TokIsData,        GoTo83, "STRING", &exit070}, /* STRING=s...s   */
      /* rule-0078 */ {TokIsData,        GoTo94, "EXIT", &exit070}, /* EXIT=x...x     */
      /* rule-0079 */ {TokIsData,        GoTo105, "COMMENT", &exit070}, /* COMMENT=...  */
      /* rule-0080 */ {SYNTAXERR},                /* End:   Process STRING, EXIT, COMMENT */
      /* rule-0081 */ {CONTRULE,         GoTo77}, /* Store continuation point */
      /* rule-0082 */ {GOTORULE,         GoTo46},  
      /* rule-0083 */ {TokIsEqual,       GoTo85}, /* Start: Process STRING=s...s */
      /* rule-0084 */ {SYNTAXERR},                 
      /* rule-0085 */ {TokIsData,        GoTo87, NULL, &exit080},
      /* rule-0086 */ {SYNTAXERR},                 
      /* rule-0087 */ {TokIsComma,       GoTo90},  
      /* rule-0088 */ {TokIsEOS,         GoTo48},  
      /* rule-0089 */ {SYNTAXERR},                 
      /* rule-0090 */ {TokIsPlus,        GoTo92},  
      /* rule-0091 */ {GOTORULE,         GoTo78},  
      /* rule-0092 */ {CONTRULE,         GoTo78}, /* Store continuation point */
      /* rule-0093 */ {GOTORULE,         GoTo46}, /* End:   Process STRING=s...s */
      /* rule-0094 */ {TokIsEqual,       GoTo96}, /* Start: Process EXIT=x...x */
      /* rule-0095 */ {SYNTAXERR},                 
      /* rule-0096 */ {TokIsData,        GoTo98, NULL, &exit090},
      /* rule-0097 */ {SYNTAXERR},                 
      /* rule-0098 */ {TokIsComma,       GoTo101}, 
      /* rule-0099 */ {TokIsEOS,         GoTo48},  
      /* rule-0100 */ {SYNTAXERR},                 
      /* rule-0101 */ {TokIsPlus,        GoTo103}, 
      /* rule-0102 */ {GOTORULE,         GoTo79},  
      /* rule-0103 */ {CONTRULE,         GoTo79},  
      /* rule-0104 */ {GOTORULE,         GoTo46}, /* End:  Process EXIT=x...x */
      /* rule-0105 */ {TokIsEqual,       GoTo107}, /* Start: Process COMMENT=... */
      /* rule-0106 */ {SYNTAXERR},                 
      /* rule-0107 */ {TokIsDoubleQuote, GoTo109, NULL, &exit095},
      /* rule-0108 */ {SYNTAXERR},                 
      /* rule-0109 */ {TokIsBlank,       GoTo109, NULL, &exit100},
      /* rule-0110 */ {TokIsData,        GoTo109, NULL, &exit100},
      /* rule-0111 */ {TokIsNum,         GoTo109, NULL, &exit100},
      /* rule-0112 */ {TokIsComma,       GoTo109, NULL, &exit100},
      /* rule-0113 */ {TokIsEqual,       GoTo109, NULL, &exit100},
      /* rule-0114 */ {TokIsPlus,        GoTo109, NULL, &exit100},
      /* rule-0115 */ {TokIsSingleQuote, GoTo109, NULL, &exit100},
      /* rule-0116 */ {TokIsDoubleQuote, GoTo118, NULL, &exit105},
      /* rule-0117 */ {SYNTAXERR},                 
      /* rule-0118 */ {TokIsEOS,         GoTo48},  
      /* rule-0119 */ {SYNTAXERR},                /* End:  Process COMMENT=... */
      /* rule-0120 */ {CONTRULE,         GoTo122}, /* Anything after TYPE=FINAL is an error */
      /* rule-0121 */ {GOTORULE,         GoTo46},  
      /* rule-0122 */ {SYNTAXERR},                 
   }; /* end of syntax table */


   int iFlag;
   int loopFlag;
   int iSkipInput;
   int toklen;
   int tokloc;
   int ruleindex;
   int i;
   int rc;

   char ErrMsg[101];


   iFlag     = SUCCESS;
   loopFlag  = SUCCESS;
   EndOfFile = FALSE;

   printf("\n+++ USER INPUT FOLLOWS\n\n");

   while ((EndOfFile == FALSE) && (loopFlag == SUCCESS)) { /* Process all syntax rules */

      memset(CtlCardBuff, '\0', MAXSTRINGLEN+1);

      fgets(CtlCardBuff, MAXSTRINGLEN-1, fpInputFile);

      if (CtlCardBuff[0] != '\0') { /* Any data to process? */

         printf("%s", CtlCardBuff); /* Echo the syntax rule */

#if PLATFORM==AS400
         if (CtlCardBuff[strlen(CtlCardBuff) - 1] != '\n')
#else
         if (CtlCardBuff[strlen(CtlCardBuff) - 1] != LF)
#endif
            fprintf(stdout, "\n");

         iSkipInput = FALSE;

         if ((CtlCardBuff[0] == '*') || (CtlCardBuff[0] == '\n')) { /* Skip comment cards */
            iSkipInput = TRUE;
         } /* end if */

         if (iSkipInput == FALSE) { /* Check for blank lines */
            iSkipInput = TRUE; /* Assume input has all blanks */
            i = 0;

            while((CtlCardBuff[i]) && (iSkipInput)) { /* Check until NULL or non-blank found */
            
               if ((CtlCardBuff[i] != BLNK && (CtlCardBuff[i]) != '\n')) {
                  iSkipInput = FALSE; /* A mon-blank char found */
               } /* end if */

               i++;
            } /* end while */
         } /* end if */

         if (iSkipInput == FALSE) { /* Process non-comment input */

            rc = parse(CtlCardBuff, userdelim); /* Parse the syntax rule */

            if (rc) { /* Process parse error */
               loopFlag = ERROR;
               iFlag = ERROR;
               printf(">>> PARSE ERROR, RC=%d\n", rc);
            } /* end if */
            else { /* Check the syntax of the syntax rule */

               rc = syntaxchk(CtlCardBuff, syntaxtab, &tokloc, &toklen, &ruleindex);

               if (rc) { /* Process syntax error */
                  if (rc == SYNTAX_ERROR)
                     printf(">>> INVALID CONTROL STATEMENT, COLUMN %d\n", tokloc+1);

                  if (rc == SYNTAX_PARM_IS_NULL)
                     printf(">>> SYNTAXCHK CALL PARM LIST HAS NULL(S)\n");

                  if (rc == SYNTAX_NEXTRULE_ZERO)
                     printf(">>> NEXTRULE VALUE IS ZERO IN RULE=%d\n", ruleindex);

                  loopFlag = ERROR;
                  iFlag = ERROR;
               } /* end if */
               else { /* Interpret the syntax rule */

                  rc = interpret(CtlCardBuff, syntaxtab, &tokloc, &toklen, ErrMsg);
                     
                  if (rc) { /* Process interpret error */
                     if (rc == INTERPRET_CONTEXT_ERROR)
                        printf("%s", ErrMsg);

                     if (rc == INTERPRET_PARM_IS_NULL)
                        printf(">>> INTERPRET CALL PARM LIST HAS NULL(S)\n");

                     loopFlag = ERROR;
                     iFlag = ERROR;
                  } /* end if */

               } /* end else */

            } /* end else - Check the syntax of the syntax rule */
         } /* end if - Process non-comment input */

      } /* end if - Any data to process? */

      if (feof(fpInputFile)) /* Check if we're at EOF */
         EndOfFile = TRUE;

   } /* end while - Process all syntax rules */

   fclose(fpInputFile);

   if (iFlag)
      return(ERROR);

   if ((INITIAL_flg == FALSE) || (FINAL_flg == FALSE)) {
      printf(">>> TYPE=INITIAL AND/OR TYPE=FINAL IS MISSING\n");
      iFlag = ERROR;
   } /* end if */

   if (DEBUG_FLAG) {
      PrtRuleTable();
      PrtGoToTable();
   } /* end if */

   return(SUCCESS);

} /* end DoPassOne */


/*****************************Documentation Start***********************************

  NAME: DoPassTwo - Check for valid branch labels

******************************Documentation End************************************/
 
int DoPassTwo()
{
   int i;
   int j;
   int iFound;


   for (i = 0; i < ixR; i++) {
       if (strlen(rule_table[i].nextlabel) != 0) { /* Find array index for NEXT=label */
          iFound = FALSE;
          j = 0;

          while ((j < MAX_RULES) && (iFound == FALSE) && 
                 (strlen(goto_table[j].label) != 0)) {
             if (!strcmp(rule_table[i].nextlabel, goto_table[j].label)) {
                rule_table[i].nextlabel_idx = goto_table[j].ruleindex;
                iFound = TRUE;
             } /* end if */

             j++;

          } /* end while */

          if (iFound == FALSE) {
             printf(">>> LABEL IN NEXT=%-8s IS NOT DEFINED\n", rule_table[i].nextlabel);
             return(ERROR);
          } /* end if */
       } /* end if */

   } /* end for */

   if (DEBUG_FLAG) {
      PrtRuleTable();
   } /* end if */

   return(SUCCESS);

} /* end DoPassTwo */


/*****************************Documentation Start***********************************

  NAME: DoPassThree - Generate array elements

******************************Documentation End************************************/
 
int DoPassThree()
{
   int i;
   int isz;
   int string_len;
   int exit_len;
   int comment_len;
   int freebytes;
   int initfree;


   char rulehdr[]  = "   struct syntax_table syntaxtab[MAX_RULES] = { /* Syntax rules */\n";
   char hdr1a[]    = "      /* rule-";
   char hdr1b[]    = " */ {";
   char hdr2[]     = "GoTo";
   char lastrule[] = "   }; /* end of syntax table */\n";

   char label[5];
   char tokentype[18];
   char gotolabel[5];

   char *pch;
   
   union {
      char rulebuff[213];     /* AS/400 buffer for C-version of syntax rule */
      struct {
         char fill_12[12];    /* AS/400 12-byte prefix for data members */
         char buff[201];
      } Rule;
   } AS400Rule;

   union {
      char rulebuff[201];     /* buffer for C-version of syntax rule */
      struct {
         char label_hdr1a[14];
         char label_num[4];
         char label_hdr1b[5];
         char tokentype[18];  /* includes room for ', ' */
         char label_hdr2[4];
         char gotolabel[6];   /* max 3-digits, and '}, ' */
         char filler[149];    
      } Field;
   } Rule;



   initfree = (sizeof(Rule.rulebuff) - 1)    -
               sizeof(Rule.Field.label_hdr1a) -
               sizeof(Rule.Field.label_num)   -
               sizeof(Rule.Field.label_hdr1b) -
               sizeof(Rule.Field.tokentype)   -
               (sizeof(Rule.Field.label_hdr2) + 5); /* Assume 3 digits and '},' */

#if PLATFORM==AS400
   memset(AS400Rule.rulebuff, '\0', sizeof(AS400Rule.rulebuff));
   memset(AS400Rule.Rule.fill_12, BLNK, sizeof(AS400Rule.Rule.fill_12));
   memcpy(AS400Rule.Rule.buff, rulehdr, strlen(rulehdr));

   fwrite(AS400Rule.rulebuff, strlen(AS400Rule.rulebuff), 1, fpOutputFile);
#else
   fwrite(rulehdr, strlen(rulehdr), 1, fpOutputFile);
#endif


   for (i = 0; i < ixR; i++) { /* Generate C-version of syntax rules */
      freebytes = initfree;
      memset(Rule.rulebuff, '\0', sizeof(Rule.rulebuff));

      /* Format label header and value */
      memset(label, '\0', sizeof(label)); 
      sprintf(label, "%04i", i);
      memcpy(Rule.Field.label_hdr1a, hdr1a, sizeof(hdr1a) - 1);
      memcpy(Rule.Field.label_num,   label, sizeof(label) - 1);
      memcpy(Rule.Field.label_hdr1b, hdr1b, sizeof(hdr1b) - 1);

      /* Format tokentype */
      memset(tokentype, '\0', sizeof(tokentype));
      memset(tokentype, BLNK, sizeof(tokentype) - 1);
      isz = strlen(rule_table[i].tokentype);
      memcpy(tokentype, rule_table[i].tokentype, isz);
      memset(Rule.Field.tokentype, BLNK, sizeof(Rule.Field.tokentype));
      memcpy(Rule.Field.tokentype, tokentype, isz);
      pch = &Rule.Field.tokentype[isz]; /* Point to byte following tokentype string */

      if (rule_table[i].nextlabel_idx == -1) { /* No NEXT=index */
         memcpy(pch, "},", 2);
         memset(Rule.Field.label_hdr2, BLNK, sizeof(Rule.Field.label_hdr2));
         memset(Rule.Field.gotolabel, BLNK, sizeof(Rule.Field.gotolabel));
         pch = &Rule.Field.gotolabel[sizeof(Rule.Field.gotolabel)] - 2; /* Point to next byte */
      } /* end if */
      else { /* Process NEXT=index, and other keywords as needed */
         memcpy(pch, ", ", 2);

         /* Format GoToxxx */
         memcpy(Rule.Field.label_hdr2, hdr2, strlen(hdr2));
         memset(gotolabel, '\0', sizeof(gotolabel));
         
         if (rule_table[i].nextlabel_idx < 10)
            sprintf(gotolabel, "%-i", rule_table[i].nextlabel_idx);
         if ((rule_table[i].nextlabel_idx > 10) && (rule_table[i].nextlabel_idx < 100))
            sprintf(gotolabel, "%-2i", rule_table[i].nextlabel_idx);
         if ((rule_table[i].nextlabel_idx > 99) && (rule_table[i].nextlabel_idx < 1000))
            sprintf(gotolabel, "%-3i", rule_table[i].nextlabel_idx);

         memset(Rule.Field.gotolabel, BLNK, sizeof(Rule.Field.gotolabel));
         isz = strlen(gotolabel);
         memcpy(Rule.Field.gotolabel, gotolabel, isz);
         pch = &Rule.Field.gotolabel[isz]; /* Point to byte after GoToxxx */

         string_len = strlen(rule_table[i].string);
         exit_len   = strlen(rule_table[i].exitname);

         if ((string_len == 0) && (exit_len == 0)) { /* No STRING and EXIT keyword values */
            memcpy(pch, "},", 2);
            pch = pch + 2;
         } /* end if */
         else { /* Process STRING or EXIT */
            memcpy(pch, ", ", 2);
            pch = &Rule.Field.gotolabel[isz + 2]; /* Point to start of STRING value */
            freebytes = freebytes - (isz + 6);

            if (string_len == 0) { /* No STRING keyword */
               isz = strlen("NULL");
               memcpy(pch, "NULL", isz);
               pch = pch + (isz - 1); /* Point to byte after NULL */
               freebytes = freebytes - (isz - 1);
            } /* end if */
            else { /* Process STRING value */
               memcpy(pch, "\"", 1);
               pch++; /* Point to byte following double quote */
               memcpy(pch, rule_table[i].string, string_len);
               pch = pch + string_len; /* Point to byte following string */
               memcpy(pch, "\"", 1);
               freebytes = freebytes - (string_len + 2);
            } /* end else */

            pch++; /* Point to byte following unquoted/double quoted string */

            if (exit_len == 0) { /* No EXIT coded */
               memcpy(pch, "},", 2);
               pch = pch + 2;
               freebytes = freebytes - (isz + 2);
            } /* end if */
            else { /* Process EXIT value */
               memcpy(pch, ", &", 3);
               pch = pch + 3;
               memcpy(pch, rule_table[i].exitname, exit_len);
               pch = pch + exit_len;
               memcpy(pch, "},", 2);
               pch = pch + 2;
               freebytes = freebytes - (exit_len + 5);
            } /* end else */

         } /* end else - process STRING or EXIT */
      } /* end else - Process NEXT=index, and other keywords as needed */

      comment_len = strlen(rule_table[i].comment);

      if (comment_len != 0) { /* Process COMMENT value */
         memcpy(pch, " /* ", 4);
         pch = pch + 4;
         freebytes = freebytes - 4;

         if (comment_len > (freebytes + 3)) /* Use as much of comment string as possible */
            memcpy(pch, rule_table[i].comment, freebytes);
         else /* Use entire comment string */
            memcpy(pch, rule_table[i].comment, comment_len);

         pch = pch + comment_len;
         memcpy(pch, " */", 3);
      } /* end if */

#if PLATFORM!=AS400
      strcat(Rule.rulebuff, "\n");
#endif

#if PLATFORM==AS400
      memset(AS400Rule.rulebuff, '\0', sizeof(AS400Rule.rulebuff));
      memset(AS400Rule.Rule.fill_12, BLNK, sizeof(AS400Rule.Rule.fill_12));
      memcpy(AS400Rule.Rule.buff, Rule.rulebuff, strlen(Rule.rulebuff));

      fwrite(AS400Rule.rulebuff, strlen(AS400Rule.rulebuff), 1, fpOutputFile);
#else
      fwrite(Rule.rulebuff, strlen(Rule.rulebuff), 1, fpOutputFile);
#endif

   } /* end for - Generate C-version of syntax rules */

   memset(Rule.rulebuff, '\0', sizeof(Rule.rulebuff));
   strcat(Rule.rulebuff, lastrule);

#if PLATFORM==AS400
   memset(AS400Rule.rulebuff, '\0', sizeof(AS400Rule.rulebuff));
   memset(AS400Rule.Rule.fill_12, BLNK, sizeof(AS400Rule.Rule.fill_12));
   memcpy(AS400Rule.Rule.buff, Rule.rulebuff, strlen(Rule.rulebuff));

   fwrite(AS400Rule.rulebuff, strlen(AS400Rule.rulebuff), 1, fpOutputFile);
#else
   fwrite(Rule.rulebuff, strlen(Rule.rulebuff), 1, fpOutputFile);
#endif

   return(SUCCESS);

} /* end DoPassThree */


/*****************************Documentation Start***********************************

  NAME: PrtGoToTable - Print the contents of the goto_table

******************************Documentation End************************************/
 
int PrtGoToTable()
{
   int i;

   printf("\n+++ DEBUG: goto_table contents follows\n");

   i = 0;

   while ((i < MAX_RULES) && (goto_table[i].ruleindex != -1)) {
      printf("+++ DEBUG: goto_table[%4i] label=%-8s, ruleindex=%4i\n", i,
              goto_table[i].label, goto_table[i].ruleindex);
      i++;
   } /* end while */

   return(SUCCESS);

} /* end PrtGoToTable */


/*****************************Documentation Start***********************************

  NAME: PrtRuleTable - Print the contents of the rule_table

******************************Documentation End************************************/
 
int PrtRuleTable()
{
   int i;

   printf("\n+++ DEBUG: rule_table contents follows\n");

   for (i = 0; i < ixR; i++) {
      printf("+++ DEBUG: rule_table[%4i] label=%-8s, tokentype=%-17s, nextlabel=%-8s, nextlabel_idx=%4i\n",
              i,rule_table[i].label, rule_table[i].tokentype, rule_table[i].nextlabel,
              rule_table[i].nextlabel_idx);
      printf("           string=%-24s, exit=%-22s, comment=\"%-46s\"\n", rule_table[i].string, 
              rule_table[i].exitname, rule_table[i].comment);
   } /* end for */

   return(SUCCESS);

} /* end PrtRuleTable */


/*****************************Documentation Start***********************************

  NAME: exit000 - exit1000 functions called by interpret

******************************Documentation End************************************/
 
int exit000(char token[], int tokenlen, char *errmsg) /* Process label */
{
   int i;


   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit000, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   if (memcmp(CtlCardBuff, token, tokenlen-1)) {
      printf(">>> LABEL MUST STARTING IN COLUMN 1\n");
      return(ERROR);
   } /* end if */

   if (!isdigit(token[0])) { /* is 1st char alpha? */
      for (i = 0; i < tokenlen; i++) { /* check for valid chars */
         if (!isalnum(token[i])) { /* is it an alphanumeric? */
            printf(">>> INVALID CHARACTER IN LABEL, %c\n", token[i]);
            return(ERROR);
         } /* end if */
      } /* end for */
   } /* end if */
   else {
      printf(">>> LABEL MUST BEGIN WITH AN ALPHABETIC CHARACTER\n");
      return(ERROR);
   } /* end else */

   if (tokenlen > 8) { 
      printf(">>> LABEL MUST BE 1-8 CHARACTERS\n");
      return(ERROR);
   } /* end if */

   strcat(temp_label, token);

   return(SUCCESS);

} /* end exit000 */

 
int exit005(char token[], int tokenlen, char *errmsg) /* Process @RULE */
{
   int i;
   int iFound;

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit005, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   ixR++; /* bump index into rule_table */

   if (strlen(temp_label) != 0) { /* if associated label exist, store it */
      strcat(rule_table[ixR].label, temp_label);

      i = 0;
      iFound = FALSE;

      while ((i < MAX_RULES) && (iFound == FALSE)) { /* add label to goto_table */
          if (goto_table[i].ruleindex == -1) {
             strcat(goto_table[i].label, temp_label);
             goto_table[i].ruleindex = ixR;
             iFound = TRUE;
          } /* end if */
          else {
             if (!strcmp(goto_table[i].label, temp_label)) {
                printf(">>> DUPLICATE LABEL: %s\n", temp_label);
                return(ERROR);
             } /* end if */
             i++;
          } /* end else */
      } /* end while */

      if (i == MAX_RULES) {
         printf(">>> goto_table is full\n");
         return(ERROR);
      } /* end if */

      memset(temp_label, '\0', strlen(temp_label));

   } /* end if */

   return(SUCCESS);

} /* end exit005 */


int exit010(char token[], int tokenlen, char *errmsg) /* Process TYPE=INITIAL|FINAL */
{

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit010, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   if (!strcmp(token,"INITIAL")) { /* TYPE=INITIAL */
      if (ixR != 0) {
         if (INITIAL_flg == TRUE)
            printf(">>> DUPLICATE TYPE=INITIAL STATEMENT\n");
         else
            printf(">>> TYPE=INITIAL MUST BE FIRST SYNTAX STATEMENT\n");
         return(ERROR);
      } /* end if */

      INITIAL_flg = TRUE;
      strcat(rule_table[ixR].tokentype, "STARTRULE");

   } /* end if */
   else { /* TYPE=FINAL */
      FINAL_flg = TRUE;
   } /* end else */

   return(SUCCESS);

} /* end exit010 */


int exit020(char token[], int tokenlen, char *errmsg) /* Process SYNTAXERR|LASTRULE */
{
  
   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit020, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   if (!strcmp(token, "SYNTAXERR"))
      strcat(rule_table[ixR].tokentype, "SYNTAXERR");
   else
      strcat(rule_table[ixR].tokentype, "LASTRULE");

   return(SUCCESS);

} /* end exit020 */


int exit030(char token[], int tokenlen, char *errmsg) /* Process CONTRULE|GOTORULE */
{

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit030, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   if (!strcmp(token, "CONTRULE"))
      strcat(rule_table[ixR].tokentype, "CONTRULE");
   else
      strcat(rule_table[ixR].tokentype, "GOTORULE");

   return(SUCCESS);

} /* end exit030 */


int exit040(char token[], int tokenlen, char *errmsg) /* Process NEXT=label for
                                                         CONTRULE|GOTORULE */
{

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit040, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   if (tokenlen > 8) { 
      printf(">>> NEXT=LABEL MUST BE 1-8 CHARACTERS\n");
      return(ERROR);
   } /* end if */

   strcat(rule_table[ixR].nextlabel, token);

   return(SUCCESS);

} /* end exit040 */


int exit050(char token[], int tokenlen, char *errmsg) /* Process tokentype */
{
   int i;
   int iFound;

   
   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit050, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   i = 0;
   iFound = FALSE;

   while ((tokentype_table[i].macro_tokentype != NULL) && (iFound == FALSE)){
      if (!strcmp(token, tokentype_table[i].macro_tokentype)) {
         iFound = TRUE;
         strcat(rule_table[ixR].tokentype, tokentype_table[i].C_tokentype);
      } /* end if */
      i++;
   } /* end while */

   if (iFound == FALSE) {
      printf(">>> TOKENTYPE NOT SUPPORTED\n");
      return(ERROR);
   } /* end if */

   return(SUCCESS);

} /* end exit050 */


int exit060(char token[], int tokenlen, char *errmsg) /* Process NEXT=label for tokentype */
{

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit060, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   if (tokenlen > 8) { 
      printf(">>> NEXT=LABEL MUST BE 1-8 CHARACTERS\n");
      return(ERROR);
   } /* end if */

   strcat(rule_table[ixR].nextlabel, token);

   return(SUCCESS);

} /* end exit060 */


int exit070(char token[], int tokenlen, char *errmsg) /* Process STRING, EXIT, COMMENT */
{

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit070, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   if (!memcmp(CtlCardBuff, token, tokenlen-1)) {
      printf(">>> CONTINUATION MUST BEGIN IN COLUMN > 1\n");
      return(ERROR);
   } /* end if */

   return(SUCCESS);

} /* end exit070 */


int exit080(char token[], int tokenlen, char *errmsg) /* Process STRING=s...s */
{

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit080, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   if (tokenlen > MAXSTRINGLEN) { 
      printf(">>> LENGTH OF STRING=S...S TOO LARGE\n");
      return(ERROR);
   } /* end if */

   strcat(rule_table[ixR].string, token);

   return(SUCCESS);

} /* end exit080 */


int exit090(char token[], int tokenlen, char *errmsg) /* Process EXIT=x...x */
{

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit090, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   if (tokenlen > MAXEXITNAMELEN) { 
      printf(">>> LENGTH OF EXIT=E...E TOO LARGE\n");
      return(ERROR);
   } /* end if */

   strcat(rule_table[ixR].exitname, token);

   return(SUCCESS);

} /* end exit090 */


int exit095(char token[], int tokenlen, char *errmsg) /* Process COMMENT=" */
{

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit095, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   memset(comment, '\0', sizeof(comment));

   return(SUCCESS);

} /* end exit095 */


int exit100(char token[], int tokenlen, char *errmsg) /* Process c...c for COMMENT */
{

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit100, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   strcat(comment, token);

   return(SUCCESS);

} /* end exit100 */


int exit105(char token[], int tokenlen, char *errmsg) /* Process ending " for COMMENT */
{

   if (DEBUG_FLAG)
      printf("+++ DEBUG: exit=exit105, token=%s, tokenlen=%d +++\n",
              token, tokenlen);

   if (strlen(comment) > MAXCOMMENTLEN) { 
      printf(">>> LENGTH OF COMMENT=\"...\" TOO LARGE\n");
      return(ERROR);
   } /* end if */

   memcpy(rule_table[ixR].comment, comment, strlen(comment));

   return(SUCCESS);

} /* end exit105 */
