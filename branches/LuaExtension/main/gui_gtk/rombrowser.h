/***************************************************************************
                          rombrowser.h  -  description
                             -------------------
    begin                : Sat Nov 9 2002
    copyright            : (C) 2002 by blight
    email                : blight@Ashitaka
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __ROMBROWSER_H__
#define __ROMBROWSER_H__

#include <gtk/gtk.h>

#include "../mupenIniApi.h"

int create_romBrowser( void );
void rombrowser_refresh( void );

// cache
void rombrowser_readCache( void );
void rombrowser_writeCache( void );

/** global rom list */
typedef struct
{
	char cFilename[PATH_MAX];

	char cName[100];
	char cSize[20];
	char cCountry[20];

	// rom info
	struct
	{
		char          cName[21];					// rom name
		int           iSize;							// size in bytes
		short         sCartID;						// cartridge id
		int           iManufacturer;			// manufacturer
		unsigned char cCountry;						// country id
		unsigned int  iCRC1;							// crc part 1
		unsigned int  iCRC2;							// crc part 2
		char          cMD5[33];						// md5 code
		char          cGoodName[100];			// from ini
		char          cComments[200];			// from ini

//		char     Status[60];				// from ini
//		char     FileName[200];
//		char     PluginNotes[250];	// from ini
//		char     CoreNotes[250];		// from ini
//		char     UserNotes[250];		// from ini
//		char     Developer[30];			// from ini
//		char     ReleaseDate[30];		// from ini
//		char     Genre[15];					// from ini
	} info;	// data saved in cache

	// other data
	GtkWidget  *flag;			// flag GtkPixmap
	mupenEntry *iniEntry;	// ini entry of this rom
} SRomEntry;
extern GList *g_RomList;

#endif // __ROMBROWSER_H__
