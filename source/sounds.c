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

#if !PLATFORM_MACOSX
#include <malloc.h>
#endif

#if PLATFORM_DOS
#include <conio.h>
#include <dos.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "shadow.h" // added for unix port.  --ryan.
#undef MAXPLAYERS
#include "keys.h"

#include "names2.h"
#include "types.h"
#include "sndcards.h"
#include "fx_man.h"
#include "music.h"
#include "util_lib.h"
#include "gamedefs.h"
#include "config.h"


#include "panel.h"
#include "game.h"
#include "sounds.h"
#include "ai.h"
#include "net.h"

#include "def.h"
#include "cache.h"

extern USERp User[MAXSPRITES];
void DumpSounds(void);

// Parentally locked sounds list
int PLocked_Sounds[] = {
    483,328,334,335,338,478,450,454,452,453,456,457,458,455,460,462,
    461,464,466,465,467,463,342,371,254,347,350,432,488,489,490,76,339,
    499,500,506,479,480,481,482,78,600,467,548,547,544,546,545,542,541,540,
    539,536,529,525,522,521,515,516,612,611,589,625,570,569,567,565,
    558,557
};

// Global vars used by ambient sounds to set spritenum of ambient sounds for later lookups in
// the sprite array so FAFcansee can know the sound sprite's current sector location
BOOL Use_SoundSpriteNum = FALSE;
SHORT SoundSpriteNum = -1;  // Always set this back to -1 for proper validity checking!

BOOL MusicInitialized = FALSE;
BOOL FxInitialized = FALSE;

void SoundCallBack(unsigned long num);
char *LoadMidi(char *);

#define NUM_SAMPLES 10

char *BitNames[2] =
    {
    "8-bit", "16-bit"
    };

char *ChannelNames[2] =
    {
    "Mono", "Stereo"
    };

char *VoiceNames[8] =
    {
    "1", "2", "3", "4", "5", "6", "7", "8"
    };

char *SoundCardNames[NumSoundCards + 1] =
    {
    "None",
    "SoundBlaster",
    "Pro AudioSpectrum",
    "Sound Man 16",
    "Adlib",
    "General Midi",
    "Sound Canvas",
    "Awe 32",
    "WaveBlaster",
    "Ensoniq SoundScape",
    "Gravis UltraSound",
    "Disney Sound Source",
    "Tandy Sound Source",
    "PC speaker"
    };

int music;
int soundfx;
int num_voices;

int NumSounds = 0;

int angle;
int distance;
int voice;

int loopflag;

char *SongPtr = NULL;
char *SongName = NULL;
extern BOOL DemoMode;

//
// Includes digi.h to build the table
//

#define DIGI_TABLE
VOC_INFO voc[] =
    {
#include "digi.h"
    };

#undef  DIGI_TABLE

//
// Includes ambient.h to build the table of ambient sounds for game
//

#define AMBIENT_TABLE
AMB_INFO ambarray[] =
    {
#include "ambient.h"
    };
#undef  AMBIENT_TABLE
#define MAX_AMBIENT_SOUNDS 82

#define MAXSONGS        10              // This is the max songs per episode

BOOL OpenSound(VOC_INFOp vp, long *handle, long *length);
int ReadSound(long handle, VOC_INFOp vp, long length);

// 3d sound engine function prototype
VOC3D_INFOp Insert3DSound(void);

#if 0
// DEBUG
char *globsndata[DIGI_MAX], *globvpdata[DIGI_MAX];
long glength[DIGI_MAX];
#endif

/*
===================
=
= My stuff
=
===================
*/

int PlayerPainVocs[] = {
    DIGI_PLAYERPAIN1,
    DIGI_PLAYERPAIN2,
    DIGI_PLAYERPAIN3,
    DIGI_PLAYERPAIN4,
    DIGI_PLAYERPAIN5
};

// Don't have these sounds yet
int PlayerLowHealthPainVocs[] = {
    DIGI_HURTBAD1,
    DIGI_HURTBAD2,
    DIGI_HURTBAD3,
    DIGI_HURTBAD4,
    DIGI_HURTBAD5
};

int TauntAIVocs[] = {
    DIGI_TAUNTAI1,
    DIGI_TAUNTAI2,
    DIGI_TAUNTAI3,
    DIGI_TAUNTAI4,
    DIGI_TAUNTAI5,
    DIGI_TAUNTAI6,
    DIGI_TAUNTAI7,
    DIGI_TAUNTAI8,
    DIGI_TAUNTAI9,
    DIGI_TAUNTAI10,
    DIGI_COWABUNGA,
    DIGI_NOCHARADE,
    DIGI_TIMETODIE,
    DIGI_EATTHIS,
    DIGI_FIRECRACKERUPASS,
    DIGI_HOLYCOW,
    DIGI_HAHA2,
    DIGI_HOLYPEICESOFCOW,
    DIGI_HOLYSHIT,
    DIGI_HOLYPEICESOFSHIT,
    DIGI_PAYINGATTENTION,
    DIGI_EVERYBODYDEAD,
    DIGI_KUNGFU,
    DIGI_HOWYOULIKEMOVE,
    DIGI_HAHA3,
    DIGI_NOMESSWITHWANG,
    DIGI_RAWREVENGE,
    DIGI_YOULOOKSTUPID,
    DIGI_TINYDICK,
    DIGI_NOTOURNAMENT,
    DIGI_WHOWANTSWANG,
    DIGI_MOVELIKEYAK,
    DIGI_ALLINREFLEXES
};

int PlayerGetItemVocs[] = {
    DIGI_GOTITEM1,
    DIGI_HAHA1,
    DIGI_BANZAI,
    DIGI_COWABUNGA,
    DIGI_TIMETODIE
};

int PlayerYellVocs[] = {
    DIGI_PLAYERYELL1,
    DIGI_PLAYERYELL2,
    DIGI_PLAYERYELL3
};

extern char lumplockbyte[];

#if 0
// DEBUG
void CheckSndData( char *file, int line )
    {
    short i;

    //return;
    
    for (i = 0; i<DIGI_MAX ; i++)
        {
        if (!globsndata[i] && !globvpdata[i]) continue;
        
        if (memcmp(globsndata[i], globvpdata[i], glength[i]) != 0)
            {
            printf("%s %d\n",file,line);
            printf("CheckSndData: Data is not the same! num = %d",i);
            exit(0);
            }
        }    
    }    
#endif    

//
// Routine called when a sound is finished playing
//

// CTW MODIFICATION
//#if 0
#if 1
// CTW MODIFICATION END
void
// CTW MODIFICATION
// SoundCallBack(long num)
SoundCallBack(unsigned long num)
// CTW MODIFICATION END
    {
    VOC_INFOp vp = &voc[num];
    
    // RTS sounds are negative
    if (num < 0)
        {
        ASSERT(-num < 11);
        lumplockbyte[-num]--;
        return;
        }

    // Update counter
    //vp->playing--;
    vp->lock--;
    }
