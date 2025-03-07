
#include <conio.h>
#include <dos.h>
#include <io.h>

#include "../gfx/prefapi.h"
#include "../gfx/ptrapi.h"
#include "../gfx/tsmapi.h"
#include "../gfx/kbdapi.h"

#include "inc/file0000.inc"
#include "inc/file0001.inc"

#include "bonus.h"
#include "input.h"
#include "intro.h"
#include "loadsave.h"
#include "windows.h"

bool buttons[4] = { false, false, false, false };

ITYPE control = I_MOUSE;

#define MAX_ADDX 10
#define MAX_ADDY 8

PRIVATE int k_Up;
PRIVATE int k_Down;
PRIVATE int k_Left;
PRIVATE int k_Right;

PRIVATE int k_Fire;
PRIVATE int k_FireSp;
PRIVATE int k_ChangeSp;
PRIVATE int k_Mega;

PRIVATE int m_lookup[3] = { 0, 1, 2 };
PRIVATE int j_lookup[4] = { 0, 1, 2, 3 };

int f_addx = 0;
int f_addy = 0;

PRIVATE int g_addx = 0;
PRIVATE int g_addy = 0;
PRIVATE int xm;
PRIVATE int ym;
PRIVATE int tsm_id;

extern int joy_x;
extern int joy_y;

int demo_mode = DEMO_OFF;
PRIVATE RECORD playback[MAX_DEMO + 1];
PRIVATE int cur_play = 0;
PRIVATE char demo_name[32];
PRIVATE int demo_wave;
PRIVATE int demo_game;
PRIVATE int max_play;
PRIVATE bool control_pause = false;

/***************************************************************************
DEMO_MakePlayer() -
 ***************************************************************************/
void DEMO_MakePlayer( int game ) {
    memset( &plr, 0, sizeof( PLAYEROBJ ) );

    plr.sweapon = EMPTY;
    plr.diff[0] = DIFF_3;
    plr.diff[1] = DIFF_3;
    plr.diff[2] = DIFF_3;
    plr.id_pic = 0;

    RAP_SetPlayerDiff();

    OBJS_Add( S_FORWARD_GUNS );
    OBJS_Add( S_ENERGY );
    OBJS_Add( S_ENERGY );
    OBJS_Add( S_ENERGY );
    OBJS_Add( S_ENERGY );
    OBJS_Add( S_DETECT );
    plr.score = 10000;

    switch ( game ) {
        default:
            OBJS_Add( S_MICRO_MISSLE );
            OBJS_Add( S_MEGA_BOMB );
            OBJS_Add( S_MINI_GUN );
            OBJS_Add( S_AIR_MISSLE );
            OBJS_Add( S_TURRET );
            OBJS_Add( S_DEATH_RAY );
            break;

        case 1:
            OBJS_Add( S_MEGA_BOMB );
            OBJS_Add( S_MEGA_BOMB );
            OBJS_Add( S_MEGA_BOMB );
            OBJS_Add( S_MEGA_BOMB );
            OBJS_Add( S_SUPER_SHIELD );
            OBJS_Add( S_PLASMA_GUNS );
            OBJS_Add( S_MICRO_MISSLE );
            OBJS_Add( S_TURRET );
            OBJS_Add( S_GRD_MISSLE );
            plr.score += 327683;
            break;

        case 2:
            OBJS_Add( S_MEGA_BOMB );
            OBJS_Add( S_MEGA_BOMB );
            OBJS_Add( S_MEGA_BOMB );
            OBJS_Add( S_MEGA_BOMB );
            OBJS_Add( S_SUPER_SHIELD );
            OBJS_Add( S_PLASMA_GUNS );
            OBJS_Add( S_MICRO_MISSLE );
            OBJS_Add( S_FORWARD_LASER );
            OBJS_Add( S_GRD_MISSLE );
            plr.score += 876543;
            break;
    }

    OBJS_GetNext();
}

/***************************************************************************
DEMO_DisplayStats (
 ***************************************************************************/
