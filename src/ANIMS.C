

#include "public.h"

#include "inc/file0001.inc"

#include "tile.h"

#include "anims.h"

typedef struct ANIMLIB_S {
    DWORD item;
    int numframes;
    GFLAG groundflag;
    bool playerflag;
    bool transparent;
    ANIM_DIR adir;
    int xoff;
    int yoff;
} ANIMLIB;

typedef struct ANIMS_S {
    struct ANIMS_S* prev;
    struct ANIMS_S* next;
    DWORD item;
    int dx;
    int dy;
    int x;
    int y;
    int curframe;
    ANIMLIB* lib;
    GFLAG groundflag;
    SPRITE_SHIP* en;
    bool edone;
} ANIMS;

#define MAX_ANIMLIB 25
#define MAX_ANIMS   100

ANIMLIB animlib[MAX_ANIMLIB];
ANIMS anims[MAX_ANIMS];

ANIMS first_anims;
ANIMS last_anims;
ANIMS* free_anims;

PRIVATE int curlib = 0;
PRIVATE int adir[3] = { 0, -1, 1 };

/***************************************************************************
ANIMS_Clear () - Clears out All ANIM Objects
 ***************************************************************************/
void ANIMS_Clear( void ) {
    int i;

    first_anims.prev = NULL;
    first_anims.next = &last_anims;

    last_anims.prev = &first_anims;
    last_anims.next = NULL;

    free_anims = anims;

    memset( anims, 0, sizeof( anims ) );

    for ( i = 0; i < MAX_ANIMS - 1; i++ ) {
        anims[i].next = &anims[i + 1];
    }
}

/*-------------------------------------------------------------------------*
ANIMS_Get () - Gets A Free ANIM from Link List
 *-------------------------------------------------------------------------*/
PRIVATE ANIMS* ANIMS_Get( void ) {
    ANIMS* pNew;

    if ( !free_anims ) {
        return NULL;
    }

    pNew = free_anims;
    free_anims = free_anims->next;

    memset( pNew, 0, sizeof( ANIMS ) );

    pNew->next = &last_anims;
    pNew->prev = last_anims.prev;
    last_anims.prev = pNew;
    pNew->prev->next = pNew;

    return pNew;
}

/*-------------------------------------------------------------------------*
ANIMS_Remove () Removes ANIM from Link List
 *-------------------------------------------------------------------------*/
PRIVATE ANIMS* ANIMS_Remove( ANIMS* anim ) {
    ANIMS* next;

    next = anim->prev;

    anim->next->prev = anim->prev;
    anim->prev->next = anim->next;

    memset( anim, 0, sizeof( ANIMS ) );

    anim->next = free_anims;

    free_anims = anim;

    return next;
}

/***************************************************************************
ANIMS_Register () - Register a ANIM for USE with this stuff
 ***************************************************************************/
int ANIMS_Register(
    DWORD item, // INPUT : lumpnum of first frame
    int numframes, // INPUT : number of frames
    GFLAG groundflag,
    bool playerflag, // INPUT : follow player movements
    bool transparent, // INPUT : Transparent ( LIGHT )
    ANIM_DIR adir // INPUT : Anim Direction
) {
    int handle = curlib;
    ANIMLIB* cur;
    GFX_PIC* h;

    if ( curlib >= MAX_ANIMLIB ) {
        EXIT_Error( "ANIMS_Register() - Max LIBs" );
    }

    cur = &animlib[curlib];
    curlib++;

    cur->item = item;
    cur->numframes = numframes;
    cur->groundflag = groundflag;
    cur->playerflag = playerflag;
    cur->transparent = transparent;
    cur->adir = adir;

    h = (GFX_PIC*) GLB_LockItem( item );
    cur->xoff = h->width >> 1;
    cur->yoff = h->height >> 1;
    GLB_FreeItem( item );

    return handle;
}

/***************************************************************************
ANIMS_Init () Initializes ANIM Stuff
 ***************************************************************************/