#endif    

// 
    
void 
ClearSoundLocks(void)
{
    long i;

    for(i = 0; i < SIZ(voc); i++)
        {
        if(voc[i].lock >= 200)
            voc[i].lock = 199;
        }    
            
    for(i = 0; i < 11; i++)
        {
        if(lumplockbyte[i] >= 200)
            lumplockbyte[i] = 199;
        }    
}

VOID
UnInitSound(VOID)
    {
    SoundShutdown();
    MusicShutdown();
    }

VOID
InitFX(VOID)
    {
    VOC_INFOp vp;
    short i;

    #if 0
    // DEBUG
    for (i=0;i<DIGI_MAX;i++)
        {
        globsndata[i] = globvpdata[i] = NULL;
        glength[i] = 0;
        }
    #endif    

    //ExternalSoundMod();

    // Select which cards to use
    SoundStartup();

    // Get the current volume of the music and sound fx
    gs.SoundVolume = FX_GetVolume();

    for (vp = voc; vp < &voc[SIZ(voc)]; vp++)
        {
        vp->playing = 0;
        }

    // Set up our fx callback so we can display the sounds that are playing
    FX_SetCallBack(SoundCallBack);
    }

VOID
InitMusic(VOID)
    {
    // Select which cards to use
    MusicStartup();
    //SendGeneralMidiSysX();
    }

VOID
ExternalSoundMod(VOID)
    {
    FILE *fin;
    VOC_INFOp vp;
    char name[40];
    char new_name[40];
    long pri;
    long pitch_lo, pitch_hi;
    long ret;

    fin = fopen("swextern.snd", "r");

    if (!fin)
        return;

    while (TRUE)
        {
        ret = fscanf(fin, "%s %s %d %d", name, new_name, &pitch_lo, &pitch_hi);

        if (ret == EOF)
            break;

        for (vp = voc; vp < &voc[SIZ(voc)]; vp++)
            {
            if (!vp->name)
                continue;

            if (stricmp(name, vp->name) == NULL)
                {
                // vp->priority = pri;
                strcpy(vp->name, new_name);
                vp->pitch_lo = pitch_lo;
                vp->pitch_hi = pitch_hi;
                }
            }
        }

    fclose(fin);
    }

extern short Level;

int
PlaySong(char *song_file_name)
    {
    if (!gs.MusicOn)
        return(MUSIC_Error);
    
    if (DemoMode)    
        return(MUSIC_Error);
    
    if (SongPtr)    
        StopSong();
    
    //DSPRINTF(ds,"Song %s",song_file_name);
    //MONO_PRINT(ds);
    
    SongPtr = LoadMidi(song_file_name);

    //DSPRINTF(ds,"Song Ptr %d",SongPtr);
    //MONO_PRINT(ds);
    
    if (!SongPtr)
        return(MUSIC_Error);

    // Set loop to continually play song
    loopflag = MUSIC_LoopSong;

    //DSPRINTF(ds,"Playing song");
    //MONO_PRINT(ds);

    return((int)MUSIC_PlaySong(SongPtr, loopflag));
    }

VOID
StopFX(VOID)
    {
    FX_StopAllSounds();
    }

int
StopSong(VOID)
    {
    int status;
    
    if (DemoMode)    
        return(MUSIC_Error);
    
    if (!SongPtr)    
        return(MUSIC_Error);

    // Stop the music
    status = MUSIC_StopSong();
    FreeMem(SongPtr);
    SongPtr = NULL;
    
    return(status);
    }

VOID
StopSound(VOID)
    {
    StopFX();
    StopSong();
    CDAudio_Stop(); 
    }

//
// Sound Distance Calculation
//

#define MAXLEVLDIST 19000   // The higher the number, the further away you can hear sound

short
SoundDist(long x, long y, long z, long basedist)
    {
    double tx, ty, tz;
    double sqrdist,retval;
    double decay,decayshift;
    extern short screenpeek;

    #define DECAY_CONST 4000

    
    tx = fabs(Player[screenpeek].posx - x);
    ty = fabs(Player[screenpeek].posy - y);
    tz = fabs((Player[screenpeek].posz - z) >> 4);

    // Use the Pythagreon Theorem to compute the magnitude of a 3D vector
    sqrdist = fabs(tx*tx + ty*ty + tz*tz);
    retval = sqrt(sqrdist);

    if(basedist < 0) // if basedist is negative
        {
        short i;

        decayshift=2;
        decay = labs(basedist) / DECAY_CONST;

        for (i=0;i<decay;i++)
            decayshift *= 2;
            
        if(labs(basedist/decayshift) >= retval)
            retval = 0;
        else
            retval *= decay;
        } else    
        {
        if(basedist > retval)
            retval = 0;
        else    
            retval -= basedist;
        }    
        
    retval = retval * 256 / MAXLEVLDIST;

    if (retval < 0) retval = 0;
    if (retval > 255) retval = 255;

    return(retval);
    }

//
// Angle calcuations - may need to be checked to make sure they are right
//

short
SoundAngle(long x, long y)
    {
    extern short screenpeek;

    short angle, delta_angle;

    angle = getangle(x - Player[screenpeek].posx, y - Player[screenpeek].posy);

    delta_angle = GetDeltaAngle(angle, Player[screenpeek].pang);

    // convert a delta_angle to a real angle if negative
    if (delta_angle < 0)
        delta_angle = NORM_ANGLE((1024 + delta_angle) + 1024);

    // convert 2048 degree angle to 32 degree angle
    return (delta_angle >> 6);
    }

// CTW MODIFICATION
//void
int
// CTW MODIFICATION END
_PlayerSound(char *file, int line, int num, long *x, long *y, long *z, Voc3D_Flags flags, PLAYERp pp)
//PlayerSound(int num, long *x, long *y, long *z, Voc3D_Flags flags, PLAYERp pp)
{
    int handle;
    VOC_INFOp vp;

    if (Prediction)
        return 0;

    if (pp < Player || pp >= Player + MAX_SW_PLAYERS)   
        {
        TerminateGame();
        printf("Player Sound invalid player: file %s, line %d\n",file,line);
        exit(0);
        }
        
    PRODUCTION_ASSERT(pp >= Player && pp < Player+MAX_SW_PLAYERS);
    PRODUCTION_ASSERT(num >= 0 && num < DIGI_MAX);
        
    if (TEST(pp->Flags, PF_DEAD)) return 0; // You're dead, no talking!

    // If this is a player voice and he's already yacking, forget it.
    vp = &voc[num];
    if (vp == NULL)
        {
        TerminateGame();
        printf("vp == NULL in PlayerSound, num = %d\n",num);
        exit(0);
        }

    // Not a player voice, bail.
    if(vp->priority != PRI_PLAYERVOICE && vp->priority != PRI_PLAYERDEATH) 
        return 0;

    // He wasn't talking, but he will be now.
    if(!pp->PlayerTalking)
    {
        pp->PlayerTalking = TRUE;
        pp->TalkVocnum = num;   // Set the voc number
        pp->TalkVocHandle = PlaySound(num, x, y, z, flags); // Play the sound
        if(pp->TalkVocHandle < 0) 
        {
            pp->PlayerTalking = FALSE;
            pp->TalkVocnum = -1;
            pp->TalkVocHandle = -1;
        }
    }

    return 0;
}