void DEMO_DisplayStats( void ) {
    char temp[81];

    if ( demo_mode != DEMO_RECORD ) {
        return;
    }

    sprintf( temp, "REC %d", MAX_DEMO - cur_play );
    GFX_Print( MAP_LEFT + 5, 20, temp, GLB_GetItem( FONT2_FNT ), 84 );
}

/***************************************************************************
DEMO_StartRec (
 ***************************************************************************/
void DEMO_StartRec( void ) {
    SND_Patch( FX_BONUS, 127 );
    memset( playback, 0, sizeof( playback ) );
    demo_mode = DEMO_RECORD;
    cur_play = 1;
}

/***************************************************************************
DEMO_StartPlayback (
 ***************************************************************************/
void DEMO_StartPlayback( void ) {
    demo_mode = DEMO_PLAYBACK;
    cur_play = 1;
}

/***************************************************************************
DEMO_SetFileName ()
 ***************************************************************************/
void DEMO_SetFileName( char* in_name ) {
    strcpy( demo_name, in_name );
}

/***************************************************************************
DEMO_GLBFile (
 ***************************************************************************/
void DEMO_GLBFile( DWORD item ) {
    BYTE* mem;

    mem = GLB_GetItem( item );
    if ( !mem ) {
        return;
    }

    memcpy( playback, mem, sizeof( playback ) );

    cur_play = 1;

    max_play = playback[0].playerpic;
    demo_game = playback[0].px;
    demo_wave = playback[0].py;

    GLB_FreeItem( item );
}

/***************************************************************************
DEMO_LoadFile()
 ***************************************************************************/
void DEMO_LoadFile( void ) {
    int filesize;

    filesize = GLB_ReadFile( demo_name, NULL );
    GLB_ReadFile( demo_name, playback );

    cur_play = 1;

    max_play = playback[0].playerpic;
    demo_game = playback[0].px;
    demo_wave = playback[0].py;
}

/***************************************************************************
DEMO_SaveFile (
 ***************************************************************************/
void DEMO_SaveFile( void ) {
    if ( cur_play < 2 ) {
        return;
    }

    playback[0].px = cur_game;
    playback[0].py = game_wave[cur_game];
    playback[0].playerpic = cur_play;

    GLB_SaveFile( demo_name, playback, cur_play * sizeof( RECORD ) );
}

/***************************************************************************
DEMO_Play() - Demo playback routine
 ***************************************************************************/
bool // true=Aborted, false = timeout
DEMO_Play( void ) {
    extern DWORD songsg1[];
    extern DWORD songsg2[];
    extern DWORD songsg3[];
    bool rval = false;

    DEMO_StartPlayback();

    cur_game = demo_game;
    game_wave[cur_game] = demo_wave;

    DEMO_MakePlayer( demo_game );

    switch ( cur_game ) {
        default:
        case 0:
            SND_PlaySong( songsg1[demo_wave], true, true );
            break;
        case 1:
            SND_PlaySong( songsg2[demo_wave], true, true );
            break;
        case 2:
            SND_PlaySong( songsg3[demo_wave], true, true );
            break;
    }

    WIN_LoadComp();
    RAP_LoadMap();
    GFX_SetRetraceFlag( false );

    rval = Do_Game();

    GFX_SetRetraceFlag( true );

    if ( !rval ) {
        if ( OBJS_GetAmt( S_ENERGY ) <= 0 ) {
            SND_PlaySong( RAP5_MUS, true, true );
            INTRO_Death();
        } else {
            INTRO_Landing();
        }
    }

    RAP_ClearPlayer();

    demo_mode = DEMO_OFF;

    return rval;
}

/***************************************************************************
DEMO_Think (
 ***************************************************************************/
