*******************************************************************************
*                                                                             *
*   OS/400 README                                                             *
*                                                                             *
*   CBT: Parsing, Syntax Checking and Interpreting                            *
*                                                                             *
*   rtsujimoto@nyc.rr.com                                                     *
*                                                                             *
*******************************************************************************

Description
-----------
The README document describes how to install the executables, object modules, and other files required to
build and execute the tools, and sample programs.

NOTE: unless the restored library has the name RXT, in all likelihood, the sample programs will have to
      be rebuilt


Installation
------------
01. Issue CRTSAVF to create a called OS400SAVF of type SAVF to receive the FTP'd file
02. ftp the following file to your OS/400 system:

    ftp your.OS400.dns or IP address
    binary
    cd to.your.target.library
    put os400savf
    bye

03. Issue the RSTLIB command, using OS400SAVF as input, to restore the library

    NOTE: this command may require administrator authority


Running the sample programs
---------------------------
01. Enter: call pgm(pgmname), where pgmname is a sample program name, e.g. TESTPARSE


Build Instructions
------------------
01. FTP the following files to your OS/400 system and rename them accordingly:

    dfltdelim.h		QCSRC.DFLTDELIM
    interpret.c		QCSRC.INTERPRET
    interpret.h		H.INTERPRET
    parse.c		QCSRC.PARSE
    parse.h		H.PARSE
    syntaxchk.c		QCSRC.SYNTAXCHK
    syntaxchk.h		H.SYNTAXCHK
    syntblgen.c		QCSRC.SYNTBLGEN
    syntblgen.h		H.SYNTBLGEN
    TestINTERPRET.c	QCSRC.TESTINTRPT
    TestPARSE.c		QCSRC.TESTPARSE
    TestSYNTAXCHK.c	QCSRC.TESTSYNTXC
    Testsyntbl.h	H.TESTSYNTBL

01. To build the tools (PARSE, SYNTAXCHK, INTERPRET):

    a. Edit and modify the QMAKE members, as per your environment
    b. Edit and modfiy the members in QCLSRC, as per your environment
    c. Rebuild the object modules and/or executables:

       call pgm(makemod) parm(pgmname)

02. To build the sample programs:

    a. Edit and modify the sample programs in QCSRC (locate INPUTFILE)
    b. Edit and modify the QMAKE members, as per your environment
    c. Edit and modfiy the members in QCLSRC, as per your environment
    d. Rebuild the object modules and/or executables:

       call pgm(makemod) parm(pgmname)