void LockSound(int num)
    {
    VOC_INFOp vp = &voc[num];
    // if data is not locked
    if (vp->lock <= CACHE_UNLOCK_MAX)
        {
        vp->lock = CACHE_LOCK_START;
        }
    else
        // if data is already locked
        {
        vp->lock++;
        if (vp->lock >= CACHE_LOCK_MAX || vp->lock == 0)
            {
            DumpSounds();
            TerminateGame();
            printf("lock > MAX, num = %d",num);
            exit(0);
            }
        //ASSERT(vp->lock < CACHE_LOCK_MAX);
        //ASSERT(vp->lock != 0);
        }
    }    

BOOL CacheSound(int num, int type)
    {
    VOC_INFOp vp = &voc[num];
    extern char cachedebug;
    
    PRODUCTION_ASSERT(num >= 0 && num < DIGI_MAX); 
    
    // if no data we need to cache it in
    if (!vp->data)
        {
        long handle;
        long length;

        if (!OpenSound(vp, &handle, &length))
            {
            sprintf(ds,"Could not open sound %s, num %d, priority %d\n",vp->name,num,vp->priority);
            CON_ConMessage(ds);
            return(FALSE);
            }

        if (vp != NULL)
            {
            //FILE *fp;

            if (cachedebug)
                printf("Sound %s, %d\n",vp->name,num);
              
            /*    
            if (type == CACHE_SOUND_PLAY)    
                // start it out locked at the min
                vp->lock = CACHE_LOCK_START;
            else    
            if (type == CACHE_SOUND_PRECACHE)    
                // start it out unlocked at the max    
            */
                vp->lock = CACHE_UNLOCK_MAX;
                
            allocache((long *)(&vp->data), length, &vp->lock);
            
            #if 0
            // DEBUG
            globsndata[num] = AllocMem(length);
            glength[num] = length;

            fp = fopen( vp->name, "rb" );
            if( fp != NULL ) 
                {
                fread(globsndata[num], length, 1, fp);
                ASSERT(globsndata[num] != NULL);
                fclose( fp );
                }
            #endif    
            ///////    
            
            ASSERT(vp->data);
            ReadSound(handle, vp, length);
    
            #if 0
            //DEBUG
            globvpdata[num] = vp->data;
            CheckSndData( __FILE__, __LINE__ );
            #endif
            }
        }    
        
    return(TRUE);
    }

////////////////////////////////////////////////////////////////////////////
// Play a sound
////////////////////////////////////////////////////////////////////////////

