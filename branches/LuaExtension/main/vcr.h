#ifndef __VCR_H__
#define __VCR_H__

//#include "../config.h"

#ifdef VCR_SUPPORT

#ifndef __WIN32__
# include "winlnxdefs.h"
#else
# include <windows.h>
#endif

#include "plugin.h"

#define SUCCESS (0)
#define WRONG_FORMAT (1)
#define WRONG_VERSION (2)
#define FILE_NOT_FOUND (3)
#define NOT_FROM_THIS_MOVIE (4)
#define NOT_FROM_A_MOVIE (5)
#define INVALID_FRAME (6)
#define UNKNOWN_ERROR (7)

#define MOVIE_START_FROM_SNAPSHOT	(1<<0)
#define MOVIE_START_FROM_NOTHING	(1<<1)

#define CONTROLLER_X_PRESENT(x)	(1<<(x))
#define CONTROLLER_1_PRESENT	(1<<0)
#define CONTROLLER_2_PRESENT	(1<<1)
#define CONTROLLER_3_PRESENT	(1<<2)
#define CONTROLLER_4_PRESENT	(1<<3)
#define CONTROLLER_1_MEMPAK 	(1<<4)
#define CONTROLLER_2_MEMPAK 	(1<<5)
#define CONTROLLER_3_MEMPAK 	(1<<6)
#define CONTROLLER_4_MEMPAK 	(1<<7)
#define CONTROLLER_1_RUMBLE 	(1<<8)
#define CONTROLLER_2_RUMBLE 	(1<<9)
#define CONTROLLER_3_RUMBLE 	(1<<10)
#define CONTROLLER_4_RUMBLE 	(1<<11)

#define MOVIE_AUTHOR_DATA_SIZE (222)
#define MOVIE_DESCRIPTION_DATA_SIZE (256)
#define MOVIE_MAX_METADATA_SIZE (MOVIE_DESCRIPTION_DATA_SIZE > MOVIE_AUTHOR_DATA_SIZE ? MOVIE_DESCRIPTION_DATA_SIZE : MOVIE_AUTHOR_DATA_SIZE)

/*
	(0x0001)	Directional Right
	(0x0002)	Directional Left
	(0x0004)	Directional Down
	(0x0008)	Directional Up
	(0x0010)	Start
	(0x0020)	Z
	(0x0040)	B
	(0x0080)	A
	(0x0100)	C Right
	(0x0200)	C Left
	(0x0400)	C Down
	(0x0800)	C Up
	(0x1000)	R
	(0x2000)	L
	(0x00FF0000) Analog X
	(0xFF000000) Analog Y
*/

extern void VCR_getKeys( int Control, BUTTONS *Keys );
extern void VCR_updateScreen();
extern void VCR_aiDacrateChanged( int SystemType );
extern void VCR_aiLenChanged();

extern BOOL VCR_isActive();
extern BOOL VCR_isIdle(); // not the same as !isActive()
extern BOOL VCR_isPlaying();
extern BOOL VCR_isRecording();
extern BOOL VCR_isCapturing();
extern void VCR_invalidatedCaptureFrame();
extern BOOL VCR_getReadOnly();
extern void VCR_setReadOnly(BOOL val);
extern unsigned long VCR_getLengthVIs();
extern unsigned long VCR_getLengthSamples();
extern void VCR_setLengthVIs(unsigned long val);
extern void VCR_setLengthSamples(unsigned long val);
extern void VCR_updateFrameCounter();
extern void VCR_toggleReadOnly ();

extern void VCR_movieFreeze (char** buf, unsigned long* size);
extern int VCR_movieUnfreeze (const char* buf, unsigned long size);
extern void VCR_clearAllSaveData();

extern int VCR_startRecord( const char *filename, BOOL fromSnapshot, const char *authorUTF8, const char *descriptionUTF8 );
extern int VCR_stopRecord();
extern int VCR_startPlayback( const char *filename, const char *authorUTF8, const char *descriptionUTF8 );
extern int VCR_stopPlayback();
extern int VCR_startCapture( const char *recFilename, const char *aviFilename );
extern int VCR_stopCapture();

extern void VCR_coreStopped();

#pragma pack(push, 1)
//#pragms pack(1)
typedef struct 
{
	unsigned long	magic;		// M64\0x1a
	unsigned long	version;	// 3
	unsigned long	uid;		// used to match savestates to a particular movie

	unsigned long	length_vis; // number of "frames" in the movie
	unsigned long	rerecord_count;
	unsigned char   vis_per_second; // "frames" per second
	unsigned char   num_controllers;
	unsigned short  reserved1;
	unsigned long	length_samples;

	unsigned short	startFlags; // should equal 2 if the movie is from a clean start
	unsigned short  reserved2;
	unsigned long	controllerFlags;
	unsigned long	reservedFlags [8];

	char	oldAuthorInfo [48];
	char	oldDescription [80];
	char	romNom [32]; // internal rom name
	unsigned long	romCRC;
	unsigned short	romCountry;
	char	reservedBytes [56];
	char	videoPluginName [64];
	char	soundPluginName [64];
	char	inputPluginName [64];
	char	rspPluginName [64];
	char	authorInfo [MOVIE_AUTHOR_DATA_SIZE]; // utf8-encoded
	char	description [MOVIE_DESCRIPTION_DATA_SIZE]; // utf8-encoded
	

} SMovieHeader; // should be exactly 1024 bytes
#pragma pack(pop)
	

extern SMovieHeader VCR_getHeaderInfo(const char* filename);

#endif // VCR_SUPPORT

#endif // __VCR_H__