bool DEMO_Think( void ) {
    bool rval = false;

    switch ( demo_mode ) {
        default:
            rval = false;
            break;

        case DEMO_PLAYBACK:
            BUT_1 = playback[cur_play].b1;
            BUT_2 = playback[cur_play].b2;
            BUT_3 = playback[cur_play].b3;
            BUT_4 = playback[cur_play].b4;
            playerx = playback[cur_play].px;
            playery = playback[cur_play].py;
            player_cx = playerx + ( PLAYERWIDTH / 2 );
            player_cy = playery + ( PLAYERHEIGHT / 2 );
            playerpic = playback[cur_play].playerpic;
            cur_play++;
            if ( cur_play > max_play ) {
                demo_mode = DEMO_OFF;
                rval = true;
            }
            break;

        case DEMO_RECORD:
            playback[cur_play].b1 = (BYTE) BUT_1;
            playback[cur_play].b2 = (BYTE) BUT_2;
            playback[cur_play].b3 = (BYTE) BUT_3;
            playback[cur_play].b4 = (BYTE) BUT_4;
            playback[cur_play].px = (short) playerx;
            playback[cur_play].py = (short) playery;
            playback[cur_play].playerpic = (short) playerpic;
            cur_play++;
            if ( cur_play == MAX_DEMO ) {
                SND_Patch( FX_BONUS, 127 );
                demo_mode = DEMO_OFF;
                rval = true;
            }
            break;
    }

    return rval;
}

/*------------------------------------------------------------------------
   IPT_GetButtons () - Reads in Joystick and Keyboard game buttons
  ------------------------------------------------------------------------*/
void IPT_GetButtons( void ) {
    int num;

    if ( control == I_JOYSTICK ) {
        num = inp( 0x201 );

        num = num >> 4;

        if ( !( num & 1 ) ) {
            buttons[j_lookup[0]] = true;
        }
        if ( !( num & 2 ) ) {
            buttons[j_lookup[1]] = true;
        }
        if ( !( num & 4 ) ) {
            buttons[j_lookup[2]] = true;
        }
        if ( !( num & 8 ) ) {
            buttons[j_lookup[3]] = true;
        }
    }

    if ( KBD_Key( k_Fire ) ) {
        buttons[0] = true;
    }

    if ( KBD_Key( k_FireSp ) ) {
        buttons[1] = true;
    }

    if ( KBD_Key( k_ChangeSp ) ) {
        buttons[2] = true;
    }

    if ( KBD_Key( k_Mega ) ) {
        buttons[3] = true;
    }
}

/*------------------------------------------------------------------------
IPT_GetJoyStick()
  ------------------------------------------------------------------------*/
PRIVATE void IPT_GetJoyStick( void ) {
    extern int joy_sx, joy_sy;
    extern int joy_limit_xh, joy_limit_xl;
    extern int joy_limit_yh, joy_limit_yl;

    _disable();
    PTR_ReadJoyStick();
    _enable();

    xm = joy_x - joy_sx;
    ym = joy_y - joy_sy;

    if ( xm < joy_limit_xl || xm > joy_limit_xh ) {
        if ( xm < 0 ) {
            if ( g_addx >= 0 ) {
                g_addx = -1;
            }
            g_addx--;
            if ( -g_addx > MAX_ADDX ) {
                g_addx = -MAX_ADDX;
            }
        } else {
            if ( g_addx <= 0 ) {
                g_addx = 1;
            }
            g_addx++;
            if ( g_addx > MAX_ADDX ) {
                g_addx = MAX_ADDX;
            }
        }
    } else {
        if ( g_addx != 0 ) {
            g_addx /= 2;
        }
    }

    if ( ym < joy_limit_yl || ym > joy_limit_yh ) {
        if ( ym < 0 ) {
            if ( g_addy >= 0 ) {
                g_addy = -1;
            }

            g_addy--;
            if ( -g_addy > MAX_ADDY ) {
                g_addy = -MAX_ADDY;
            }
        } else {
            if ( g_addy <= 0 ) {
                g_addy = 1;
            }

            g_addy++;
            if ( g_addy > MAX_ADDY ) {
                g_addy = MAX_ADDY;
            }
        }
    } else {
        if ( g_addy != 0 ) {
            g_addy /= 2;
        }
    }
}

