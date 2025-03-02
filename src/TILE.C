
#include "../gfx/gfxapi.h"

#include "public.h"

#include "anims.h"
#include "fx.h"

#include "tile.h"
#include "tile_a.h"

#define MAX_STILES ( MAP_ONSCREEN * MAP_COLS )

DWORD titems[MAP_SIZE];
DWORD eitems[MAP_SIZE];
short hits[MAP_SIZE];
short money[MAP_SIZE];
short tdead[MAP_SIZE];
DWORD startflat[4];
int tilepos;
int tileyoff;

int g_mapleft;
BYTE* tilepic;
BYTE* tilestart;
int tileloopy;

PRIVATE char game_start[4][17] = { "STARTG1TILES", "STARTG2TILES", "STARTG3TILES", "STARTG4TILES" };

bool scroll_flag;
bool last_tile;

PRIVATE TILESPOT tspots[MAX_STILES];
PRIVATE TILESPOT* lastspot;

TILEDELAY first_delay;
TILEDELAY last_delay;
TILEDELAY* free_delay;

#define MAX_TILEDELAY ( ( MAP_ONSCREEN + 1 ) * MAP_COLS )
PRIVATE TILEDELAY tdel[MAX_TILEDELAY];

PRIVATE int spark_delay = 0;
PRIVATE int flare_delay = 0;

/*-------------------------------------------------------------------------*
TClear () - Clears TILE Delay Link List
 *-------------------------------------------------------------------------*/
PRIVATE void TClear( void ) {
    int i;

    first_delay.prev = NULL;
    first_delay.next = &last_delay;

    last_delay.prev = &first_delay;
    last_delay.next = NULL;

    free_delay = tdel;

    memset( tdel, 0, sizeof( tdel ) );

    for ( i = 0; i < MAX_TILEDELAY - 1; i++ ) {
        tdel[i].next = &tdel[i + 1];
    }
}

/*-------------------------------------------------------------------------*
TGet () - Get a TDELAY object
 *-------------------------------------------------------------------------*/
PRIVATE TILEDELAY* TGet( void ) {
    TILEDELAY* pNew;

    if ( !free_delay ) {
        EXIT_Error( "TILEDELAY_Get() - Max " );
    }

    pNew = free_delay;
    free_delay = free_delay->next;

    memset( pNew, 0, sizeof( TILEDELAY ) );

    pNew->next = &last_delay;
    pNew->prev = last_delay.prev;
    last_delay.prev = pNew;
    pNew->prev->next = pNew;

    return pNew;
}

/*-------------------------------------------------------------------------*
TRemove () - Remove a TDELAY Object from link list
 *-------------------------------------------------------------------------*/
PRIVATE TILEDELAY* TRemove( TILEDELAY* sh ) {
    TILEDELAY* next;

    next = sh->prev;

    sh->next->prev = sh->prev;
    sh->prev->next = sh->next;

    memset( sh, 0, sizeof( TILEDELAY ) );

    sh->next = free_delay;

    free_delay = sh;

    return next;
}

/*--------------------------------------------------------------------------
TILE_DoDamage
 --------------------------------------------------------------------------*/
PRIVATE void TILE_DoDamage( int mapspot, int damage ) {
    static int mlookup[3] = { -1, -MAP_COLS, 1 };
    static int xlookup[3] = { -1, 0, 1 };
    int ix = mapspot % MAP_COLS;
    int i;
    int spot;
    int x;

    for ( i = 0; i < 3; i++ ) {
        spot = mapspot + mlookup[i];

        if ( spot < 0 ) {
            continue;
        }
        if ( spot >= MAP_SIZE ) {
            continue;
        }

        if ( eitems[spot] == titems[spot] ) {
            continue;
        }

        x = ix + xlookup[i];
        if ( x < 0 ) {
            continue;
        }
        if ( x >= MAP_COLS ) {
            continue;
        }

        hits[spot] -= damage;
    }
}

/***************************************************************************
TILE_Draw () - Draws 32 by 32 TILE Clips on y only
 ***************************************************************************/
void TILE_Put(
    BYTE* inpic, // INPUT : pointer to GFX_PIC ( norm )
    int x, // INPUT : x position
    int y // INPUT : y position
) {
    bool flag = false;

    if ( y + 32 <= 0 ) {
        return;
    }
    if ( y >= SCREENHEIGHT ) {
        return;
    }

    tileloopy = 32;
    tilepic = inpic;

    if ( y < 0 ) {
        tilepic -= y * 32;
        tileloopy += y;
        y = 0;
        flag = true;
    }

    if ( y + tileloopy > SCREENHEIGHT ) {
        tileloopy = SCREENHEIGHT - y;
        flag = true;
    }

    tilestart = displaybuffer + x + ylookup[y];

    TILE_Draw( flag ? tileloopy : 32 );
}

