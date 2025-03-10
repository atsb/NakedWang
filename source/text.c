//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

Shadow Warrior is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1997 - Frank Maddin and Jim Norwood
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#include "condef.h"
#ifdef SW_3DFX
//#define __3DFX_ACC__
#endif
#undef MAIN
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "stdio.h"
#include "shadow.h" // added for unix port.  --ryan.
#include "keys.h"
#include "names2.h"
#include "panel.h"
#include "lists.h"
#include "game.h"
#include "mathutil.h"
#include "pal.h"
#include "text.h"
// CTW REMOVED
//#include "tensw.h"
// CTW REMOVED END

#include "def.h"
#include "net.h"

#define PANEL_FONT_G 3636
#define PANEL_FONT_Y 3646
#define PANEL_FONT_R 3656

#define PANEL_SM_FONT_G 3601
#define PANEL_SM_FONT_Y 3613
#define PANEL_SM_FONT_R 3625

CHARp KeyDoorMessage[] =
    {
    "You need a RED key for this door.",
    "You need a BLUE key for this door.",
    "You need a GREEN key for this door.",
    "You need a YELLOW key for this door.",
    "You need a GOLD key for this door.",
    "You need a SILVER key for this door.",
    "You need a BRONZE key for this door.",
    "You need a RED key for this door."
    };

VOID
DisplaySummaryString(PLAYERp pp, short xs, short ys, short color, short shade, CHARp buffer)
    {
    short size,x;
    CHARp ptr;
    PANEL_SPRITEp nsp;
    short font_pic;
    static short font_base[] = {PANEL_SM_FONT_G, PANEL_SM_FONT_Y, PANEL_SM_FONT_R};
    
    for (ptr = buffer, x = xs; *ptr; ptr++, x += size)
        {
        
        if (*ptr == ' ')
            {
            size = 4;
            continue;
            }
            
        switch (*ptr)
            {
            case '\\':
                *ptr = '0' - 1; // one pic before 0
                break;
            case ':':
                *ptr = '9' + 1; // one pic after nine
                break;
            }    
        
        ASSERT(color < 3);
        font_pic = font_base[color] + (*ptr - '0');
        nsp = pSpawnFullScreenSprite(pp, font_pic, PRI_FRONT_MAX, x, ys);
        nsp->shade = shade;
        size = tilesizx[font_pic] + 1;
        }
    }
    
PANEL_SPRITEp
pClearTextLineID(PLAYERp pp, short id, long y, short pri)
    {
    PANEL_SPRITEp nsp=NULL;
    PANEL_SPRITEp psp=NULL, next;

    TRAVERSE(&pp->PanelSpriteList, psp, next)
        {
        // early out
        if (psp->priority > pri)
            return (NULL);

        if (psp->ID == id && psp->y == y && psp->priority == pri)
            {
            SetRedrawScreen(psp->PlayerP);
            //SET(psp->flags, PANF_INVISIBLE);
            pSetSuicide(psp);
            }
        }

    return (NULL);
    }

// only call this from menu code - it does a pKillSprite
PANEL_SPRITEp
pMenuClearTextLineID(PLAYERp pp, short id, long y, short pri)
    {
    PANEL_SPRITEp nsp=NULL;
    PANEL_SPRITEp psp=NULL, next;

    TRAVERSE(&pp->PanelSpriteList, psp, next)
        {
        // early out
        if (psp->priority > pri)
            return (NULL);

        if (psp->ID == id && psp->y == y && psp->priority == pri)
            {
            SetRedrawScreen(psp->PlayerP);
            pKillSprite(psp);
            //pSetSuicide(psp);
            }
        }

    return (NULL);
    }


void
pClearTextLine(PLAYERp pp, long y)
    {
    SetRedrawScreen(pp);
    pClearTextLineID(pp, ID_TEXT, y, PRI_FRONT_MAX);
    }
    
void
StringTimer(PANEL_SPRITEp psp)
    {
    if ((psp->kill_tics -= synctics) <= 0)
        {
        SetRedrawScreen(psp->PlayerP);
        //pSetSuicide(psp);  // did not work here
        pKillSprite(psp);
        return;
        }
    }

