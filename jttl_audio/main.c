/***************************************************************************
                          main.c  -  SDL Audio plugin for mupen64
                             -------------------
    begin                : Fri Oct 3 2003
    copyright            : (C) 2003 by Juha Luotio aka JttL
    email                : juha.luotio@porofarmi.net
    version              : 1.2
 ***************************************************************************/
/***************************************************************************
     This plug-in is originally based on Hactarux's "mupen audio plugin"
     and was modified by JttL. Actually there is no much original code
     left, but I think it's good to mention it anyway.
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 CHANGELOG:
 1.4:
 +Use only standard frequency for higher compatibility
 +Fast resample algorithm (use only integers)
 +Slight improvements in buffer management : pausing audio when buffer underrun occur
 
 1.2:
 +Added possibility to swap channels
 +Some more optimizations
 +Calling RomOpen() is not required anymore. Plugin should now follow Zilmar's specs.
 +Added test functions.
 +Added support for config file

 1.1.1:
 +Fixed the bug that was causing terrible noise (thanks Law)
 +Much more debugging data appears now if DEBUG is defined
 +Few more error checks

 1.1:
 +Audio device is opened now with native byte ordering of the machine. Just
  for compatibility (thanks Flea).
 +Fixed possible double freeing bug (thanks Flea)
 +Optimizations in AiLenChanged
 +Fixed segmentation fault when changing rom.
 +Syncronization redone

 1.0.1.3:
 +Smarter versioning. No more betas.
 +More cleaning up done.
 +Buffer underrun and overflow messages appear now at stderr (if DEBUG is
  defined)
 +Many things are now precalculated (this should bring a small performance
  boost)
 +Buffer underrun bug fixed.
 +Segmentation fault when closing rom fixed (at least I think so)

 1.0 beta 2:
 +Makefile fixed to get rid of annoying warning messages
 +Cleaned up some old code
 +Default frequency set to 33600Hz (for Master Quest compatibility)
 +Better syncronization (needs some work still though)

 1.0 beta 1:
 +First public release


 ***************************************************************************/
/***************************************************************************
 TODO:
 +GUI for adjusting config file settings ;)

 ***************************************************************************/
/***************************************************************************
 KNOWN BUGS:

 ***************************************************************************/
#ifdef USE_GTK
#include <gtk/gtk.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_thread.h"
#include "../main/winlnxdefs.h"
#include "Audio_#1.2.h"

/* Current version number */
#define VERSION "1.3"

/* Size of primary buffer in bytes. This is the buffer where audio is loaded
   after it's extracted from n64's memory. */
#define PRIMARY_BUFFER_SIZE 65536

/* If buffer load goes under LOW_BUFFER_LOAD_LEVEL then game is speeded up to
   fill the buffer. If buffer load exeeds HIGH_BUFFER_LOAD_LEVEL then some
   extra slowdown is added to prevent buffer overflow (which is not supposed
   to happen in any circumstanses if syncronization is working but because
   computer's clock is such inaccurate (10ms) that might happen. I'm planning
   to add support for Real Time Clock for greater accuracy but we will see.

   The plugin tries to keep the buffer's load always between these values.
   So if you change only PRIMARY_BUFFER_SIZE, nothing changes. You have to
   adjust these values instead. You propably want to play with
   LOW_BUFFER_LOAD_LEVEL if you get dropouts. */
#define LOW_BUFFER_LOAD_LEVEL 16384
#define HIGH_BUFFER_LOAD_LEVEL 32768

/* Size of secondary buffer. This is actually SDL's hardware buffer. This is
   amount of samples, so final bufffer size is four times this. */
#define SECONDARY_BUFFER_SIZE 2048

/* This sets default frequency what is used if rom doesn't want to change it.
   Popably only game that needs this is Zelda: Ocarina Of Time Master Quest */
#define DEFAULT_FREQUENCY 33600

/* Name of config file */
#define CONFIG_FILE "jttl_audio.conf"