/***************************************************************************
TILE_Init () - Sets Up A level for Displaying
 ***************************************************************************/
void TILE_Init( void ) {

    TClear();
    g_mapleft = MAP_LEFT;

    scroll_flag = true;
    last_tile = false;

    // FIND START OF GAME TILES ========
    startflat[0] = GLB_GetItemID( game_start[0] );
    startflat[0]++;

    startflat[1] = GLB_GetItemID( game_start[1] );
    startflat[1]++;

    startflat[2] = GLB_GetItemID( game_start[2] );
    startflat[2]++;

    startflat[3] = GLB_GetItemID( game_start[3] );
    startflat[3]++;
}

/***************************************************************************
TILE_CacheLevel () - Cache tiles in current level
 ***************************************************************************/
void TILE_CacheLevel( void ) {
    int game;
    FLATS* lib;
    DWORD item;
    int i;

    TClear();
    g_mapleft = MAP_LEFT;

    // SET UP START POS ON MAP =========
    tilepos = ( MAP_ROWS - MAP_ONSCREEN ) * MAP_COLS;
    tileyoff = 200 - MAP_ONSCREEN * MAP_BLOCKSIZE;
    lastspot = tspots + ( MAX_STILES - 1 );

    scroll_flag = true;
    last_tile = false;

    memset( titems, 0, sizeof( titems ) );
    memset( eitems, 0, sizeof( eitems ) );
    memset( hits, 0, sizeof( hits ) );
    memset( tdead, 0, sizeof( tdead ) );

    // == CACHE TILES =========================
    for ( i = 0; i < MAP_SIZE; i++ ) {
        game = ml->map[i].fgame;
        lib = flatlib[game];

        money[i] = lib[ml->map[i].flats].bounty;

        item = startflat[game];
        item += ml->map[i].flats;
        titems[i] = item;
        GLB_CacheItem( item );

        item = startflat[game];
        item += (DWORD) lib[ml->map[i].flats].linkflat;
        eitems[i] = item;
        if ( eitems[i] != titems[i] ) {
            GLB_CacheItem( item );
        }

        if ( eitems[i] != titems[i] ) {
            hits[i] = lib[ml->map[i].flats].bonus;
        } else {
            hits[i] = 1;
        }
    }
}

/***************************************************************************
TILE_FreeLevel () - Free tile level
 ***************************************************************************/
void TILE_FreeLevel( void ) {
    int i;

    for ( i = 0; i < MAP_SIZE; i++ ) {
        GLB_FreeItem( titems[i] );

        if ( eitems[i] != titems[i] ) {
            GLB_FreeItem( eitems[i] );
        }
    }
}

/***************************************************************************
TILE_DamageAll () - Damages All tiles on screen
 ***************************************************************************/
void TILE_DamageAll( void ) {
    TILESPOT* ts = tspots;

    for ( ;; ) {
        if ( eitems[ts->mapspot] != titems[ts->mapspot] ) {
            hits[ts->mapspot] -= 20;
        }

        if ( ts == lastspot ) {
            break;
        }

        ts++;
    }
}

/***************************************************************************
TILE_Think () - Does Position Calculations for tiles
 ***************************************************************************/
void TILE_Think( void ) {
    TILESPOT* ts;
    TILEDELAY* td;
    int ix;
    int iy;
    int mapspot;
    int x;
    int y;
    int tx;
    int ty;

    y = tileyoff;
    mapspot = tilepos;

    ts = tspots;

    for ( iy = 0; iy < MAP_ONSCREEN; iy++, y += 32 ) {
        x = MAP_LEFT;
        for ( ix = 0; ix < MAP_COLS; ix++, x += 32, mapspot++, ts++ ) {
            ts->mapspot = mapspot;
            ts->x = x;
            ts->y = y;
            ts->item = titems[mapspot];

            if ( hits[mapspot] < 0 && !tdead[mapspot] ) {
                SND_3DPatch( FX_GEXPLO, x + 16, y + 16 );

                TILE_DoDamage( ts->mapspot, 5 );

                plr.score += money[mapspot];

                TILE_Explode( ts, 10 );
                ANIMS_StartAnim( A_LARGE_GROUND_EXPLO1, x + 16, y + 16 );

                tdead[mapspot] = 1;
            }
        }
    }

    for ( td = first_delay.next; td != &last_delay; td = td->next ) {
        if ( td->mapspot - tilepos > MAP_ONSCREEN * MAP_COLS ) {
            td = TRemove( td );
            continue;
        }

        if ( td->frames < 0 ) {
            tx = td->ts->x + 8 + random( 8 );
            ty = td->ts->y + 16 + 10;

            TILE_DoDamage( td->mapspot, 20 );

            spark_delay++;
            flare_delay++;

            if ( spark_delay > 2 ) {
                ANIMS_StartAnim( A_GROUND_SPARKLE, tx, ty );
                spark_delay = 0;
            }

            if ( flare_delay > 4 ) {
                ANIMS_StartAnim( A_GROUND_FLARE, tx, ty );
                flare_delay = 0;
            }

            ts = tspots + ( td->mapspot - tilepos );
            titems[ts->mapspot] = td->item;
            eitems[ts->mapspot] = td->item;
            td = TRemove( td );
            continue;
        }

        td->frames--;
    }
}