#define SOUND_UNIT  MAXLEVLDIST/255
// NOTE: If v3df_follow == 1, x,y,z are considered literal coordinates
int
PlaySound(int num, long *x, long *y, long *z, Voc3D_Flags flags)
    {
    VOC_INFOp vp;
    VOC3D_INFOp v3p;
    int pitch;
    short angle, sound_dist;
    long tx, ty, tz;
    BYTE priority;
    SPRITEp sp=NULL;

    // DEBUG
	//extern BOOL Pachinko_Win_Cheat;

    
    // Don't play game sounds when in menus
    //if (UsingMenus && (*x!=0 || *y!=0 || *z!=0)) return(-1);
    
    // Weed out parental lock sounds if PLock is active
    if (gs.ParentalLock || Global_PLock)
        {
        short i;
        
        for (i=0; i<sizeof(PLocked_Sounds); i++)
            {
            if (num == PLocked_Sounds[i])
                return(-1);
            }
        }
        
    if (Prediction)
        return(-1);
    
    if (!gs.FxOn)
        return(-1);
    
    PRODUCTION_ASSERT(num >= 0 && num < DIGI_MAX); 
    
    // Reset voice
    voice = -1;
    
    // This is used for updating looping sounds in Update3DSounds
    if (Use_SoundSpriteNum && SoundSpriteNum >= 0)
        {
        ASSERT(SoundSpriteNum >= 0 && SoundSpriteNum < MAXSPRITES);
        sp = &sprite[SoundSpriteNum];
        }

    if (gs.Ambient && TEST(flags,v3df_ambient) && !TEST(flags,v3df_nolookup))  // Look for invalid ambient numbers
    {
        if(num < 0 || num > MAX_AMBIENT_SOUNDS)
        {
            sprintf(ds,"Invalid or out of range ambient sound number %d\n",num);
            PutStringInfo(Player+screenpeek, ds);
            return(-1);
        }
    }


    // Call queue management to add sound to play list.
    // 3D sound manager will update playing sound 10x per second until
    // the sound ends, at which time it is removed from both the 3D
    // sound list as well as the actual cache.
    v3p = Insert3DSound();

    // If the ambient flag is set, do a name conversion to point to actual
    // digital sound entry.
    v3p->num = num;
    v3p->priority = 0;
    v3p->FX_Ok = FALSE; // Hasn't played yet

    if (gs.Ambient && TEST(flags,v3df_ambient) && !TEST(flags,v3df_nolookup))
        {
            v3p->maxtics = STD_RANDOM_RANGE(ambarray[num].maxtics);
            flags |= ambarray[num].ambient_flags;   // Add to flags if any
            num = ambarray[num].diginame;
        }

    PRODUCTION_ASSERT(num >= 0 && num < DIGI_MAX); 


    // Assign voc to voc pointer
    vp = &voc[num];
    if (UsingMenus && *x==0 && *y==0 && *z==0)  // Menus sound outdo everything
        priority = 100;
    else    
        priority = vp->priority;
    v3p->vp = vp;

    // Assign voc info to 3d struct for future reference
    v3p->x = x;
    v3p->y = y;
    v3p->z = z;
    v3p->fx = *x;
    v3p->fy = *y;
    v3p->fz = *z;
    v3p->flags = flags;

    if (flags & v3df_follow)
        {
        tx = *x;
        ty = *y;
        if (!z)
            tz = 0;                     // Some sound calls don't have a z
        // value
        else
            tz = *z;
        }
    else
        {
        // Don't use pointers to coordinate values.
        tx = v3p->fx;
        ty = v3p->fy;
        tz = v3p->fz;
        }

    // Special case stuff for sounds being played in a level
    if(*x==0 && *y==0 && *z==0)
        tx = ty = tz = 0;

    if ((vp->voc_flags & vf_loop) && Use_SoundSpriteNum && SoundSpriteNum >= 0 && sp)
        {
        tx=sp->x;
        ty=sp->y;
        tz=sp->z;
        //CON_Message("Using sp to set tx=%ld,ty=%ld,tz=%ld",tx,ty,tz);
        }

    // Calculate sound angle
    if (flags & v3df_dontpan)               // If true, don't do panning
        angle = 0;
    else
        angle = SoundAngle(tx, ty);

    // Calculate sound distance
    if(tx == 0 && ty == 0 && tz == 0)
        sound_dist = 255;  // Special case for menus sounds,etc.
    else
        sound_dist = SoundDist(tx, ty, tz, vp->voc_distance);

    v3p->doplr_delta = sound_dist;      // Save of distance for doppler
    // effect

//  //DSPRINTF(ds,"sound dist = %d\n",sound_dist);
//  MONO_PRINT(ds);

    // Can the ambient sound see the player?  If not, tone it down some.
    if ((vp->voc_flags & vf_loop) && Use_SoundSpriteNum && SoundSpriteNum >= 0)
        {
        PLAYERp pp = Player+screenpeek;

        //MONO_PRINT("PlaySound:Checking sound cansee");
        if (!FAFcansee(tx, ty, tz, sp->sectnum,pp->posx, pp->posy, pp->posz, pp->cursectnum))
            {
            //MONO_PRINT("PlaySound:Reducing sound distance");
            sound_dist += ((sound_dist/2)+(sound_dist/4));  // Play more quietly
            if(sound_dist > 255) sound_dist = 255;
            
            // Special Cases
            if(num == DIGI_WHIPME) sound_dist = 255;
            }
        }

    // Assign ambient priorities based on distance
    if (gs.Ambient && TEST(flags, v3df_ambient))
        {
        v3p->priority = v3p->vp->priority - (sound_dist / 26);
        priority = v3p->priority;
        }
    
    if (!CacheSound(num, CACHE_SOUND_PLAY))
        {
        v3p->flags = v3df_kill;
        v3p->handle = -1;
        v3p->dist = 0;
        v3p->deleted = TRUE;            // Sound init failed, remove it!
        return(-1);
        }

    LockSound(num);
        
    if (sound_dist < 5)
        angle = 0;

    // Check for pitch bending
    if (vp->pitch_lo > vp->pitch_hi)
        ASSERT(vp->pitch_lo <= vp->pitch_hi);

    if (vp->pitch_hi == vp->pitch_lo)
        pitch = vp->pitch_lo;
    else 
    if (vp->pitch_hi != vp->pitch_lo)
        pitch = vp->pitch_lo + (STD_RANDOM_RANGE(vp->pitch_hi - vp->pitch_lo));

    #if 0    
    // DEBUG
    if (Pachinko_Win_Cheat)
        {
        CheckSndData( __FILE__, __LINE__ );
        Pachinko_Win_Cheat = FALSE;
        CON_Message("S O U N D S   C H E C K E D");
        }
    #endif    

    // Request playback and play it as a looping sound if flag is set.    
    if (vp->voc_flags & vf_loop)    
        {    
        unsigned short start;    
        short loopvol=0;    

        start = *(unsigned short *) (vp->data + 0x14);    
        if((loopvol = 255-sound_dist) <= 0)    
            loopvol = 0;    

        if(sound_dist < 255 || (flags & v3df_init))
            {
            voice = FX_PlayLoopedVOC(vp->data, start, 65536,    
                pitch, loopvol, loopvol, loopvol, priority, num);    
            } 
        else
            voice = -1;        
    
        }    
    else    
    //if(!flags & v3df_init)  // If not initing sound, play it    
        if(tx==0 && ty==0 && tz==0) // It's a non-inlevel sound    
            {
            voice = FX_PlayVOC( vp->data, pitch, 255, 255, 255, priority, num);    
            }
        else // It's a 3d sound    
            {
            if (sound_dist < 255)
                {
                voice = FX_PlayVOC3D(vp->data, pitch, angle, sound_dist, priority, num);    
                }
            else
                voice = -1;    
            }
         
    // If sound played, update our counter
    if (voice > FX_Ok)
        {
        //vp->playing++;
        v3p->FX_Ok = TRUE;
        } 
    else
        {
        vp->lock--;
        }

    // Assign voc info to 3d struct for future reference
    v3p->handle = voice;                // Save the current voc handle in struct
    v3p->dist = sound_dist;
    v3p->tics = 0;                      // Reset tics
    if (flags & v3df_init)
        v3p->flags ^= v3df_init;        // Turn init off now

    return(voice);
    }

VOID PlaySoundRTS(long rts_num)
    {
    char *rtsptr;
    long voice=-1;
    
    if (RTS_NumSounds() <= 0 || !gs.FxOn)
        return;
        
    rtsptr = (char *)RTS_GetSound (rts_num - 1);
    
    ASSERT(rtsptr);
    
    if(*rtsptr == 'C')
        voice = FX_PlayVOC3D(rtsptr, 0, 0, 0, 255, -rts_num);
    else 
        voice = FX_PlayWAV3D(rtsptr, 0, 0, 0, 255, -rts_num);
    
    if (voice <= FX_Ok)    
        {
        lumplockbyte[rts_num]--;
        }
    }    
    
///////////////////////////////////////////////

BOOL
OpenSound(VOC_INFOp vp, long *handle, long *length)
    {
    *handle = kopen4load(vp->name, 0);

    if (*handle == -1)
        {
        return (FALSE);
        }

    *length = kfilelength(*handle);

    return (TRUE);
    }

    
int
ReadSound(long handle, VOC_INFOp vp, long length)
    {
    if (kread(handle, vp->data, length) != length)
        {
        TerminateGame();
        printf("Error reading file '%s'.\n", vp->name);
        exit(0);
        }

    kclose(handle);
    return(0);
    }

char *
LoadMidi(char *filename)
    {
    long handle;
    long size;
    char *MidiPtr;

    if ((handle = kopen4load(filename, 0)) == -1)
        {
        return(NULL);
        //TerminateGame();
        //printf("Cannot open '%s' for read.\n", filename);
        //exit(1);
        }

    size = kfilelength(handle);

    MidiPtr = (char *) AllocMem(size);
    if (MidiPtr == NULL)
        {
        return(NULL);
        //TerminateGame();
        //printf("Out of memory while reading '%s'.\n", filename);
        //exit(1);
        }

    if (kread(handle, MidiPtr, size) != size)
        {
        return(NULL);
        //TerminateGame();
        //printf("Unexpected end of file while reading '%s'.\n", filename);
        //exit(1);
        }

    kclose(handle);

    return (MidiPtr);
    }


void FlipStereo(void)
    {
    FX_SetReverseStereo(gs.FlipStereo);
    }    
    