/*--------------- VARIABLE DEFINITIONS ----------------*/

/* Read header for type definition */
static AUDIO_INFO AudioInfo;
/* The hardware specifications we are using */
static SDL_AudioSpec *hardware_spec;
/* Pointer to the primary audio buffer */
static Uint8 *buffer = NULL;
/* Position in buffer array where next audio chunk should be placed */
static unsigned int buffer_pos = 0;
/* Audio frequency, this is usually obtained from the game, but for compatibility we set default value */
static int frequency = DEFAULT_FREQUENCY;
/* This is for syncronization, it's ticks saved just before AiLenChanged() returns. */
static Uint32 last_ticks = 0;
/* This is amount of ticks that are needed for previous audio chunk to be played */
static Uint32 expected_ticks = 0;
/* AI_LEN_REG at previous round */
static DWORD prev_len_reg = 0;
/* If this is true then left and right channels are swapped */
static BOOL SwapChannels = FALSE;
/* */
static Uint32 PrimaryBufferSize = PRIMARY_BUFFER_SIZE;
/* */
static Uint32 SecondaryBufferSize = SECONDARY_BUFFER_SIZE;
/* */
static Uint32 LowBufferLoadLevel = LOW_BUFFER_LOAD_LEVEL;
/* */
static Uint32 HighBufferLoadLevel = HIGH_BUFFER_LOAD_LEVEL;


static int realFreq;

/* ----------- FUNCTIONS ------------- */
/* This function closes the audio device and reinitializes it with requested frequency */
void InitializeAudio(int freq);
void ReadConfig();
void InitializeSDL();

EXPORT void CALL
AiDacrateChanged( int SystemType )
{
        int f = frequency;
        switch (SystemType)
        {
        case SYSTEM_NTSC:
                f = 48681812 / (*AudioInfo.AI_DACRATE_REG + 1);
                break;
        case SYSTEM_PAL:
                f = 49656530 / (*AudioInfo.AI_DACRATE_REG + 1);
                break;
        case SYSTEM_MPAL:
                f = 48628316 / (*AudioInfo.AI_DACRATE_REG + 1);
                break;
        }
        InitializeAudio(f);
}


EXPORT void CALL
AiLenChanged( void )
{

        DWORD LenReg = *AudioInfo.AI_LEN_REG;
        Uint8 *p = (Uint8*)(AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_REG & 0xFFFFFF));
#ifdef DEBUG
        printf("(DD) New audio chunk, %i bytes\n", LenReg);
#endif
        if(buffer_pos + LenReg  < PrimaryBufferSize)
        {
                register unsigned int i;
	   
	        SDL_LockAudio();
                for ( i = 0 ; i < LenReg ; i += 4 )
                {

			if(SwapChannels == FALSE) {
				/* Left channel */
				buffer[ buffer_pos + i ] = p[ i + 2 ];
				buffer[ buffer_pos + i + 1 ] = p[ i + 3 ];

				/* Right channel */
				buffer[ buffer_pos + i + 2 ] = p[ i ];
				buffer[ buffer_pos + i + 3 ] = p[ i + 1 ];
			 } else {
				/* Left channel */
				buffer[ buffer_pos + i ] = p[ i ];
				buffer[ buffer_pos + i + 1 ] = p[ i + 1 ];

				/* Right channel */
				buffer[ buffer_pos + i + 2 ] = p[ i + 2];
				buffer[ buffer_pos + i + 3 ] = p[ i + 3 ];
			}
                }
	        buffer_pos += i;
	        SDL_UnlockAudio();
        }
#ifdef DEBUG
        else
        {
                fprintf(stderr, "(DD) Audio buffer overflow.\n");
        }
