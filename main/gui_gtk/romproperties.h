/***************************************************************************
                          romproperties.h  -  description
                             -------------------
    begin                : Wed Nov 13 2002
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

#ifndef __ROMPROPERTIES_H__
#define __ROMPROPERTIES_H__

#include <gtk/gtk.h>

#include "rombrowser.h"

typedef struct
{
	GtkWidget *dialog;

	// entries
	GtkWidget *romNameEntry;
	GtkWidget *fileNameEntry;
	GtkWidget *sizeEntry;
	GtkWidget *countryEntry;
	GtkWidget *iniCodeEntry;
	GtkWidget *md5Entry;
	GtkWidget *commentsEntry;
} SRomPropertiesDialog;
extern SRomPropertiesDialog g_RomPropDialog;

int create_romPropDialog( void );
void show_romPropDialog( SRomEntry *entry );

#endif // __ROMPROPERTIES_H__