void
SoundStartup(void)
    {
    int32 status;

    // if they chose None lets return
    if (FXDevice == NumSoundCards)
        {
        gs.FxOn = FALSE;
        return;
        }

    //gs.FxOn = TRUE;

    // Do special Sound Blaster, AWE32 stuff
    if (
        (FXDevice == SoundBlaster) ||
        (FXDevice == Awe32)
        )
        {
        int MaxVoices;
        int MaxBits;
        int MaxChannels;

        status = FX_SetupSoundBlaster
            (
            BlasterConfig, &MaxVoices, &MaxBits, &MaxChannels
            );
        }
    else
        {
        status = FX_Ok;
        }

    if (status == FX_Ok)
        {
        status = FX_Init(FXDevice, NumVoices, NumChannels, NumBits, MixRate);
        if (status == FX_Ok)
            {
            FxInitialized = TRUE;
            FX_SetVolume(FXVolume);
            
             if (gs.FlipStereo)
                FX_SetReverseStereo(!FX_GetReverseStereo());
            }
        }
    if (status != FX_Ok)
        {
        printf("%s\n",FX_ErrorString(FX_Error));
        }

    status = FX_SetCallBack(SoundCallBack);

    if (status != FX_Ok)
        {
        printf("%s\n",FX_ErrorString(FX_Error));
        }

    }

/*
===================
=
= SoundShutdown
=
===================
*/

void
SoundShutdown(void)
    {
    int32 status;

    // if they chose None lets return
    if (FXDevice == NumSoundCards)
        {
        return;
        }

    if (!FxInitialized)
        return;

    status = FX_Shutdown();
    if (status != FX_Ok)
        {
        printf("%s\n",FX_ErrorString(FX_Error));
        }
    }

#if 0    
void TestMusic( void )
   {
   int32 status;

   // if they chose None lets return
   if (MusicDevice == NumSoundCards)
      return;

   BlasterConfig.Midi = MidiPort;

   status = MUSIC_Init( MusicDevice, MidiPort );
   
   if ( status == MUSIC_Ok )
      {
      MUSIC_SetVolume( MusicVolume );
      //status = MUSIC_PlaySong( &theme, MUSIC_PlayOnce );
      status = PlaySong("theme.mid");

      if ( status == MUSIC_Ok )
         {
         char temp[200];
         
         sprintf(temp,"You should hear the %s\n"
                      "     Press any key to end testing","song");
                      
         MONO_PRINT(temp);
                      
         while (1)
            {
            if (KB_KeyWaiting() == true)
               {
               KB_Getch();
               break;
               }
            if (!MUSIC_SongPlaying())
               {
               break;
               }
            }
            
         //status = MUSIC_StopSong();
         status = StopSong();
         }
         
      status = MUSIC_Shutdown();
      }
   if ( status != MUSIC_Ok )
      {
      Error( MUSIC_ErrorString( MUSIC_ErrorCode ));
      }
   }
#endif   
    

/*
===================
=
= MusicStartup
=
===================
*/

void loadtmb(void)
{
    char tmb[8000];
    long fil, l;

    fil = kopen4load("swtimbr.tmb",0);
    if(fil == -1) 
        return;
        
    l = kfilelength(fil);
    kread(fil,(char *)tmb,l);
    MUSIC_RegisterTimbreBank(tmb);
    kclose(fil);
}

void MusicStartup( void )
   {
   int32 status;

   // if they chose None lets return
   if (MusicDevice == NumSoundCards)
        {
        return;
        }

   // satisfy AWE32 and WAVEBLASTER stuff
   BlasterConfig.Midi = MidiPort;

   // Do special Sound Blaster, AWE32 stuff
   if (
         ( FXDevice == SoundBlaster ) ||
         ( FXDevice == Awe32 )
      )
      {
      int MaxVoices;
      int MaxBits;
      int MaxChannels;

      FX_SetupSoundBlaster
                  (
                  BlasterConfig, (int *)&MaxVoices, (int *)&MaxBits, (int *)&MaxChannels
                  );
      }
   status = MUSIC_Init( MusicDevice, MidiPort );

   if ( status == MUSIC_Ok )
      {
      MusicInitialized = TRUE;
      MUSIC_SetVolume( MusicVolume );
      }
   else
      {
      printf("%s\n",MUSIC_ErrorString(MUSIC_ErrorCode));
      gs.MusicOn = FALSE;
      }
      
   if (MusicInitialized)   
       loadtmb();
   }

void COVER_SetReverb(long amt)
    {
    if (FXDevice == SoundScape)
        {
        #if 0
        long MaxReverbDelay = FX_GetMaxReverbDelay();
        //MaxReverbDelay = FX_GetReverbDelay();
        FX_SetReverbDelay(MaxReverbDelay);
        ASSERT(amt <= 255);
        FX_SetReverb(amt);
        #endif
        return;
        }
    
    FX_SetReverb(amt);
    }
   
/*
===================
=
= MusicShutdown
=
===================
*/

void
MusicShutdown(void)
    {
    int32 status;

    // if they chose None lets return
    if (MusicDevice == NumSoundCards)
        return;

    if (!MusicInitialized)
        return;

    StopSong();
    
    status = MUSIC_Shutdown();
    if (status != MUSIC_Ok)
        {
        printf("%s\n",MUSIC_ErrorString(MUSIC_ErrorCode));
        }
    }

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
//
//  3D sound engine
//  Sound management routines that keep a list of
//  all sounds being played in a level.
//  Doppler and Panning effects are achieved here.
//
///////////////////////////////////////////////
// Declare and initialize linked list of vocs.
VOC3D_INFOp voc3dstart = NULL;
VOC3D_INFOp voc3dend = NULL;

//////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////
// Initialize new vocs in the 3D sound queue
///////////////////////////////////////////////
VOC3D_INFOp
InitNew3DSound(VOC3D_INFOp v3p)
{
    v3p->handle = -1;                    // Initialize handle to new sound
    // value
    v3p->owner = -1;
    v3p->deleted = FALSE;                // Used for when sound gets deleted

    return(v3p);
}

///////////////////////////////////////////////
// Inserts new vocs in the 3D sound queue
///////////////////////////////////////////////
VOC3D_INFOp
Insert3DSound(void)
    {
    VOC3D_INFOp vp, old;

    // Allocate memory for new sound
    // You can allocate new sounds as long as memory holds out.
    // If you run out of memory for sounds, you got problems anyway.
    vp = (VOC3D_INFOp) AllocMem(sizeof(VOC3D_INFO));
    ASSERT(vp != NULL);
    memset(vp,0xCC,sizeof(VOC3D_INFO)); // Zero out the memory

    if (!voc3dend)                      // First item in list
        {
        vp->next = vp->prev = NULL;
        voc3dend = vp;
        voc3dstart = vp;
        InitNew3DSound(vp);
        return (vp);
        }

    old = voc3dend;                     // Put it on the end
    old->next = vp;
    vp->next = NULL;
    vp->prev = old;
    voc3dend = vp;

    InitNew3DSound(vp);
    return (vp);

    }

