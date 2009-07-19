/**************************************************************************
   DebugView
   DebugView.h

   Copyright 2002-2003 Linker.  All Rights Reserved.
**************************************************************************/

/**************************************************************************
   Function Prototypes
**************************************************************************/
// DLL must be loaded
/*void FileLog(char *fmt, ...);
void OpenDV( HINSTANCE hInst );
void DVMsg( int icon, char *Message, ...);
void CloseDV( void );*/

void (__cdecl* FileLog)( char *fmt, ... );
HWND (__cdecl* OpenDV)( HINSTANCE hInst, int mode );
void (__cdecl* DVMsg)( int icon, char *Message, ... );
void (__cdecl* CloseDV)( void );
void (__cdecl* ShowDV)( int mode );
void (__cdecl* SetUserIcon)( int iconnum, HICON hIcon );
void (__cdecl* SetUserIconName)( int iconnum, char *messagetype );
void (__cdecl* DVClear)( void );

/**************************************************************************
   Icon types
**************************************************************************/
#define DV_NOICON		-1
#define DV_INFO			0
#define	DV_WARNING		1
#define DV_ERROR		2
#define DV_QUESTION		3

#define DV_USERICON1	10
#define DV_USERICON2	11
#define DV_USERICON3	12
#define DV_USERICON4	13
#define DV_USERICON5	14
#define DV_USERICON6	15
#define DV_USERICON7	16
#define DV_USERICON8	17
#define DV_USERICON9	18
#define DV_USERICON10	19

/**************************************************************************
   Window Mode
**************************************************************************/
#define DV_HIDE			0
#define	DV_SHOW 		1
#define DV_AUTO 		2