/*------------------------------------------------------------------------
IPT_GetKeyBoard (
  ------------------------------------------------------------------------*/
PRIVATE void IPT_GetKeyBoard( void ) {
    if ( KBD_Key( k_Left ) || KBD_Key( k_Right ) ) {
        if ( KBD_Key( k_Left ) ) {
            if ( g_addx >= 0 ) {
                g_addx = -1;
            }
            g_addx--;
            if ( -g_addx > MAX_ADDX ) {
                g_addx = -MAX_ADDX;
            }
        } else if ( KBD_Key( k_Right ) ) {
            if ( g_addx <= 0 ) {
                g_addx = 1;
            }
            g_addx++;
            if ( g_addx > MAX_ADDX ) {
                g_addx = MAX_ADDX;
            }
        }
    } else {
        if ( g_addx != 0 ) {
            g_addx /= 2;
        }
    }

    if ( KBD_Key( k_Up ) || KBD_Key( k_Down ) ) {
        if ( KBD_Key( k_Up ) ) {
            if ( g_addy >= 0 ) {
                g_addy = -1;
            }
            g_addy--;
            if ( -g_addy > MAX_ADDY ) {
                g_addy = -MAX_ADDY;
            }
        } else if ( KBD_Key( k_Down ) ) {
            if ( g_addy <= 0 ) {
                g_addy = 1;
            }
            g_addy++;
            if ( g_addy > MAX_ADDY ) {
                g_addy = MAX_ADDY;
            }
        }
    } else {
        if ( g_addy != 0 ) {
            g_addy /= 2;
        }
    }
}

/*------------------------------------------------------------------------
IPT_GetMouse (
  ------------------------------------------------------------------------*/
PRIVATE void IPT_GetMouse( void ) {
    int plx;
    int ply;
    int ptrx;
    int ptry;

    plx = playerx + ( PLAYERWIDTH / 2 );
    ply = playery + ( PLAYERHEIGHT / 2 );

    ptrx = cur_mx;
    ptry = cur_my;

    xm = ptrx - plx;
    ym = ptry - ply;

    if ( xm != 0 ) {
        xm = xm >> 3;

        if ( xm == 0 ) {
            xm = 1;
        } else if ( xm > 10 ) {
            xm = 10;
        } else if ( xm < -10 ) {
            xm = -10;
        }
    }

    if ( ym != 0 ) {
        ym = ym >> 3;

        if ( ym == 0 ) {
            ym = 1;
        } else if ( ym > 10 ) {
            ym = 10;
        } else if ( ym < -10 ) {
            ym = -10;
        }
    }

    g_addx = xm;
    g_addy = ym;

    if ( mouseb1 ) {
        buttons[m_lookup[0]] = true;
    }

    if ( mouseb2 ) {
        buttons[m_lookup[1]] = true;
    }

    if ( mouseb3 ) {
        buttons[m_lookup[2]] = true;
    }
}

PRIVATE void WaitJoyButton( void ) {
    int num = 0;

    for ( ;; ) {
        num = inp( 0x201 );
        num = num >> 4;

        if ( ( num & 0x0f ) != 0x0f ) {
            break;
        }
    }

    for ( ;; ) {
        num = inp( 0x201 );
        num = num >> 4;

        if ( ( num & 0x0f ) == 0xf ) {
            break;
        }
    }

    num = inp( 0x201 );
    num = inp( 0x201 );

    for ( ;; ) {
        num = inp( 0x201 );
        num = num >> 4;

        if ( ( num & 0x0f ) == 0xf ) {
            break;
        }
    }
}

/***************************************************************************
IPT_CalJoy() - Calibrates Joystick ( joystick must be centered )
 ***************************************************************************/