#endif

        /* Time that should be sleeped to keep game in sync */
        int wait_time = 0;

       /* And then syncronization */

       /* If buffer is running slow we speed up the game a bit. Actually we skip the syncronization. */
       if(buffer_pos < LowBufferLoadLevel)
       {
               //wait_time -= (LOW_BUFFER_LOAD_LEVEL - buffer_pos);
               wait_time = -1;
	       if(buffer_pos < SecondaryBufferSize*4*3) 
	         SDL_PauseAudio(1);
       }
       else SDL_PauseAudio(0);
       if(wait_time != -1) {
               /* If for some reason game is runnin extremely fast and there is risk buffer is going to
                 overflow, we slow down the game a bit to keep sound smooth. The overspeed is caused
                 by inaccuracy in machines clock. */
               if(buffer_pos > HighBufferLoadLevel)
               {
                       wait_time += (float)(buffer_pos - HIGH_BUFFER_LOAD_LEVEL) / (float)(frequency / 250);
               }
		expected_ticks = ((float)(prev_len_reg) / (float)(frequency / 250));

               if(last_ticks + expected_ticks > SDL_GetTicks()) {
                       wait_time += (last_ticks + expected_ticks) - SDL_GetTicks();
#ifdef DEBUG
                       printf("(DD) wait_time: %i, Buffer: %i/%i\n", wait_time, buffer_pos, PrimaryBufferSize);
#endif
                       SDL_Delay(wait_time);
               }
       
       }
       last_ticks = SDL_GetTicks();
       prev_len_reg = LenReg;

}

EXPORT DWORD CALL
AiReadLength( void )
{
   return 0;
}

EXPORT void CALL
CloseDLL( void )
{
}

EXPORT void CALL
DllAbout( HWND hParent )
{
#ifdef USE_GTK
   char tMsg[256];
   GtkWidget *dialog, *label, *okay_button;

   dialog = gtk_dialog_new();
   sprintf(tMsg,"Mupen64 SDL Sound Plugin %s by JttL", VERSION);
   label = gtk_label_new(tMsg);
   okay_button = gtk_button_new_with_label("OK");

   gtk_signal_connect_object(GTK_OBJECT(okay_button), "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     GTK_OBJECT(dialog));
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
		     okay_button);

   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
		     label);
   gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
   gtk_widget_show_all(dialog);
#else
   char tMsg[256];
   sprintf(tMsg,"Mupen64 SDL Sound Plugin %s by JttL", VERSION);
   fprintf(stderr, "About\n%s\n", tMsg);
#endif
}

EXPORT void CALL
DllConfig ( HWND hParent )
{
}

