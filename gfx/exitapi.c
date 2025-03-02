/****************************************************************************
 * EXITapi  - Program Exiting routines
 *----------------------------------------------------------------------------
 * Copyright (C) 1992  Scott Hostynski All Rights Reserved
 *----------------------------------------------------------------------------
 *
 * Created by:  Scott Host
 * Date:        Oct, 1992
 *
 * CONTENTS: exitapi.c exitapi.h
 *
 * EXTERN MODULES - Main module of your program
 *
 * NOTES: Use EXIT_Install ( function pointer ) and set it to a routine
 *        that Shuts down all your systems
 *---------------------------------------------------------------------------*/
#include <conio.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "exitapi.h"

PRIVATE void ( *ShutDown )( int ) = (void( * )) 0;
PRIVATE char* clean_exit_msg = "Clean Exit";

/***************************************************************************
   EXIT_Install() Sets User routine to shut down systems
 ***************************************************************************/
void EXIT_Install( void ( *SD )( int ) // INPUT : pointer to function
) {
    ShutDown = SD;
}

/***************************************************************************
   EXIT_Error() - Terminate Program in Error with message
 ***************************************************************************/
void EXIT_Error(
    char* instr, // INPUT : message string ot format
    ... // INPUT : args for instr
) {
    va_list args;

    if ( ShutDown ) {
        ShutDown( 1 );
    }

    va_start( args, instr );
    vprintf( instr, args );
    va_end( args );

    exit( 1 );
}

/***************************************************************************
   EXIT_Assert() - Terminate Program in Error with Assertion
 ***************************************************************************/
void EXIT_Assert(
    char* expression, // INPUT : expression that failed
    char* filename, // INPUT : filename assert occured in
    unsigned linenum // INPUT : line number of assert statement
) {
    if ( ShutDown ) {
        ShutDown( 2 );
    }

    printf( "Assertion failed: %s, %s, %d\n", expression, filename, linenum );

    exit( 1 );
}

/***************************************************************************
   EXIT_Clean() - Terminate Program Clean
 ***************************************************************************/
void EXIT_Clean( void ) {
    if ( ShutDown ) {
        ShutDown( 0 );
    } else {
        printf( "\n%s\n", clean_exit_msg );
    }

    exit( 0 );
}