void IPT_CalJoy( void ) {
    extern bool godmode;
    extern int joy_sx, joy_sy;
    extern int joy_limit_xh, joy_limit_xl;
    extern int joy_limit_yh, joy_limit_yl;
    int xh, yh;
    int xl, yl;

    if ( control != I_JOYSTICK ) {
        return;
    }

    printf( "\n" );

    _disable();
    PTR_ReadJoyStick();
    _enable();

    printf( "CENTER Joystick and press a button\n" );
    fflush( stdout );
    WaitJoyButton();
    PTR_ReadJoyStick();
    _disable();
    PTR_ReadJoyStick();
    _enable();
    joy_sx = joy_x;
    joy_sy = joy_y;

    printf( "PUSH the Joystick in the UPPER LEFT and press a button\n" );
    fflush( stdout );
    WaitJoyButton();
    PTR_ReadJoyStick();
    _disable();
    PTR_ReadJoyStick();
    _enable();
    xl = joy_x;
    yl = joy_y;

    printf( "PUSH the Joystick in the LOWER RIGHT and press a button\n" );
    fflush( stdout );
    WaitJoyButton();
    PTR_ReadJoyStick();
    _disable();
    PTR_ReadJoyStick();
    _enable();
    xh = joy_x;
    yh = joy_y;

    if ( godmode ) {
        printf( "INPUT xh = %d    xl = %d\n", xh, xl );
        printf( "INPUT yh = %d    yl = %d\n", yh, yl );
    }

    joy_limit_xh = abs( xh - joy_sx ) / 3;
    joy_limit_xl = -( abs( xl - joy_sx ) / 3 );
    joy_limit_yh = abs( yh - joy_sy ) / 3;
    joy_limit_yl = -( abs( yl - joy_sy ) / 3 );

    if ( godmode ) {
        printf( "LIMIT xh = %d    xl = %d\n", joy_limit_xh, joy_limit_xl );
        printf( "LIMIT yh = %d    yl = %d\n", joy_limit_yh, joy_limit_yl );
        printf( "CENTER sx = %d    sy = %d\n", joy_sx, joy_sy );
    }

    printf( "\n" );
    fflush( stdout );
}

/***************************************************************************
IPT_Init () - Initializes INPUT system
 ***************************************************************************/
void IPT_Init( void ) {
    tsm_id = TSM_NewService( IPT_GetButtons, 26, 254, 1 );
    IPT_CalJoy();
}

/***************************************************************************
IPT_DeInit() - Freeze up resources used by INPUT system
 ***************************************************************************/
void IPT_DeInit( void ) {
    TSM_DelService( tsm_id );
}

/***************************************************************************
IPT_Start() - Tranfers control from PTRAPI stuff to IPT stuff
 ***************************************************************************/
void IPT_Start( void ) {
    PTR_DrawCursor( false );
    PTR_Pause( true );
    TSM_ResumeService( tsm_id );
}

/***************************************************************************
IPT_End() - Tranfers control from IPT stuff to PTR stuff
 ***************************************************************************/
void IPT_End( void ) {
    TSM_PauseService( tsm_id );
    PTR_Pause( false );
    PTR_DrawCursor( false );
}

/***************************************************************************
IPT_MovePlayer() - Perform player movement for current input device
 ***************************************************************************/