EXPORT void CALL
DllTest ( HWND hParent )
{
	/* Defining flags for tests */
	BOOL init_audio = FALSE;
	BOOL init_timer = FALSE;
	BOOL open_audio_device = FALSE;
	BOOL format_match = FALSE;
	BOOL freq_match = FALSE;


	printf("(II) Starting DLL Test.\n");
	SDL_PauseAudio(1);

	SDL_CloseAudio();
	if(SDL_WasInit(SDL_INIT_AUDIO) != 0) SDL_QuitSubSystem(SDL_INIT_AUDIO);
	if(SDL_WasInit(SDL_INIT_TIMER) != 0) SDL_QuitSubSystem(SDL_INIT_TIMER);
	if(SDL_Init(SDL_INIT_AUDIO) < 0 ) {
		printf("(EE) Couldn't initialize audio subsystem.\n");
		init_audio = FALSE;
	} else {
		printf("(II) Audio subsystem initialized.\n");
		init_audio = TRUE;
	}
	if(SDL_InitSubSystem(SDL_INIT_TIMER) < 0 ) {
		printf("(EE) Couldn't initialize timer subsystem.\n");
		init_timer = FALSE;

	} else {
		printf("(II) Timer subsystem initialized.\n");
		init_timer = TRUE;
	}

	SDL_PauseAudio(1);
        SDL_CloseAudio();

        /* Prototype of our callback function */
        void my_audio_callback(void *userdata, Uint8 *stream, int len);

        /* Open the audio device */
        SDL_AudioSpec *desired, *obtained;
        /* Allocate a desired SDL_AudioSpec */
        desired = malloc(sizeof(SDL_AudioSpec));
        /* Allocate space for the obtained SDL_AudioSpec */
        obtained = malloc(sizeof(SDL_AudioSpec));
        /* 22050Hz - FM Radio quality */
        desired->freq=DEFAULT_FREQUENCY;

        printf("(II) Requesting frequency: %iHz.\n", desired->freq);

        /* 16-bit signed audio */
        desired->format=AUDIO_S16SYS;

        printf("(II) Requesting format: %i.\n", desired->format);

        /* Stereo */
        desired->channels=2;
        /* Large audio buffer reduces risk of dropouts but increases response time */
        desired->samples=SecondaryBufferSize;

        /* Our callback function */
        desired->callback=my_audio_callback;
        desired->userdata=NULL;

        /* Open the audio device */
        if ( SDL_OpenAudio(desired, obtained) < 0 ){
                fprintf(stderr, "(EE) Couldn't open audio device: %s\n", SDL_GetError());
		open_audio_device = FALSE;
        } else {
		open_audio_device = TRUE;
	}
        /* desired spec is no longer needed */

        if(desired->format != obtained->format)
        {
                fprintf(stderr, "(EE) Obtained audio format differs from requested.\n");
		format_match = FALSE;
        } else {
		format_match = TRUE;
	}
        if(desired->freq != obtained->freq)
        {
                fprintf(stderr, "(EE) Obtained frequency differs from requested.\n");
		freq_match = FALSE;
        } else {
		freq_match = TRUE;
	}

        free(desired);
	free(obtained);

	SDL_PauseAudio(1);
	SDL_CloseAudio();
	if(SDL_WasInit(SDL_INIT_AUDIO) != 0) SDL_QuitSubSystem(SDL_INIT_AUDIO);
	if(SDL_WasInit(SDL_INIT_TIMER) != 0) SDL_QuitSubSystem(SDL_INIT_TIMER);
#ifdef USE_GTK
   char tMsg[256];
   GtkWidget *dialog, *label, *okay_button;

   dialog = gtk_dialog_new();
   if((init_audio == TRUE) && ( init_timer == TRUE ) && ( open_audio_device == TRUE ) && (format_match == TRUE) && (freq_match == TRUE)) {
 	 sprintf(tMsg,"Audio test successfull.");
   } else {
	sprintf(tMsg,"Audio test failed. See console for details.");
   }

   label = gtk_label_new(tMsg);
   okay_button = gtk_button_new_with_label("OK");

   gtk_signal_connect_object(GTK_OBJECT(okay_button), "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     GTK_OBJECT(dialog));
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
		     okay_button);

   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
		     label);
   gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
   gtk_widget_show_all(dialog);
#else
   if((init_audio == TRUE) && ( init_timer == TRUE ) && ( open_audio_device == TRUE ) && (format_match == TRUE) && (freq_match == TRUE)) {
  	printf("Audio test successfull.");
   } else {
	printf("Audio test failed. See console for details.");
   }
#endif
}

EXPORT void CALL
GetDllInfo( PLUGIN_INFO * PluginInfo )
{
   PluginInfo->Version = 0x0101;
   PluginInfo->Type    = PLUGIN_TYPE_AUDIO;
   sprintf(PluginInfo->Name,"JttL's SDL plugin %s", VERSION);
   PluginInfo->NormalMemory  = TRUE;
   PluginInfo->MemoryBswaped = TRUE;
}

EXPORT BOOL CALL
InitiateAudio( AUDIO_INFO Audio_Info )
{
   AudioInfo = Audio_Info;
   return TRUE;
}
#ifdef DEBUG
static int underrun_count = 0;
#endif