void
PutStringTimer(PLAYERp pp, short x, short y, char *string, short seconds)
    {
    int ndx, offset;
    char c;
    PANEL_SPRITEp nsp;
    extern unsigned short xlatfont[];
    long kill_tics;
    short id, ac;
    void *func;
    

    offset = x;

    if (seconds == 999)
        {
        pClearTextLineID(pp, ID_TEXT, y, PRI_FRONT_MAX);
        func = NULL;
        kill_tics = 0;
        id = ID_TEXT;
        }
    else
        {
        pClearTextLineID(pp, ID_TEXT, y, PRI_FRONT_MAX);
        func = StringTimer;
        kill_tics = seconds * 120;
        id = ID_TEXT;
        }    

    for (ndx = 0; (c = string[ndx]) != 0; ndx++)
        {
        ac = c - '!' + STARTALPHANUM;
        if( (ac < STARTALPHANUM || ac > ENDALPHANUM)  && c != asc_Space )
            break;

        if (c > asc_Space && c < 127)
            {
            nsp = pSpawnFullViewSprite(pp, ac, PRI_FRONT_MAX, offset, y);
            nsp->PanelSpriteFunc = func;
            nsp->kill_tics = kill_tics;
            nsp->ID = id;
            offset += tilesizx[ac];
            }
        else if (c == asc_Space)
            offset += 4;                // Special case for space char
        }
    }

void
KillString(PLAYERp pp, short y)
    {
    pClearTextLineID(pp, ID_TEXT, y, PRI_FRONT_MAX);
    }

PANEL_SPRITEp
pClearSpriteXY(PLAYERp pp, short x, short y)
    {
    PANEL_SPRITEp nsp=NULL;
    PANEL_SPRITEp psp=NULL, next;

    TRAVERSE(&pp->PanelSpriteList, psp, next)
        {
        if (psp->x == x && psp->y == y)
            pSetSuicide(psp);
        }

    return (NULL);
    }

PANEL_SPRITEp
pClearSpriteID(PLAYERp pp, short id)
    {
    PANEL_SPRITEp nsp=NULL;
    PANEL_SPRITEp psp=NULL, next;

    TRAVERSE(&pp->PanelSpriteList, psp, next)
        {
        if (psp->ID == id)
            pSetSuicide(psp);
        }

    return (NULL);
    }
    

VOID
DisplayPanelNumber(PLAYERp pp, short xs, short ys, int number)
    {
    char buffer[32];
    char *ptr;
    short x, size;

    sprintf(buffer, "%03d", number);

    for (ptr = buffer, x = xs; *ptr; ptr++, x += size)
        {
        if (!isdigit(*ptr))
            {
            size = 0;
            continue;
            }

        pSpawnFullScreenSprite(pp, PANEL_FONT_G + (*ptr - '0'), PRI_FRONT_MAX, x, ys);

        size = tilesizx[PANEL_FONT_G + (*ptr - '0')] + 1;
        }
    }