/////////////////////////////////////////////////////
// Deletes vocs in the 3D sound queue with no owners
/////////////////////////////////////////////////////
void
DeleteNoSoundOwner(short spritenum)
    {
    VOC3D_INFOp vp, dp;

    vp = voc3dstart;

    while (vp)
        {
        dp = NULL;
        if (vp->owner == spritenum && vp->owner >= 0 && (vp->vp->voc_flags & vf_loop))
            {
            //DSPRINTF(ds,"Deleting owner %d\n",vp->owner);
            //MONO_PRINT(ds);

            // Make sure to stop active
            // sounds
            if (FX_SoundActive(vp->handle))
                {
                FX_StopSound(vp->handle); 
                }

            #if 0    
            // Clean up the sound active counter for cacheing locks    
            if (vp->FX_Ok) // Only decrement if sound ever played
                vp->vp->playing--; // Decrement instance of sound playing          
            if (vp->vp->playing == 0 && vp->vp->lock > CACHE_UNLOCK_MAX)      
                vp->vp->lock = CACHE_UNLOCK_MAX;
            #endif    


            dp = vp;                    // Point to sound to be deleted

            if (vp->prev)
                {
                vp->prev->next = vp->next;
                }
            else
                {
                voc3dstart = vp->next;  // New first item
                if (voc3dstart)
                    voc3dstart->prev = NULL;
                }

            if (vp->next)
                {
                vp->next->prev = vp->prev;      // Middle element
                }
            else
                {
                voc3dend = vp->prev;    // Delete last element
                }
            }

        vp = vp->next;

        if (dp != NULL)
            FreeMem(dp);                // Return memory to heap
        }
    }
    
// This is called from KillSprite to kill a follow sound with no valid sprite owner
// Stops and active sound with the follow bit set, even play once sounds.
void DeleteNoFollowSoundOwner(short spritenum)
    {
    VOC3D_INFOp vp, dp;
    SPRITEp sp = &sprite[spritenum];

    vp = voc3dstart;

    while (vp)
        {
        dp = NULL;
        // If the follow flag is set, compare the x and y addresses.
        if ((vp->flags & v3df_follow) && vp->x == &sp->x && vp->y == &sp->y)
            {
            if (FX_SoundActive(vp->handle))
                {
                FX_StopSound(vp->handle);
                }
                
            #if 0    
            if (vp->FX_Ok) // Only decrement if sound ever played
                vp->vp->playing--; // Decrement instance of sound playing          
            if (vp->vp->playing == 0 && vp->vp->lock > CACHE_UNLOCK_MAX)      
                vp->vp->lock = CACHE_UNLOCK_MAX;
            #endif    

            dp = vp;                    // Point to sound to be deleted

            if (vp->prev)
                {
                vp->prev->next = vp->next;
                }
            else
                {
                voc3dstart = vp->next;  // New first item
                if (voc3dstart)
                    voc3dstart->prev = NULL;
                }

            if (vp->next)
                {
                vp->next->prev = vp->prev;      // Middle element
                }
            else
                {
                voc3dend = vp->prev;    // Delete last element
                }
            }

        vp = vp->next;

        if (dp != NULL)
            FreeMem(dp);                // Return memory to heap
        }
    }
    
///////////////////////////////////////////////
// Deletes vocs in the 3D sound queue
///////////////////////////////////////////////
void
Delete3DSounds(void)
    {
    VOC3D_INFOp vp, dp;
    PLAYERp pp;
    int cnt=0;

    
    vp = voc3dstart;

    while (vp)
        {
        dp = NULL;
        if (vp->deleted)
            {
            #if 0
            if (vp->FX_Ok) // Only decrement if sound ever played
                vp->vp->playing--; // Decrement instance of sound playing          
            if (vp->vp->playing == 0 && vp->vp->lock > CACHE_UNLOCK_MAX)      
                vp->vp->lock = CACHE_UNLOCK_MAX;
            #endif    

            //DSPRINTF(ds,"Delete3DSounds: deleting owner %d digi %d\n",vp->owner,vp->num);
            //MONO_PRINT(ds);
            // Reset Player talking flag if a voice was deleted
            //if(vp->num > DIGI_FIRSTPLAYERVOICE && vp->num < DIGI_LASTPLAYERVOICE) 
	    if (!vp->vp) printf("Delete3DSounds(): NULL vp->vp\n"); else	// JBF: added null check
            if(vp->vp->priority == PRI_PLAYERVOICE || vp->vp->priority == PRI_PLAYERDEATH)
            {
                SHORT pnum;

                TRAVERSE_CONNECT(pnum)
                {
                    pp = &Player[pnum];

                    if(vp->num == pp->TalkVocnum)
                    {
                        pp->PlayerTalking = FALSE;
                        pp->TalkVocnum = -1;
                        pp->TalkVocHandle = -1;  
                        //DSPRINTF(ds,"DELETED PLAYER VOICE VOC! NUM=%d\n",vp->num);
                        //MONO_PRINT(ds);
                    }
                }
            }

            dp = vp;                    // Point to sound to be deleted
            if (vp->prev)
                {
                vp->prev->next = vp->next;
            } else
                {
                voc3dstart = vp->next;  // New first item
                if (voc3dstart)
                    voc3dstart->prev = NULL;
                }

            if (vp->next)
                {
                vp->next->prev = vp->prev;      // Middle element
                }
            else
                {
                voc3dend = vp->prev;    // Delete last element
                }
            }

        vp = vp->next;

        if (dp != NULL)
            {
            FreeMem(dp);                // Return memory to heap
            }
        }
    }

////////////////////////////////////////////////////////////////////////////
// Play a sound
////////////////////////////////////////////////////////////////////////////

int 
RandomizeAmbientSpecials(int handle)
    {
    #define MAXRNDAMB 12
    int ambrand[] = {
        56,57,58,59,60,61,62,63,64,65,66,67
    };
    short i;

    // If ambient sound is found in the array, randomly pick a new sound    
    for (i=0;i<MAXRNDAMB;i++)
        {
        if(handle == ambrand[i])
            return(ambrand[STD_RANDOM_RANGE(MAXRNDAMB-1)]);
        }
        
    return(handle); // Give back the sound, no new one was found    
    }    

void
DoTimedSound(VOC3D_INFOp p)
    {
    p->tics += synctics;

    if (p->tics >= p->maxtics)
        {
        if (!FX_SoundActive(p->handle))
            {
            // Check for special case ambient sounds
            p->num = RandomizeAmbientSpecials(p->num);

            // Sound was bumped from active sounds list, try to play again.
            // Don't bother if voices are already maxed out.
            if (FX_SoundsPlaying() < NumVoices)
                {
                if (p->flags & v3df_follow)
                    {
                    PlaySound(p->num, p->x, p->y, p->z, p->flags);
                    p->deleted = TRUE;  // Mark old sound for deletion
                    }
                else
                    {
                    PlaySound(p->num, &p->fx, &p->fy, &p->fz, p->flags);
                    p->deleted = TRUE;  // Mark old sound for deletion
                    }
                }
            }                           // !FX_SoundActive

        p->tics = 0;
        //while (p->tics >= p->maxtics)  // Really stupid thing to do!
        //    {
        //    p->tics -= p->maxtics;
        //    }
        }
    }