void IPT_MovePlayer( void ) {
    PRIVATE int oldx = PLAYERINITX;
    int delta;

    if ( demo_mode == DEMO_PLAYBACK ) {
        return;
    }

    if ( !control_pause ) {
        switch ( control ) {
            default:
            case I_KEYBOARD:
                IPT_GetKeyBoard();
                break;

            case I_JOYSTICK:
                IPT_GetJoyStick();
                break;

            case I_MOUSE:
                IPT_GetMouse();
                break;
        }
    }

    playerx += g_addx;
    playery += g_addy;

    if ( startendwave == EMPTY ) {
        if ( playery < MINPLAYERY ) {
            playery = MINPLAYERY;
            g_addy = 0;
        } else if ( playery > MAXPLAYERY ) {
            playery = MAXPLAYERY;
            g_addy = 0;
        }

        if ( playerx < PLAYERMINX ) {
            playerx = PLAYERMINX;
            g_addx = 0;
        } else if ( playerx + PLAYERWIDTH > PLAYERMAXX ) {
            playerx = PLAYERMAXX - PLAYERWIDTH;
            g_addx = 0;
        }
    }

    delta = abs( playerx - oldx );
    delta = delta >> 2;

    if ( delta > 3 ) {
        delta = 3;
    }

    if ( playerx < oldx ) {
        if ( playerpic < playerbasepic + delta ) {
            playerpic++;
        }
    } else if ( playerx > oldx ) {
        if ( playerpic > playerbasepic - delta ) {
            playerpic--;
        }
    } else {
        if ( playerpic > playerbasepic ) {
            playerpic--;
        } else if ( playerpic < playerbasepic ) {
            playerpic++;
        }
    }

    oldx = playerx;

    player_cx = playerx + ( PLAYERWIDTH / 2 );
    player_cy = playery + ( PLAYERHEIGHT / 2 );
}

/***************************************************************************
IPT_PauseControl() - Lets routines run without letting user control anyting
 ***************************************************************************/
void IPT_PauseControl( bool flag ) {
    control_pause = flag;
}

/***************************************************************************
IPT_FMovePlayer() - Forces player to move addx/addy
 ***************************************************************************/
void IPT_FMovePlayer(
    int addx, // INPUT : add to x pos
    int addy // INPUT : add to y pos
) {
    g_addx = addx;
    g_addy = addy;

    IPT_MovePlayer();
}

/***************************************************************************
IPT_LoadPrefs() - Load Input Prefs from setup.ini
 ***************************************************************************/
void IPT_LoadPrefs( void ) {
    extern int  bday_num, seed;
    extern bool tai_flag, quick_mode, godmode;

    opt_detail = INI_GetPreferenceLong( "Setup", "Detail", 1 );
    control = INI_GetPreferenceLong( "Setup", "Control", 0 );

    bday_num = INI_GetPreferenceLong( "Setup", "BirthdayNum", EMPTY );
    seed = INI_GetPreferenceLong( "Setup", "Seed", EMPTY );

    quick_mode = INI_GetPreferenceBool( "Setup", "QuickMode", false );
    tai_flag = INI_GetPreferenceBool( "Setup", "TaiwanFlag", false );
    godmode = INI_GetPreferenceBool( "Setup", "GodMode", false );

    k_Up = INI_GetPreferenceLong( "Keyboard", "MoveUp", SC_UP );
    k_Down = INI_GetPreferenceLong( "Keyboard", "MoveDn", SC_DOWN );
    k_Left = INI_GetPreferenceLong( "Keyboard", "MoveLeft", SC_LEFT );
    k_Right = INI_GetPreferenceLong( "Keyboard", "MoveRight", SC_RIGHT );
    k_Fire = INI_GetPreferenceLong( "Keyboard", "Fire", SC_CTRL );
    k_FireSp = INI_GetPreferenceLong( "Keyboard", "FireSp", SC_ALT );
    k_ChangeSp = INI_GetPreferenceLong( "Keyboard", "ChangeSp", SC_SPACE );
    k_Mega = INI_GetPreferenceLong( "Keyboard", "MegaFire", SC_RIGHT_SHIFT );

    m_lookup[0] = INI_GetPreferenceLong( "Mouse", "Fire", 0 );
    m_lookup[1] = INI_GetPreferenceLong( "Mouse", "FireSp", 1 );
    m_lookup[2] = INI_GetPreferenceLong( "Mouse", "ChangeSp", 2 );

    j_lookup[0] = INI_GetPreferenceLong( "JoyStick", "Fire", 0 );
    j_lookup[1] = INI_GetPreferenceLong( "JoyStick", "FireSp", 1 );
    j_lookup[2] = INI_GetPreferenceLong( "JoyStick", "ChangeSp", 2 );
    j_lookup[3] = INI_GetPreferenceLong( "JoyStick", "MegaFire", 3 );
}