/***************************************************************************
TILE_Display () - Displays Tiles
 ***************************************************************************/
void TILE_Display( void ) {
    TILESPOT* ts = tspots;
    BYTE* pic;

    for ( ;; ) {
        pic = GLB_GetItem( ts->item );
        pic += sizeof( GFX_PIC );
        TILE_Put( pic, ts->x, ts->y );

        if ( ts == lastspot ) {
            break;
        }

        ts++;
    }

    tileyoff++;

    if ( tileyoff > 0 ) {
        if ( last_tile && tileyoff >= 0 ) {
            tileyoff = 0;
            scroll_flag = false;
        } else {
            tileyoff -= MAP_BLOCKSIZE;
            tilepos -= MAP_COLS;
        }

        if ( tilepos <= 0 ) {
            tilepos = 0;
            last_tile = true;
        }
    }
}

/***************************************************************************
TILE_IsHit () - Checks to see if a shot hits an explodable tile
 ***************************************************************************/
bool TILE_IsHit(
    int damage, // INPUT : damage to tile
    int x, // INOUT : x screen pos, out tile x
    int y // INOUT : y screen pos, out tile y
) {
    TILESPOT* ts = tspots;

    while ( ts != lastspot ) {
        if ( x >= ts->x && x < ts->x + 32 && y >= ts->y && y < ts->y + 32 ) {
            if ( eitems[ts->mapspot] != titems[ts->mapspot] ) {
                hits[ts->mapspot] -= damage;

                switch ( random( 2 ) ) {
                    case 0:
                        ANIMS_StartGAnim( A_BLUE_SPARK, x, y );
                        break;
                    case 1:
                        ANIMS_StartGAnim( A_ORANGE_SPARK, x, y );
                        break;
                }

                return true;
            }
        }

        ts++;
    }

    return false;
}

/***************************************************************************
TILE_Bomb () - Checks to see if a BOMB hits an explodable tile
 ***************************************************************************/
bool // RETURNS : true = Tile Hit
TILE_Bomb(
    int damage, // INPUT : damage to tile
    int x, // INOUT : x screen pos, out tile x
    int y // INOUT : y screen pos, out tile y
) {
    TILESPOT* ts = tspots;

    while ( ts != lastspot ) {
        if ( x >= ts->x && x < ts->x + 32 && y >= ts->y && y < ts->y + 32 ) {
            if ( eitems[ts->mapspot] != titems[ts->mapspot] ) {
                hits[ts->mapspot] -= damage;

                TILE_DoDamage( ts->mapspot, damage );

                if ( ts->mapspot > MAP_COLS ) {
                    TILE_DoDamage( ts->mapspot - MAP_COLS, damage >> 1 );
                }

                return true;
            }
        }

        ts++;
    }

    return false;
}

/***************************************************************************
TILE_Explode () - Sets the Tile to show explosion tile
 ***************************************************************************/
void TILE_Explode(
    TILESPOT* ts, // INPUT : tilespot of explosion
    int delay // INPUT : frames to delay
) {
    TILEDELAY* td;

    if ( ts->mapspot == EMPTY || ts->mapspot < tilepos ) {
        return;
    }

    if ( delay ) {
        td = TGet();
        if ( td ) {
            td->ts = ts;
            td->mapspot = ts->mapspot;
            td->frames = delay;
            td->item = eitems[ts->mapspot];
        }
        eitems[ts->mapspot] = titems[ts->mapspot];
    } else {
        ts->item = eitems[ts->mapspot];
        titems[ts->mapspot] = eitems[ts->mapspot];
    }
}
