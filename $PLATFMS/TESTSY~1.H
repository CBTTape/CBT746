   struct syntax_table syntaxtab[500] = { /* User-specified syntax rules */
       /* rule-00 */ {STARTRULE},
       /* rule-01 */ {TokIsData,  GoTo6, "MYCMD"},
       /* rule-02 */ {SYNTAXERR},
       /* rule-03 */ {TokIsEOS,   GoTo5},      /* start: flush out rest of input */
       /* rule-04 */ {SYNTAXERR},
       /* rule-05 */ {LASTRULE},                /* end:   flush out rest of input */
       /* rule-06 */ {TokIsData,  GoTo8, "NAME"}, /* start: NAME */
       /* rule-07 */ {SYNTAXERR},		
       /* rule-08 */ {TokIsEqual, GoTo10},
       /* rule-09 */ {SYNTAXERR},
       /* rule-10 */ {TokIsData,  GoTo12, NULL, &exit010},
       /* rule-11 */ {SYNTAXERR},               /* end:   NAME */
       /* rule-12 */ {TokIsComma, GoTo14},
       /* rule-13 */ {SYNTAXERR},
       /* rule-14 */ {TokIsData,  GoTo22, "SCORES"},  /* start: SCORES */
       /* rule-15 */ {TokIsPlus,  GoTo17},
       /* rule-16 */ {SYNTAXERR},
       /* rule-17 */ {CONTRULE,   GoTo20},
       /* rule-18 */ {GOTORULE,   GoTo3},
       /* rule-19 */ {SYNTAXERR},
       /* rule-20 */ {TokIsData,  GoTo22, "SCORES"},  /* resume: SCORES */
       /* rule-21 */ {SYNTAXERR},
       /* rule-22 */ {TokIsEqual, GoTo24},
       /* rule-23 */ {SYNTAXERR},
       /* rule-24 */ {TokIsLeftParen, GoTo26},
       /* rule-25 */ {SYNTAXERR},
       /* rule-26 */ {TokIsNum,   GoTo28, NULL, &exit020},
       /* rule-27 */ {SYNTAXERR},
       /* rule-28 */ {TokIsRightParen, GoTo3},
       /* rule-29 */ {TokIsComma, GoTo31},
       /* rule-30 */ {SYNTAXERR},
       /* rule-31 */ {TokIsNum,  GoTo33, NULL, &exit030},
       /* rule-32 */ {SYNTAXERR},
       /* rule-33 */ {TokIsRightParen, GoTo3},
       /* rule-34 */ {SYNTAXERR},
   }; /* end of syntax table */