void
StopAmbientSound(void)
    {
    VOC3D_INFOp p;
    extern BOOL InMenuLevel;

    if(InMenuLevel) return;
    
    p = voc3dstart;
    
    while (p)
        {
        // kill ambient sounds if Ambient is off
        if (TEST(p->flags,v3df_ambient))    
            SET(p->flags, v3df_kill);

        if (p->flags & v3df_kill)
            {
            if (FX_SoundActive(p->handle))
                FX_StopSound(p->handle);// Make sure to stop active sounds

            p->deleted = TRUE;
            }
            
        p = p->next;
        }
        
    Delete3DSounds();
    }    

void
StartAmbientSound(void)
    {
    VOC3D_INFOp p;
    short i,nexti;
    extern BOOL InMenuLevel;
    
    if(InMenuLevel) return; // Don't restart ambience if no level is active! Will crash game.
    
    TRAVERSE_SPRITE_STAT(headspritestat[STAT_AMBIENT], i, nexti)
        {
        SPRITEp sp = &sprite[i];

        PlaySound(sp->lotag, &sp->x, &sp->y, &sp->z, v3df_ambient | v3df_init 
                | v3df_doppler | v3df_follow);
        Set3DSoundOwner(i);  // Ambient sounds need this to get sectnum for later processing
        }
    } 
    
///////////////////////////////////////////////
// Main function to update 3D sound array
///////////////////////////////////////////////
typedef struct {
    VOC3D_INFOp p;
    short dist;
    BYTE priority;
} TVOC_INFO, *TVOC_INFOp;

void
DoUpdateSounds3D(void)
    {
    VOC3D_INFOp p;
    BOOL looping;
    int pitch = 0, pitchmax;
    long delta;
    short dist, angle;
    BOOL deletesound = FALSE;

STUBBED("DoUpdateSounds3D hits an infinite loop");
return;

    TVOC_INFO TmpVocArray[32]; 
    int i;
    static BOOL MoveSkip8 = 0;

    if (UsingMenus) return;

    // This function is already only call 10x per sec, this widdles it down even more!
    MoveSkip8 = (MoveSkip8 + 1) & 15;

    //CON_Message("Sounds Playing = %d",FX_SoundsPlaying());
    
    // Zero out the temporary array
    //memset(&TmpVocArray[0],0,sizeof(TmpVocArray));
    for (i=0;i<32;i++)
        {
        TmpVocArray[i].p = NULL;
        TmpVocArray[i].dist = 0;
        TmpVocArray[i].priority = 0;
        }

    p = voc3dstart;

    while (p)
        {
        ASSERT(p->num >= 0 && p->num < DIGI_MAX);
        
        looping = p->vp->voc_flags & vf_loop;

//      //DSPRINTF(ds,"sound %d FX_SoundActive = %d\n,",p->num,FX_SoundActive(p->handle));
//      MONO_PRINT(ds);

        // If sprite owner is dead, kill this sound as long as it isn't ambient
        if (looping && p->owner == -1 && !TEST(p->flags,v3df_ambient))
            {
            SET(p->flags, v3df_kill);
            }
        
        // Is the sound slated for death? Kill it, otherwise play it.
        if (p->flags & v3df_kill)
            {
            if (FX_SoundActive(p->handle))
                FX_StopSound(p->handle);// Make sure to stop active sounds

            //DSPRINTF(ds,"%d had v3df_kill.\n",p->num);
            //MONO_PRINT(ds);
            p->deleted = TRUE;
            }
        else
            {
            if (!FX_SoundActive(p->handle) && !looping)
                {
                if (p->flags & v3df_intermit)
                {
                    DoTimedSound(p);
                } else
                //if(p->owner == -1 && !TEST(p->flags,v3df_ambient))
                    {
                    //DSPRINTF(ds,"%d is now inactive.\n",p->num);
                    //MONO_PRINT(ds);
                    p->deleted = TRUE;
                    }
                }
            else 
            if (FX_SoundActive(p->handle))
                {
                if (p->flags & v3df_follow)
                    {
                    dist = SoundDist(*p->x, *p->y, *p->z, p->vp->voc_distance);
                    angle = SoundAngle(*p->x, *p->y);
                    }
                else
                    {
                    if (p->fx == 0 && p->fy == 0 && p->fz == 0)
                        dist = 0;
                    else    
                        dist = SoundDist(p->fx, p->fy, p->fz, p->vp->voc_distance);
                    angle = SoundAngle(p->fx, p->fy);
                    }

                    // Can the ambient sound see the player?  If not, tone it down some.
                    if ((p->vp->voc_flags & vf_loop) && p->owner != -1)
                        {
                        PLAYERp pp = Player+screenpeek;
                        SPRITEp sp = &sprite[p->owner];

                        //MONO_PRINT("Checking sound cansee");
                        if (!FAFcansee(sp->x, sp->y, sp->z, sp->sectnum,pp->posx, pp->posy, pp->posz, pp->cursectnum))
                            {
                            //MONO_PRINT("Reducing sound distance");
                            dist += ((dist/2)+(dist/4));  // Play more quietly
                            if(dist > 255) dist = 255;

                            // Special cases
                            if(p->num == 76 && TEST(p->flags,v3df_ambient))
                                {
                                dist = 255; // Cut off whipping sound, it's secret
                                }

                            }
                        }
                        
                if (dist >= 255 && p->vp->voc_distance == DIST_NORMAL)
                    {
                    FX_StopSound(p->handle);    // Make sure to stop active
                    // sounds
                    }
                else
                    {
                    // Handle Panning Left and Right
                    if (!(p->flags & v3df_dontpan))
                        FX_Pan3D(p->handle, angle, dist);
                    else
                        FX_Pan3D(p->handle, 0, dist);

                    // Handle Doppler Effects
                    #define DOPPLERMAX  400
                    if (!(p->flags & v3df_doppler) && FX_SoundActive(p->handle))
                        {
                        pitch -= (dist - p->doplr_delta);

                        if (p->vp->pitch_lo != 0 && p->vp->pitch_hi != 0)
                            {
                            if (abs(p->vp->pitch_lo) > abs(p->vp->pitch_hi))
                                pitchmax = abs(p->vp->pitch_lo);
                            else
                                pitchmax = abs(p->vp->pitch_hi);

                            }
                        else
                            pitchmax = DOPPLERMAX;

                        if (pitch > pitchmax)
                            pitch = pitchmax;
                        if (pitch < -pitchmax)
                            pitch = -pitchmax;

                        p->doplr_delta = dist;  // Save new distance to
                        // struct
                        FX_SetPitch(p->handle, pitch);
                        }
                    }
                }
            else if (!FX_SoundActive(p->handle) && looping)
                {
                if (p->flags & v3df_follow)
                    {
                    dist = SoundDist(*p->x, *p->y, *p->z, p->vp->voc_distance);
                    angle = SoundAngle(*p->x, *p->y);
                    }
                else
                    {
                    dist = SoundDist(p->fx, p->fy, p->fz, p->vp->voc_distance);
                    angle = SoundAngle(p->fx, p->fy);
                    }

                // Sound was bumped from active sounds list, try to play
                // again.
                // Don't bother if voices are already maxed out.
                // Sort looping vocs in order of priority and distance
                //if (FX_SoundsPlaying() < NumVoices && dist <= 255)
                if (dist <= 255)
                    {
                        for(i=0; i<NumVoices; i++)
                        {
                            if(p->priority >= TmpVocArray[i].priority)
                            {                   
                                if(!TmpVocArray[i].p || dist < TmpVocArray[i].dist)
                                {
                                    ASSERT(p->num >= 0 && p->num < DIGI_MAX);
                                    TmpVocArray[i].p = p;
                                    TmpVocArray[i].dist = dist;
                                    TmpVocArray[i].priority = p->priority;
                                    break;
                                }
                            }
                        }
                    }
                }                       // !FX_SoundActive
            }                           // if(p->flags & v3df_kill)

        p = p->next;
        }                               // while(p)

//  //DSPRINTF(ds,"Num vocs in list: %d, Sounds playing: %d\n",numelems,FX_SoundsPlaying());
//  MONO_PRINT(ds);

    // Process all the looping sounds that said they wanted to get back in
    // Only update these sounds 5x per second!  Woo hoo!, aren't we optimized now?
    //if(MoveSkip8==0)  
    //    {
        for(i=0; i<NumVoices; i++)
            {
            int handle;

            p = TmpVocArray[i].p;
    
            //if (FX_SoundsPlaying() >= NumVoices || !p) break;
            if (!p) break;

            ASSERT(p->num >= 0 && p->num < DIGI_MAX);

            if (p->flags & v3df_follow)
                {
                if(p->owner == -1)
                    {
                    int enumber;
                    enumber = p->num;
                    TerminateGame();
                    printf("Owner == -1 on looping sound with follow flag set!\n");
                    printf("p->num = %ld\n",enumber);
                    exit(0);
                    }

                Use_SoundSpriteNum = TRUE;
                SoundSpriteNum = p->owner;

                handle = PlaySound(p->num, p->x, p->y, p->z, p->flags);
                //if(handle >= 0 || TEST(p->flags,v3df_ambient)) // After a valid PlaySound, it's ok to use voc3dend
                    voc3dend->owner = p->owner; // Transfer the owner
                p->deleted = TRUE;

                Use_SoundSpriteNum = FALSE;
                SoundSpriteNum = -1;

                //MONO_PRINT("TmpVocArray playing a follow sound");
                } else
                {
                if(p->owner == -1)
                    {
                    int enumber;
                    enumber = p->num;
                    TerminateGame();
                    printf("Owner == -1 on looping sound, no follow flag.\n");
                    printf("p->num = %ld\n",enumber);
                    exit(0);
                    }

                Use_SoundSpriteNum = TRUE;
                SoundSpriteNum = p->owner;
                
                handle = PlaySound(p->num, &p->fx, &p->fy, &p->fz, p->flags);
                //if(handle >= 0 || TEST(p->flags,v3df_ambient)) 
                    voc3dend->owner = p->owner; // Transfer the owner
                p->deleted = TRUE;

                Use_SoundSpriteNum = FALSE;
                SoundSpriteNum = -1;

                //MONO_PRINT("TmpVocArray playing a non-follow sound");
                }
            }
    //    } // MoveSkip8

    // Clean out any deleted sounds now
    Delete3DSounds();
    }

