
#include "public.h"

#include "inc/file0001.inc"

#include "bonus.h"

PRIVATE BONUS bons[MAX_BONUS];

PRIVATE BONUS first_bonus;
PRIVATE BONUS last_bonus;
PRIVATE BONUS* free_bonus;
PRIVATE DWORD glow[4];
PRIVATE int glow_lx;
PRIVATE int glow_ly;
PRIVATE int xpos[16] = { -1, 0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2 };
PRIVATE int ypos[16] = { -3, -3, -3, -2, -1, 0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2 };
PRIVATE int energy_count;

/***************************************************************************
BONUS_Clear () - Clears out All bonuses
 ***************************************************************************/
void BONUS_Clear( void ) {
    int i;

    energy_count = 0;

    first_bonus.prev = NULL;
    first_bonus.next = &last_bonus;

    last_bonus.prev = &first_bonus;
    last_bonus.next = NULL;

    free_bonus = bons;

    memset( bons, 0, sizeof( bons ) );

    for ( i = 0; i < MAX_BONUS - 1; i++ ) {
        bons[i].next = &bons[i + 1];
    }
}

/*-------------------------------------------------------------------------*
BONUS_Get () - Gets A Free BONUS from Link List
 *-------------------------------------------------------------------------*/
BONUS* BONUS_Get( void ) {
    BONUS* pNew;

    if ( !free_bonus ) {
        return NULL;
    }

    pNew = free_bonus;
    free_bonus = free_bonus->next;

    memset( pNew, 0, sizeof( BONUS ) );

    pNew->next = &last_bonus;
    pNew->prev = last_bonus.prev;
    last_bonus.prev = pNew;
    pNew->prev->next = pNew;

    return pNew;
}

/*-------------------------------------------------------------------------*
BONUS_Remove () Removes BONUS from Link List
 *-------------------------------------------------------------------------*/
BONUS* BONUS_Remove( BONUS* sh ) {
    BONUS* next;

    if ( sh->type == S_ITEMBUY6 ) {
        energy_count--;
    }

    next = sh->prev;

    sh->next->prev = sh->prev;
    sh->prev->next = sh->next;

    memset( sh, 0, sizeof( BONUS ) );

    sh->next = free_bonus;

    free_bonus = sh;

    return next;
}

/***************************************************************************
BONUS_Init () - Sets up Bonus stuff
 ***************************************************************************/
void BONUS_Init( void ) {
    int i;
    GFX_PIC* h;

    for ( i = 0; i < 4; i++ ) {
        glow[i] = ICNGLW_BLK + (DWORD) i;
    }

    h = (GFX_PIC*) GLB_CacheItem( ICNGLW_BLK );

    glow_lx = h->width;
    glow_ly = h->height;

    GLB_CacheItem( ICNGLW_BLK + 1 );
    GLB_CacheItem( ICNGLW_BLK + 2 );
    GLB_CacheItem( ICNGLW_BLK + 3 );

    BONUS_Clear();
}

/***************************************************************************
BONUS_Add () - Adds A BONUS to Game so player can Try to pick it up
 ***************************************************************************/
void BONUS_Add(
    OBJ_TYPE type, // INPUT : OBJECT TYPE
    int x, // INPUT : X POSITION
    int y // INPUT : Y POSITION
) {
    BONUS* cur;

    if ( type >= S_LAST_OBJECT ) {
        return;
    }

    if ( type == S_ITEMBUY6 ) {
        if ( energy_count > MAX_MONEY ) {
            return;
        }
    }

    cur = BONUS_Get();
    if ( !cur ) {
        return;
    }

    if ( type == S_ITEMBUY6 ) {
        energy_count++;
    }

    cur->type = type;
    cur->lib = OBJS_GetLib( type );
    cur->curframe = 0;
    cur->x = x + MAP_LEFT;
    cur->y = y;
    cur->pos = random( 16 );
}

/***************************************************************************
BONUS_Think () - Does all BONUS Thinking
 ***************************************************************************/
void BONUS_Think( void ) {
    PRIVATE int gcnt = 0;
    BONUS* cur;
    int x = playerx;
    int y = playery;
    int x2 = playerx + PLAYERWIDTH;
    int y2 = playery + PLAYERHEIGHT;

    for ( cur = first_bonus.next; cur != &last_bonus; cur = cur->next ) {
        cur->item = cur->lib->item + cur->curframe;

        cur->bx = cur->x - BONUS_WIDTH / 2 + xpos[cur->pos];
        cur->by = cur->y - BONUS_HEIGHT / 2 + ypos[cur->pos];

        cur->gx = cur->x - ( glow_lx >> 1 ) + xpos[cur->pos];
        cur->gy = cur->y - ( glow_ly >> 1 ) + ypos[cur->pos];

        cur->y++;

        if ( gcnt & 1 ) {
            cur->pos++;

            if ( cur->pos >= 16 ) {
                cur->pos = 0;
            }

            cur->curframe++;

            if ( cur->curframe >= cur->lib->numframes ) {
                cur->curframe = 0;
            }
        }

        cur->curglow++;

        if ( cur->curglow >= 4 ) {
            cur->curglow = 0;
        }

        if ( cur->x > x && cur->x < x2 && cur->y > y && cur->y < y2 ) {
            if ( !cur->dflag && OBJS_GetAmt( S_ENERGY ) > 0 ) {
                SND_Patch( FX_BONUS, 127 );

                if ( cur->type == S_ENERGY ) {
                    OBJS_AddEnergy( MAX_SHIELD / 4 );
                } else {
                    OBJS_Add( cur->type );
                }

                if ( cur->lib->moneyflag ) {
                    cur->dflag = true;
                    cur->countdown = 50;
                } else {
                    cur = BONUS_Remove( cur );
                    continue;
                }
            }
        }

        if ( cur->dflag ) {
            cur->countdown--;
            if ( cur->countdown <= 0 ) {
                cur = BONUS_Remove( cur );
                continue;
            }
        }

        if ( cur->gy > 200 ) {
            cur = BONUS_Remove( cur );
        }
    }

    gcnt++;
}

/***************************************************************************
BONUS_Display () - Displays Active Bonuses in game
 ***************************************************************************/
void BONUS_Display( void ) {
    BONUS* cur;

    for ( cur = first_bonus.next; cur != &last_bonus; cur = cur->next ) {
        if ( !cur->dflag ) {
            GFX_PutSprite( GLB_GetItem( cur->item ), cur->bx, cur->by );
            GFX_ShadeShape( LIGHT, GLB_GetItem( glow[cur->curglow] ), cur->gx, cur->gy );
        } else {
            GFX_PutSprite( GLB_GetItem( N9_PIC + (DWORD) 1 ), cur->bx, cur->by );
        }
    }
}