static void resample(Uint8 *src, int src_len, Uint8 *dest, int dest_len)
{
   int *psrc = (int*)src;
   int *pdest = (int*)dest;
   int i;
   int j=0;
   int const2 = 2*src_len/4;
   int const1 = const2 - 2*dest_len/4;
   int criteria = const2 - dest_len/4;
   
   for(i=0; i<dest_len/4; i++)
     {
	pdest[i] = psrc[j];
	if(criteria >= 0)
	  {
	     j++;
	     criteria += const1;
	  }
	else criteria += const2;
     }
}

void my_audio_callback(void *userdata, Uint8 *stream, int len)
{
        if(buffer_pos > (len * frequency) / realFreq)
        {
	        int rlen = ((len/4 * frequency) / realFreq)*4;
	        resample(buffer, rlen, stream, len);
	        //memcpy(stream, buffer, rlen);
                memmove(buffer, &buffer[ rlen ], buffer_pos  - rlen);

                buffer_pos = buffer_pos - rlen ;
        }
        else
        {
#ifdef DEBUG
                 underrun_count++;
                 fprintf(stderr, "(DD) Audio buffer underrun (%i).\n",underrun_count);
#endif
	         resample(buffer, buffer_pos, stream, ((buffer_pos/4 * realFreq) / frequency)*4);
	         memset(stream + ((buffer_pos/4 * realFreq) / frequency)*4, 0, len - ((buffer_pos/4 * realFreq) / frequency)*4);
                 //memcpy(stream, buffer, buffer_pos );
                 buffer_pos = 0;
        }
}
EXPORT void CALL RomOpen()
{
	/* This function is for compatibility with mupen. */
        //semaphore = SDL_CreateSemaphore(0);
	InitializeAudio( frequency );
}
void InitializeSDL()
{
	ReadConfig();
#ifdef DEBUG
        printf("(II) Sound plugin started in debug mode.\n");
#endif
        printf("(II) JttL's sound plugin version %s\n", VERSION);
        printf("(II) Initializing SDL audio subsystem...\n");
#ifdef DEBUG
        printf("(DD) Primary buffer: %i bytes.\n", PrimaryBufferSize);
        printf("(DD) Secondary buffer: %i bytes.\n", SecondaryBufferSize * 4);
        printf("(DD) Low buffer level: %i bytes.\n", LowBufferLoadLevel);
        printf("(DD) High buffer level: %i bytes.\n", HighBufferLoadLevel);
#endif
        if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
        {
                fprintf(stderr, "(EE) Failed to initialize SDL audio subsystem.\n(EE) Forcing exit.\n");
                exit(-1);
        }
        else
        {

                atexit(SDL_Quit);
        }

}
void InitializeAudio(int freq)
{


	if(SDL_WasInit(SDL_INIT_AUDIO|SDL_INIT_TIMER) == (SDL_INIT_AUDIO|SDL_INIT_TIMER) ) {
#ifdef DEBUG
		printf("(DD) Audio and timer allready initialized.\n");
#endif
	} else {
#ifdef DEBUG
		printf("(DD) Audio and timer not yet initialized. Initializing...\n");
#endif
		InitializeSDL();
	}

	frequency = freq; //This is important for the sync
        if(hardware_spec != NULL) free(hardware_spec);
        SDL_PauseAudio(1);
        SDL_CloseAudio();

        /* Prototype of our callback function */
        void my_audio_callback(void *userdata, Uint8 *stream, int len);

        /* Open the audio device */
        SDL_AudioSpec *desired, *obtained;
        /* Allocate a desired SDL_AudioSpec */
        desired = malloc(sizeof(SDL_AudioSpec));
        /* Allocate space for the obtained SDL_AudioSpec */
        obtained = malloc(sizeof(SDL_AudioSpec));
        /* 22050Hz - FM Radio quality */
        //desired->freq=freq;
        if(freq < 11025) realFreq = 11025;
        else if(freq < 22050) realFreq = 22050;
        else realFreq = 44100;
        desired->freq = realFreq;
#ifdef DEBUG
        printf("(DD) Requesting frequency: %iHz.\n", desired->freq);
#endif
        /* 16-bit signed audio */
        desired->format=AUDIO_S16SYS;
#ifdef DEBUG
        printf("(DD) Requesting format: %i.\n", desired->format);
#endif
        /* Stereo */
        desired->channels=2;
        /* Large audio buffer reduces risk of dropouts but increases response time */
        desired->samples=SecondaryBufferSize;

        /* Our callback function */
        desired->callback=my_audio_callback;
        desired->userdata=NULL;

        /* Open the audio device */
        if ( SDL_OpenAudio(desired, obtained) < 0 ){
                fprintf(stderr, "(EE) Couldn't open audio: %s\n", SDL_GetError());
                exit(-1);
        }
        /* desired spec is no longer needed */

        if(desired->format != obtained->format)
        {
                fprintf(stderr, "(EE) Obtained audio format differs from requested.\n");
        }
        if(desired->freq != obtained->freq)
        {
                fprintf(stderr, "(EE) Obtained frequency differs from requested.\n");
        }
        free(desired);
        hardware_spec=obtained;

#ifdef DEBUG
        printf("(DD) Frequency: %i\n", hardware_spec->freq);
        printf("(DD) Format: %i\n", hardware_spec->format);
        printf("(DD) Channels: %i\n", hardware_spec->channels);
        printf("(DD) Silence: %i\n", hardware_spec->silence);
        printf("(DD) Samples: %i\n", hardware_spec->samples);
        printf("(DD) Size: %i\n", hardware_spec->size);
#endif
        SDL_PauseAudio(0);

	if(buffer == NULL)
        {
                printf("(II) Allocating memory for audio buffer: %i bytes.\n", PrimaryBufferSize*sizeof(Uint8));
                buffer = (Uint8*)malloc(PrimaryBufferSize*sizeof(Uint8));
                SDL_PauseAudio(0);
        }
}
EXPORT void CALL
RomClosed( void )
{
   printf("(II) Cleaning up SDL sound plugin...\n");
   SDL_PauseAudio(1);
   if(buffer != NULL) free(buffer);
   if(hardware_spec != NULL) free(hardware_spec);
   hardware_spec = NULL;
   buffer = NULL;
   SDL_Quit();
}

