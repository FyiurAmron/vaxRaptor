#ifndef WINDOWS_H
#define WINDOWS_H

#include "../gfx/types.h"

/***************************************************************************
WIN_WinGame () - WIndow text for winners of a game
 ***************************************************************************/
void WIN_WinGame(
    int game // INPUT : game number 0,1,2
);

/***************************************************************************
WIN_Msg () - Display a Message for ten secs or until user Acks somting
 ***************************************************************************/
void WIN_Msg(
    char* msg // INPUT : pointer to message to ask
);

/***************************************************************************
WIN_Pause () - Display a Pause Message Wait until user does somthing
 ***************************************************************************/
void WIN_Pause( void );

/***************************************************************************
WIN_Order () - Display a Pause Message Wait until user does somthing
 ***************************************************************************/
void WIN_Order( void );

/***************************************************************************
   WIN_AskBool () - Askes USER a YES/NO Question????
 ***************************************************************************/
bool WIN_AskBool(
    char* question // INPUT : pointer to message to ask
);

/***************************************************************************
WIN_AskExit () - Opens Windows and Askes if USer wants 2 quit
 ***************************************************************************/
void WIN_AskExit( void );

/***************************************************************************
WIN_MainMenu () - Main Menu
 ***************************************************************************/
void WIN_MainMenu( void );

/***************************************************************************
WIN_Register () - Register Window
 ***************************************************************************/
bool RAP_Register( void );

/***************************************************************************
WIN_Locker () - Locker Room Window
 ***************************************************************************/
int RAP_Locker( void );

/***************************************************************************
WIN_Hangar() - Does the hanger dude
 ***************************************************************************/
int WIN_Hangar( void );

/***************************************************************************
WIN_ShipComp () - Does Game Selection 1, 2 or 3
 ***************************************************************************/
bool WIN_ShipComp( void );

/***************************************************************************
WIN_LoadComp() - Shows computer loading screen
 ***************************************************************************/
void WIN_LoadComp( void );

/***************************************************************************
WIN_SetLoadLevel()
 ***************************************************************************/
void WIN_SetLoadLevel( int level );

/***************************************************************************
WIN_EndLoad () - Shows Ship computer while loading level
 ***************************************************************************/
void WIN_EndLoad( void );

/***************************************************************************
WIN_MainLoop() - Handles Locker/Register/Store/Hangar and starting game
 ***************************************************************************/
void WIN_MainLoop( void );

extern DWORD id_pics[4];

typedef enum {
    DEM_INTRO,
    DEM_CREDITS,
    DEM_DEMO1G1,
    DEM_DEMO2G1,
    DEM_DEMO3G1,
    DEM_DEMO1G2,
    DEM_DEMO2G2,
    DEM_DEMO3G2,
    DEM_DEMO1G3,
    DEM_DEMO2G3,
    DEM_DEMO3G3
} DEMOTYPE;

extern int opt_detail;

#endif
