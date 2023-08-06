/* DEFAULT Delimters
   NOTE: the default set of delimiters can be used as-is, or a user-provided list can be provided
         instead.  Any set of characters can be specified, as long as the list is terminated by
         PTNULL.  The second attribute associated with a given character in the delimiter list,
         can be set dynamically, giving the user the ability to use the same list in different
         ways.  The second attribute must be set to either YES, NO or TOGGLE. TOGGLE is the 
         same as YES, except that it also indicates the point after which PTPAD is either
         tokenized, or not tokenized.  For example, if we specify TOGGLE with PTDQUOTE, then
         each occurrences of PTPAD after that is tokenized until another PTDQUOTE is 
         encountered.  This is useful when a blank is considered part of the data and needs
         to be tokenized, e.g. " even a blank is data". */

char dfltdelim[33][2] = {
   PTPAD,YES,
   PTCOMMA,YES,
   PTEQUAL,YES,
   PTLPAREN,YES,
   PTRPAREN,YES,
   PTLT,YES,
   PTGT,YES,
   PTLBRACE,YES,
   PTRBRACE,YES,
   PTDASH,YES,
   PTUBAR,YES,
   PTAND,YES,
   PTPOUND,YES,
   PTAT,YES,
   PTPLUS,YES,
   PTSLASH,YES,
   PTPERCENT,YES,
   PTSTAR,YES,
   PTSCOLON,YES,
   PTCOLON,YES,
   PTSQUOTE,YES,
   PTDQUOTE,YES,
   PTTILDE,YES,
   PTBAR,YES,
   PTQUEST,YES,
   PTPERIOD,YES,
   PTEXCLAM,YES,
   PTBSLASH,YES,
   PTDOLLAR,YES,
   PTRVQUOT,YES,
   PTLBRACKET,YES,
   PTRBRACKET,YES,
   PTNULL};