EXPORT void CALL
ProcessAlist( void )
{
}

void ReadConfig() {
	FILE * config_file;
	char line[256];
	char param[128];
	char *value;
	if ((config_file = fopen(CONFIG_FILE, "r")) == NULL) {
        	fprintf(stderr, "(EE) Cannot open config file.\n");
		return;
	}


	while(!feof(config_file)) {
		fgets(line, 256, config_file);
		if((line[0] != '#') && (strlen(line) > 1)) {
			value = strchr(line, ' ');
			if (value[strlen(value)-1] == '\n') 
		                value[strlen(value)-1] = '\0';

			strncpy(param, line, (strlen(line) - strlen(value)));
			param[(strlen(line) - strlen(value))] = '\0';
#ifdef DEBUG
			printf("(DD) Parameter \"%s\", value: \"%i\"\n",&param,atoi(&value[1]));
#endif
			if(strcasecmp(param, "DEFAULT_FREQUENCY") == 0) frequency = atoi(value);
			if(strcasecmp(param, "SWAP_CHANNELS") == 0) SwapChannels = atoi(value);
			if(strcasecmp(param,"PRIMARY_BUFFER_SIZE") == 0) PrimaryBufferSize = atoi(value);
			if(strcasecmp(param,"SECONDARY_BUFFER_SIZE") == 0) SecondaryBufferSize = atoi(value);
			if(strcasecmp(param,"LOW_BUFFER_LOAD_LEVEL") == 0) LowBufferLoadLevel = atoi(value);
			if(strcasecmp(param,"HIGH_BUFFER_LOAD_LEVEL") == 0) HighBufferLoadLevel = atoi(value);


		}
	}

	fclose(config_file);

}