VOID
DisplayMiniBarNumber(PLAYERp pp, short xs, short ys, int number)
    {
    char buffer[32];
    char *ptr;
    short x, size;
    short pic;

    sprintf(buffer, "%03d", number);

    for (ptr = buffer, x = xs; *ptr; ptr++, x += size)
        {
        if (!isdigit(*ptr))
            {
            size = 0;
            continue;
            }

        pic = PANEL_FONT_G + (*ptr - '0');
        
        rotatesprite((long)x << 16, (long)ys << 16, (1 << 16), 0,
            pic, 0, 0,
            ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
        
        size = tilesizx[PANEL_FONT_G + (*ptr - '0')] + 1;
        }
    }

VOID
DisplayMiniBarSmString(PLAYERp pp, short xs, short ys, short pal, CHARp buffer)
    {
    short size=4,x;
    CHARp ptr;
    PANEL_SPRITEp nsp;
    short pic;
    
    #define FRAG_FIRST_ASCII ('!') //exclamation point
    #define FRAG_FIRST_TILE 2930 //exclamation point
    
    for (ptr = buffer, x = xs; *ptr; ptr++, x += size)
        {
        if (*ptr == ' ')
            continue;
            
        ASSERT(*ptr >= '!' && *ptr <= '}');
        
        pic = FRAG_FIRST_TILE + (*ptr - FRAG_FIRST_ASCII);
        
        rotatesprite((long)x << 16, (long)ys << 16, (1 << 16), 0,
            pic, 0, pal,
            ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
        }
    }

VOID
DisplaySmString(PLAYERp pp, short xs, short ys, short pal, CHARp buffer)
    {
    short size=4,x;
    CHARp ptr;
    PANEL_SPRITEp nsp;
    // ID is base + (0-3)
    //short id = ID_TEXT + MOD4(pp->pnum);
    
    #define FRAG_FIRST_ASCII ('!') //exclamation point
    #define FRAG_FIRST_TILE 2930 //exclamation point
    
    for (ptr = buffer, x = xs; *ptr; ptr++, x += size)
        {
        if (*ptr == ' ')
            continue;
            
        ASSERT(*ptr >= '!' && *ptr <= '}');
        
        nsp = pSpawnFullScreenSprite(pp, FRAG_FIRST_TILE + (*ptr - FRAG_FIRST_ASCII), PRI_FRONT_MAX, x, ys);
        nsp->pal = pal;
        //nsp->ID = id;
        }
    }

#ifndef SW_3DFX
VOID
DisplayFragString(PLAYERp pp, short xs, short ys, CHARp buffer)
    {
    short size=4,x;
    CHARp ptr;
    PANEL_SPRITEp nsp;
    // ID is base + (0-3)
    short id = ID_TEXT + MOD4(pp->pnum);

    PLAYERp my_pp = Player + myconnectindex;
    
    #define FRAG_FIRST_ASCII ('!') //exclamation point
    #define FRAG_FIRST_TILE 2930 //exclamation point
    
    //pClearTextLineID(my_pp, id, ys, PRI_FRONT_MAX);
    
    for (ptr = buffer, x = xs; *ptr; ptr++, x += size)
        {
        if (*ptr == ' ')
            continue;
            
        ASSERT(*ptr >= '!' && *ptr <= '}');
        
        nsp = pSpawnFullScreenSprite(my_pp, FRAG_FIRST_TILE + (*ptr - FRAG_FIRST_ASCII), PRI_FRONT_MAX, x, ys);
        nsp->ID = id;
        //nsp->pal = PALETTE_PLAYER0 + pp->TeamColor;
        //if (pp->SpriteP)
        nsp->pal = User[pp->SpriteP - sprite]->spal;
        }
    }
    
VOID
DisplayFragNumbers(PLAYERp pp)
    {
    char buffer[32];
    char *ptr;
    short x, xs, ys, size;
    short frag_bar;
    short pnum = pp - Player;
    
    static int xoffs[] = 
        {
        69, 147, 225, 303
        };
    
    PLAYERp my_pp = Player + myconnectindex;

    // black tile to erase frag count
    #define FRAG_ERASE_NAME 2375
    #define FRAG_ERASE_NUMBER 2376
    #define FRAG_YOFF 2
    
        //xs = FRAG_XOFF;
        ys = FRAG_YOFF;
        
        // frag bar 0 or 1
        frag_bar = ((pnum)/4);
        // move y down according to frag bar number
        ys = ys + (tilesizy[FRAG_BAR]-2) * frag_bar;
        
        // move x over according to the number of players
        xs = xoffs[MOD4(pnum)];

        sprintf(buffer, "%03d", pp->Kills);
        
        // erase old kill count
        pSpawnFullScreenSprite(my_pp, FRAG_ERASE_NUMBER, PRI_MID+1, xs-1, ys);
        
        DisplayFragString(pp, xs, ys, buffer);
    }

VOID
DisplayFragNames(PLAYERp pp)
    {
    char *ptr;
    short x, xs, ys, size;
    short frag_bar;
    short pnum = pp - Player;
    
    static int xoffs[] = 
        {
        7, 85, 163, 241
        };

    PLAYERp my_pp = Player + myconnectindex;
        
        //xs = FRAG_XOFF;
        ys = FRAG_YOFF;
        
        // frag bar 0 or 1
        frag_bar = ((pnum)/4);
        // move y down according to frag bar number
        ys = ys + (tilesizy[FRAG_BAR]-2) * frag_bar;
        
        // move x over according to the number of players
        xs = xoffs[MOD4(pnum)];
        
        // erase old kill count
        pSpawnFullScreenSprite(my_pp, FRAG_ERASE_NAME, PRI_MID+1, xs-1, ys);

        DisplayFragString(pp, xs, ys, pp->PlayerName);
    }

#else
VOID
DrawFragString(PLAYERp pp, short xs, short ys, CHARp buffer)
    {
    short size=4,x;
    CHARp ptr;
    PANEL_SPRITEp nsp;

    #define FRAG_FIRST_ASCII ('!') //exclamation point
    #define FRAG_FIRST_TILE 2930 //exclamation point
    
    for (ptr = buffer, x = xs; *ptr; ptr++, x += size)
        {
        if (*ptr == ' ')
            continue;
            
        ASSERT(*ptr >= '!' && *ptr <= '}');
        
        //nsp = pSpawnFullScreenSprite(my_pp, FRAG_FIRST_TILE + (*ptr - FRAG_FIRST_ASCII), PRI_FRONT_MAX, x, ys);
        //nsp->ID = id;
        //nsp->pal = User[pp->SpriteP - sprite]->spal;
        rotatesprite((long)x << 16, (long)ys << 16, (1 << 16), 0,
            FRAG_FIRST_TILE + (*ptr - FRAG_FIRST_ASCII), 0, User[pp->SpriteP - sprite]->spal,
            ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
        }
    }

VOID
DrawFragNumbers(void)
    {
    PLAYERp pp;
    char buffer[32];
    char *ptr;
    short x, xs, ys, size;
    short frag_bar;
    short pnum;
    
    static xoffs[] = 
        {
        69, 147, 225, 303
        };
    
    // black tile to erase frag count
    #define FRAG_ERASE_NAME 2375
    #define FRAG_ERASE_NUMBER 2376
    #define FRAG_YOFF 2
    
    TRAVERSE_CONNECT(pnum)
        {    
        pp = Player + pnum;
    
        //xs = FRAG_XOFF;
        ys = FRAG_YOFF;
        
        // frag bar 0 or 1
        frag_bar = ((pnum)/4);
        // move y down according to frag bar number
        ys = ys + (tilesizy[FRAG_BAR]-2) * frag_bar;
        
        // move x over according to the number of players
        xs = xoffs[MOD4(pnum)];

        sprintf(buffer, "%03d", pp->Kills);
        
        // erase old kill count
        //pSpawnFullScreenSprite(my_pp, FRAG_ERASE_NUMBER, PRI_MID+1, xs-1, ys);
        
        DrawFragString(pp, xs, ys, buffer);
        }
    }
    
VOID DrawFragNames(void)
    {
    PLAYERp pp;
    char buffer[32];
    char *ptr;
    short x, xs, ys, size;
    short frag_bar;
    short pnum;
    
    static xoffs[] = 
        {
        7, 85, 163, 241
        };

    TRAVERSE_CONNECT(pnum)
        {    
        pp = Player + pnum;
        
        //xs = FRAG_XOFF;
        ys = FRAG_YOFF;
        
        // frag bar 0 or 1
        frag_bar = ((pnum)/4);
        // move y down according to frag bar number
        ys = ys + (tilesizy[FRAG_BAR]-2) * frag_bar;
        
        // move x over according to the number of players
        xs = xoffs[MOD4(pnum)];
        
        // erase old kill count
        //pSpawnFullScreenSprite(my_pp, FRAG_ERASE_NAME, PRI_MID+1, xs-1, ys);

        // TENSW use frag bar to display network debugging stats if
		// special mode is set; otherwise print as normal
		// CTW REMOVED
		//if (!gTenActivated || !tenPlayerDebugInfo(buffer, pnum))
		//	sprintf(buffer, "%s", pp->PlayerName);
		// CTW REMOVED END

        DrawFragString(pp, xs, ys, buffer);
        }
    }
    
VOID DrawFragBar(void)
    {
    short i, num_frag_bars;
    long y;
    extern SHORT OrigCommPlayers;
    PLAYERp pp = Player + myconnectindex;
    
    if (numplayers <= 1) 
        return;

    if (gNet.MultiGameType == MULTI_GAME_COOPERATIVE) 
        return;
    
    // if player sprite has not been initialized we have no business
    // sticking a frag bar up.  Prevents processing from MenuLevel etc.
    if (!pp->SpriteP)    
        return;
    
    //num_frag_bars = ((numplayers-1)/4)+1;
    num_frag_bars = ((OrigCommPlayers-1)/4)+1;
        
    for (i = 0, y = 0; i < num_frag_bars; i++)
        {
        //pSpawnFullScreenSprite(pp, FRAG_BAR, PRI_MID, 0, y);
        rotatesprite(0, (long)y << 16, (1 << 16), 0,
            FRAG_BAR, 0, 0,
            ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
        
        y += tilesizy[FRAG_BAR] - 2;
        }
        
    DrawFragNames();
    DrawFragNumbers();
    }
#endif    

short GlobInfoStringTime = TEXT_INFO_TIME;
VOID PutStringInfo(PLAYERp pp, char *string)
    {
    if (pp-Player != myconnectindex)
        return;
    
    if (!gs.Messages)
        return;
    
    CON_ConMessage(string); // Put it in the console too
    PutStringInfoLine(pp, string);    
    }
    
VOID PutStringInfoLine(PLAYERp pp, char *string)
    {
    short x,y;
    short w = 0;
    short h = 0;

    if (pp-Player != myconnectindex)
        return;
        
    MNU_MeasureString(string, &w, &h);
    
    x = TEXT_XCENTER(w);
    y = TEXT_INFO_LINE(0);
    
    // Move lower on this level because of boss meters
    //if ((Level == 20 && numplayers > 1) || numplayers > 4) 
    //    y += 20;
    //if (numplayers > 1 && numplayers <= 4) 
    //    y+= 10;
    
    PutStringTimer(pp, x, y, string, GlobInfoStringTime);
    // when printing info line clear the second line
    //PutStringInfoLine2(pp, "");
    }
    
VOID PutStringInfoLine2(PLAYERp pp, char *string)
    {
    short x,y;
    short w,h;

    if (pp-Player != myconnectindex)
        return;
    
    MNU_MeasureString(string, &w, &h);
    
    x = TEXT_XCENTER(w);
    y = TEXT_INFO_LINE(1);
    
    PutStringTimer(pp, x, y, string, GlobInfoStringTime);
    }

void
pMenuClearTextLine(PLAYERp pp)
    {
    pMenuClearTextLineID(pp, ID_TEXT, TEXT_INFO_LINE(0), PRI_FRONT_MAX);
    pMenuClearTextLineID(pp, ID_TEXT, TEXT_INFO_LINE(1), PRI_FRONT_MAX);
    }

#define TEXT_PLAYER_INFO_TIME (3)
#define TEXT_PLAYER_INFO_Y (200 - 40)

VOID PutStringPlayerInfo(PLAYERp pp, char *string)
    {
    short x,y;
    short w,h;
    
    if (pp-Player != myconnectindex)
        return;
        
    if (!gs.Messages)
        return;
        
    MNU_MeasureString(string, &w, &h);
    
    x = TEXT_XCENTER(w);
    y = TEXT_PLAYER_INFO_Y;
    
    PutStringTimer(pp, x, y, string, GlobInfoStringTime);
    }