void ANIMS_Init( void ) {
    ANIMS_Clear();

    memset( animlib, 0, sizeof( animlib ) );

    curlib = 0;

    // GROUND EXPLOSIONS
    ANIMS_Register( GEXPLO_BLK, 42, GROUND, false, false, A_NORM ); // 0
    ANIMS_Register( BOOM_PIC, 35, GROUND, false, false, A_NORM ); // 1
    ANIMS_Register( SPLAT_BLK, 7, GROUND, false, false, A_NORM ); // 3
    ANIMS_Register( BIGSPLAT_BLK, 10, GROUND, false, false, A_NORM ); // 4

    // AIR EXPLOSIONS
    ANIMS_Register( LGFLAK_BLK, 12, HIGH_AIR, false, false, A_NORM ); // 5
    ANIMS_Register( EXPLO2_BLK, 13, GROUND, false, false, A_NORM ); // 2
    ANIMS_Register( SMFLAK_BLK, 14, HIGH_AIR, false, false, A_NORM ); // 6
    ANIMS_Register( AIRBOOM_PIC, 16, HIGH_AIR, false, false, A_NORM ); // 7
    ANIMS_Register( NRGBANG_BLK, 12, HIGH_AIR, false, false, A_NORM ); // 8
    ANIMS_Register( LRBLST_BLK, 4, HIGH_AIR, false, false, A_NORM ); // 9

    // MISC ANIMS
    ANIMS_Register( SSMOKE_BLK, 9, MID_AIR, false, false, A_NORM ); // 10
    ANIMS_Register( SSMOKE_BLK + 4, 5, MID_AIR, false, true, A_MOVEDOWN ); // 11
    ANIMS_Register( SMOKTRAL_BLK, 4, MID_AIR, false, true, A_MOVEUP ); // 12

    ANIMS_Register( LGHTIN_BLK, 14, MID_AIR, false, false, A_NORM ); // 13
    ANIMS_Register( BSPARK_BLK, 9, MID_AIR, false, false, A_NORM ); // 14
    ANIMS_Register( OSPARK_BLK, 9, MID_AIR, false, false, A_NORM ); // 15
    ANIMS_Register( GUNSTR_BLK, 4, MID_AIR, true, false, A_NORM ); // 16

    // GROUND EXPLOSION OVERLAYS
    ANIMS_Register( FLARE_PIC, 26, GROUND, false, false, A_NORM ); // 17
    ANIMS_Register( SPARKLE_PIC, 17, GROUND, false, false, A_NORM ); // 18

    // ENERGY GRAB
    ANIMS_Register( LGHTIN_BLK, 14, HIGH_AIR, false, false, A_NORM ); // 19

    // SUPER SHIELD
    ANIMS_Register( SHIPGLOW_BLK, 4, HIGH_AIR, true, true, A_NORM ); // 20
}

/***************************************************************************
ANIMS_CachePics() - Cache registered anim pics
 ***************************************************************************/
void ANIMS_CachePics( void ) {
    int i;
    ANIMLIB* cur;
    DWORD frames;

    cur = animlib;

    for ( i = 0; i < curlib; i++, cur++ ) {
        for ( frames = 0; frames < cur->numframes; frames++ ) {
            GLB_CacheItem( cur->item + frames );
        }
    }
}

/***************************************************************************
ANIMS_FreePics() - Free Up Anims Used
 ***************************************************************************/
void ANIMS_FreePics( void ) {
    int i;
    ANIMLIB* cur;
    DWORD frames;

    cur = animlib;

    for ( i = 0; i < curlib; i++, cur++ ) {
        for ( frames = 0; frames < cur->numframes; frames++ ) {
            GLB_FreeItem( cur->item + frames );
        }
    }
}

/***************************************************************************
ANIMS_StartAnim () - Start An ANIM Playing
 ***************************************************************************/
void ANIMS_StartAnim(
    int handle, // INPUT : ANIM handle
    int x, // INPUT : x position
    int y // INPUT : y position
) {
    ANIMS* cur;
    ANIMLIB* lib = &animlib[handle];

    cur = ANIMS_Get();
    if ( !cur ) {
        return;
    }

    cur->lib = lib;
    cur->x = x - lib->xoff;
    cur->y = y - lib->yoff;
    cur->groundflag = lib->groundflag;
}

/***************************************************************************
ANIMS_StartGAnim () - Start An ANIM Playing with groundflag == GROUND
 ***************************************************************************/
void ANIMS_StartGAnim(
    int handle, // INPUT : ANIM handle
    int x, // INPUT : x position
    int y // INPUT : y position
) {
    ANIMS* cur;

    cur = ANIMS_Get();
    if ( !cur ) {
        return;
    }

    cur->lib = &animlib[handle];
    cur->x = x;
    cur->y = y;
    cur->groundflag = GROUND;
}

/***************************************************************************
ANIMS_StartEAnim () - Start An ANIM Playing locked onto ENEMY
 ***************************************************************************/
