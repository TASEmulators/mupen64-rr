/***************************************************************************
                          vcrcomp_dialog.h  -  description
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

#include "../../config.h"

#ifdef VCR_SUPPORT

#ifndef __VCRCOMP_DIALOG_H__
#define __VCRCOMP_DIALOG_H__

#include <gtk/gtk.h>

typedef struct
{
	GtkWidget	*dialog;
	GtkWidget	*notebook;

	GtkWidget	*videoPage;
	GtkWidget	*videoCodecCombo;
	GList		*videoCodecGList;
	GtkWidget	*videoAttribCombo;
	GList		*videoAttribGList;
	GtkWidget	*videoAttribEntry;
	GtkWidget	*videoAttribChangeButton;

	GtkWidget	*audioPage;
	GtkWidget	*audioCodecCombo;
	GList		*audioCodecGList;
	GtkWidget	*audioAttribCombo;
	GList		*audioAttribGList;
	GtkWidget	*audioAttribEntry;
	GtkWidget	*audioAttribChangeButton;
} SVcrCompDialog;
extern SVcrCompDialog g_VcrCompDialog;

int create_vcrCompDialog( void );

#endif // __VCRCOMP_DIALOG_H__

#endif // VCR_SUPPORT
