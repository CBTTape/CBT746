*******************************************************************************
*                                                                             *
*   Windows README                                                            *
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
01. Copy the *.exe and *.dat files to a directory of your choice
02. Copy the *.obj files to a directory of your choice
03. Copy the *.h files to a directory of your choice


Running the sample programs
---------------------------
01. Open a DOS window 
02. cd to the directory containing the executables
03. Enter the sample executable name, e.g. TestPARSE  


Build Instructions
------------------
01. Define a Project under Microsoft Visual C++
02. Copy the *.c files to a directory for that new Project
03. Specify the location of the source and include files for that project
04. Specify the directories used for include files and including object modules,
    e.g. Tools>Options>Directories
05. Set the platform specific flag in the source program, e.g. #define PLATFORM    WINDOWS
06. If you want to (re)build parse, syntaxchk and interpret, create object modules only
07. In the Project's link settings for user applications, specify parse.obj, syntaxchk.obj and/or interpret.obj in the 
    Object/library modules box, as needed, e.g. Project>Settings>Link