void ANIMS_StartEAnim(
    SPRITE_SHIP* en, // INPUT : pointer to ENEMY
    int handle, // INPUT : ANIM handle
    int x, // INPUT : x position
    int y // INPUT : y position
) {
    ANIMS* cur;
    ANIMLIB* lib = &animlib[handle];

    cur = ANIMS_Get();
    if ( !cur ) {
        return;
    }

    cur->en = en;
    cur->lib = &animlib[handle];
    cur->x = x - lib->xoff;
    cur->y = y - lib->yoff;
    cur->groundflag = HIGH_AIR;
}

/***************************************************************************
ANIMS_StartAAnim () - Start An ANIM Playing with groundflag == HIGH_AIR
 ***************************************************************************/
void ANIMS_StartAAnim(
    int handle, // INPUT : ANIM handle
    int x, // INPUT : x position
    int y // INPUT : y position
) {
    ANIMS* cur;
    ANIMLIB* lib = &animlib[handle];

    cur = ANIMS_Get();
    if ( !cur ) {
        return;
    }

    cur->lib = &animlib[handle];
    cur->x = x - lib->xoff;
    cur->y = y - lib->yoff;
    cur->groundflag = HIGH_AIR;
}

/***************************************************************************
ANIMS_Think () - Does all thinking for ANIMS
 ***************************************************************************/
void ANIMS_Think( void ) {
    ANIMS* cur;
    ANIMLIB* lib;

    for ( cur = first_anims.next; cur != &last_anims; cur = cur->next ) {
        lib = cur->lib;

        if ( cur->curframe >= lib->numframes ) {
            cur = ANIMS_Remove( cur );
            continue;
        }

        cur->item = lib->item + cur->curframe;

        if ( lib->playerflag ) {
            cur->dx = player_cx + cur->x;
            cur->dy = player_cy + cur->y;
        } else if ( cur->en ) {
            if ( cur->en->item == ~0 ) {
                cur->edone = true;
            }

            if ( !cur->edone ) {
                cur->dx = cur->en->move.x + cur->x;
                cur->dy = cur->en->move.y + cur->y;
            }
        } else {
            cur->dx = cur->x;
            cur->dy = cur->y;
        }

        switch ( lib->adir ) {
            case A_NORM:
                break;

            case A_MOVEDOWN:
                cur->y++;
                break;

            case A_MOVEUP:
                cur->y--;
                break;
        }

        cur->y += adir[lib->adir];

        if ( lib->groundflag == GROUND && scroll_flag ) {
            cur->y++;
        }

        cur->curframe++;
    }
}

/***************************************************************************
ANIMS_DisplayGround () - Displays All Active ANIMS on the Ground
 ***************************************************************************/
void ANIMS_DisplayGround( void ) {
    ANIMS* cur;
    BYTE* pic;

    for ( cur = first_anims.next; cur != &last_anims; cur = cur->next ) {
        if ( cur->groundflag != GROUND ) {
            continue;
        }

        pic = GLB_GetItem( cur->item );

        if ( cur->lib->transparent ) {
            GFX_ShadeShape( LIGHT, pic, cur->dx, cur->dy );
        } else {
            GFX_PutSprite( pic, cur->dx, cur->dy );
        }
    }
}

/***************************************************************************
ANIMS_DisplaySky () - Displays All Active ANIMS in SKY
 ***************************************************************************/
void ANIMS_DisplaySky( void ) {
    ANIMS* cur;
    BYTE* pic;

    for ( cur = first_anims.next; cur != &last_anims; cur = cur->next ) {
        if ( cur->groundflag != MID_AIR ) {
            continue;
        }

        pic = GLB_GetItem( cur->item );

        if ( cur->lib->transparent ) {
            GFX_ShadeShape( LIGHT, pic, cur->dx, cur->dy );
        } else {
            GFX_PutSprite( pic, cur->dx, cur->dy );
        }
    }
}

/***************************************************************************
ANIMS_DisplayHigh () - Displays All Active ANIMS in ABOVE PLAYER
 ***************************************************************************/
void ANIMS_DisplayHigh( void ) {
    ANIMS* cur;
    BYTE* pic;

    for ( cur = first_anims.next; cur != &last_anims; cur = cur->next ) {
        if ( cur->groundflag != HIGH_AIR ) {
            continue;
        }

        pic = GLB_GetItem( cur->item );

        if ( cur->lib->transparent ) {
            GFX_ShadeShape( LIGHT, pic, cur->dx, cur->dy );
        } else {
            GFX_PutSprite( pic, cur->dx, cur->dy );
        }
    }
}
