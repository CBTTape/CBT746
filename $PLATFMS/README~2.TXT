*******************************************************************************
*                                                                             *
*   HP-UX README                                                              *
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

NOTE: no compilations of the tools or sample programs are required to run the sample executables, or to 
      incorporate the tools into your applications


Installation
------------
01. ftp the following file to your HP-UX system:

    ftp your.HPUX.dns or IP address
    binary
    put hpux_cbt.tar.Z
    bye

02. Unpack the files:

    uncompress hpux_cbt.tar
    tar -xvf hpux_cbt.tar

Running the sample programs
---------------------------
01. cd to the directory containing the unpacked files
02. Enter the sample executable name, e.g. TestPARSE  


Build Instructions
------------------
01. FTP the following files to your HP-UX system:

    dfltdelim.h
    interpret.c
    interpret.h
    parse.c
    parse.h
    syntaxchk.c
    syntaxchk.h
    syntblgen.c
    syntblgen.h
    TestINTERPRET.c
    TestPARSE.c
    TestSYNTAXCHK.c
    Testsyntbl.h

02. Set the platform specific flag in the source program, e.g. #define PLATFORM    UNIX 

03. To build the tools (parse, syntaxchk, interpret):

    makeobj tool.name

04. To build syntblgen and the sample programs:

    a. Edit and modify the makefile for the target program, so that it conforms to your environment, e.g. makefile.pgmname
    b. enter: makemod pgmname