//////////////////////////////////////////////////
// Terminate the sounds list
//////////////////////////////////////////////////
void
Terminate3DSounds(void)
    {
    VOC3D_INFOp vp;

    vp = voc3dstart;

    while (vp)
        {
        FX_StopSound(vp->handle);       // Make sure to stop active sounds
        vp->deleted = TRUE;
        vp = vp->next;
        }

    Delete3DSounds();                   // Now delete all remaining sounds
    ClearSoundLocks();
    }

void
DumpSounds(void)
    {
    VOC3D_INFOp vp;

    vp = voc3dstart;

    while (vp)
        {
        if (TEST(vp->flags,v3df_ambient))
            sprintf(ds,"vp->num=%d, vp->owner=%d, vp->vp->lock=%d",ambarray[vp->num].diginame,vp->owner,vp->vp->lock);
        else    
            sprintf(ds,"vp->num=%d, vp->owner=%d, vp->vp->lock=%d",vp->num,vp->owner,vp->vp->lock);
        DebugWriteString(ds);
        if (vp->owner >= 0)
            {
            SPRITEp sp = &sprite[vp->owner];
            sprintf(ds,"sp->picnum=%d, sp->hitag=%d, sp->lotag=%d, sp->owner=%d\n",sp->picnum,sp->hitag,sp->lotag,sp->owner); 
            DebugWriteString(ds);
            }
        vp = vp->next;
        }
    }

    
//////////////////////////////////////////////////
// Set owner to check when to kill looping sounds
// Must be called immediately after PlaySound call
// since this only assigns value to last sound
// on voc list
//////////////////////////////////////////////////
void
Set3DSoundOwner(short spritenum)
{
    VOC3D_INFOp p;

//  ASSERT(p->handle != -1); // Check for bogus sounds

    p = voc3dend;

	 // DDOI
	 if (p == NULL)
	 {
		 return;
	 }

    // Queue up sounds with ambient flag even if they didn't play right away!
    if(p->handle != -1 || TEST(p->flags,v3df_ambient))
    {
        p->owner = spritenum;
    } else
    {
        p->deleted = TRUE;
        p->flags = v3df_kill;
    }
}

//////////////////////////////////////////////////
// Play a sound using special sprite setup
//////////////////////////////////////////////////
void
PlaySpriteSound(short spritenum, int attrib_ndx, Voc3D_Flags flags)
    {
    SPRITEp sp = &sprite[spritenum];
    USERp u = User[spritenum];

    ASSERT(u);

//  //DSPRINTF(ds,"index = %d, digi num = %d\n",attrib_ndx,u->Attrib->Sounds[attrib_ndx]);
//  MONO_PRINT(ds);
    PlaySound(u->Attrib->Sounds[attrib_ndx], &sp->x, &sp->y, &sp->z, flags);
    }

