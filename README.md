# CBT746
Converted to GitHub via [cbt2git](https://github.com/wizardofzos/cbt2git)

This is still a work in progress. GitHub repos will be deleted and created during this period...

```
//***FILE 746 is from Richard Tsujimoto and contains the MVS        *   FILE 746
//*           version of his Parser, that was featured in a         *   FILE 746
//*           three-part article in "Technical Support" magazine    *   FILE 746
//*           from NaSPA, (April, May, June 2006).  The materials   *   FILE 746
//*           for other platforms supported by this parser, are     *   FILE 746
//*           included in a zipped file, which is member $PLATFMS   *   FILE 746
//*           in this pds.                                          *   FILE 746
//*                                                                 *   FILE 746
//*         CBT: Parsing, Syntax Checking and Interpreting          *   FILE 746
//*                                                                 *   FILE 746
//*         email:  rtsujimoto@nyc.rr.com                           *   FILE 746
//*                                                                 *   FILE 746
//*     Other platforms for which this parser is supported, are     *   FILE 746
//*     supported by C language code.  MVS is supported in this     *   FILE 746
//*     pds, by Assembler code.  The C language code needed for     *   FILE 746
//*     "other platform support" is included in a zip file, that    *   FILE 746
//*     is member $PLATFMS in this pds.  To use this zip file,      *   FILE 746
//*     just download it in BINARY to a PC, and unzip it.           *   FILE 746
//*     Instructions for each of the other platforms supported,     *   FILE 746
//*     are included there, and you can see how to proceed          *   FILE 746
//*     further for each platform.                                  *   FILE 746
//*                                                                 *   FILE 746
//*     The other platforms supported by C code are:                *   FILE 746
//*        Windows, AIX, HPUX, and OS-400.                          *   FILE 746
//*                                                                 *   FILE 746
//*     Description                                                 *   FILE 746
//*     -----------                                                 *   FILE 746
//*     The $README member describes how to install the             *   FILE 746
//*     executables, object modules, and other files required       *   FILE 746
//*     to build and execute the tools, and sample programs.        *   FILE 746
//*                                                                 *   FILE 746
//*     NOTE: No compilations of the tools or sample programs       *   FILE 746
//*           are required to run the sample executables, or to     *   FILE 746
//*           incorporate the tools into your applications          *   FILE 746
//*                                                                 *   FILE 746
//*      Detailed documentation is found in the $$DOC member,       *   FILE 746
//*      which is in Microsoft WORD format.  To read this           *   FILE 746
//*      document, you must FTP it, or otherwise "file              *   FILE 746
//*      transfer" it in BINARY to a PC or other machine that       *   FILE 746
//*      can read Microsoft WORD.  Then you must read the           *   FILE 746
//*      document on that machine.                                  *   FILE 746
//*                                                                 *   FILE 746
//*     Installation                                                *   FILE 746
//*     ------------                                                *   FILE 746
//*     01. The four TSO XMIT-format datasets are already           *   FILE 746
//*          loaded onto an MVS system, if you are reading this.    *   FILE 746
//*                                                                 *   FILE 746
//*         These are the JCL, LOADLIB, MACLIB, and SOURCE          *   FILE 746
//*          members of the CBT File 746.                           *   FILE 746
//*                                                                 *   FILE 746
//*     02. Use TSO RECEIVE to restore the data sets:               *   FILE 746
//*                                                                 *   FILE 746
//*         (See sample job $TSORECV to do all of this at           *   FILE 746
//*         once.)                                                  *   FILE 746
//*                                                                 *   FILE 746
//*         RECEIVE USERID(your.userid) INDS(RTI.JCL.XMI)           *   FILE 746
//*                                                                 *   FILE 746
//*         When prompted for the restore parameters, enter:        *   FILE 746
//*                                                                 *   FILE 746
//*         DSN('whatever you want to call it')                     *   FILE 746
//*                                                                 *   FILE 746
//*     03. Repeat Step 02 for the remaining data sets              *   FILE 746
//*          (or use the $TSORECV sample job).                      *   FILE 746
//*                                                                 *   FILE 746
//*     Running the sample programs                                 *   FILE 746
//*     ---------------------------                                 *   FILE 746
//*     01. Edit the JCL library using ISPF                         *   FILE 746
//*     02. Select a member and modify the JCL accordingly          *   FILE 746
//*     03. Submit the job                                          *   FILE 746
//*                                                                 *   FILE 746
//*     Build Instructions                                          *   FILE 746
//*     ------------------                                          *   FILE 746
//*     01. Edit the JCL library using ISPF                         *   FILE 746
//*                                                                 *   FILE 746
//*     02. To build the tools:                                     *   FILE 746
//*                                                                 *   FILE 746
//*         a. Select member BLDTOOLS and modify the JCL            *   FILE 746
//*            accordingly                                          *   FILE 746
//*         b. Submit the job                                       *   FILE 746
//*                                                                 *   FILE 746
//*     03. To build the sample programs:                           *   FILE 746
//*                                                                 *   FILE 746
//*         a. Select member BLDSAMPL and modify the JCL            *   FILE 746
//*            accordingly                                          *   FILE 746
//*         b. Submit the job                                       *   FILE 746
//*                                                                 *   FILE 746
//*         NOTE:                                                   *   FILE 746
//*                                                                 *   FILE 746
//*         1. T@SYNTBL and T@UEXITS MUST BE BUILT BEFORE           *   FILE 746
//*            T@SYNTXC                                             *   FILE 746
//*                                                                 *   FILE 746
//*         2. T@SYNTBL MUST BE LINKED WITH NCAL 3. The sample      *   FILE 746
//*            code shows how the syntax table and user exits       *   FILE 746
//*            can be built separate from each other, and the       *   FILE 746
//*            main program.  In the simplest case, the syntax      *   FILE 746
//*            table and user exits can be coded in the main        *   FILE 746
//*            program.                                             *   FILE 746
//*                                                                 *   FILE 746
```
