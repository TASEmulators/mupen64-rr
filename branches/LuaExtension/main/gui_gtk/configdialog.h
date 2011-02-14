/***************************************************************************
                          configdialog.h  -  description
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

#ifndef __CONFIGDIALOG_H__
#define __CONFIGDIALOG_H__

#include <gtk/gtk.h>

typedef struct
{
	GtkWidget	*dialog;

	GtkWidget	*notebook;
	GtkWidget	*configMupen;
	GtkWidget	*configPlugins;
	GtkWidget	*configRomBrowser;

	GtkWidget	*coreInterpreterCheckButton;
	GtkWidget	*coreDynaRecCheckButton;
	GtkWidget	*corePureInterpCheckButton;

	GtkWidget	*gfxCombo;
	GtkWidget	*audioCombo;
	GtkWidget	*inputCombo;
	GtkWidget	*RSPCombo;
	GList		*gfxPluginGList;
	GList		*audioPluginGList;
	GList		*inputPluginGList;
	GList		*RSPPluginGList;

	GtkWidget	*romDirectoryList;
	GtkWidget	*romDirsScanRecCheckButton;
        GtkWidget       *noAudioDelayCheckButton;
        GtkWidget       *noCompiledJumpCheckButton;
        GtkWidget       *autoincSaveSlotCheckButton;
} SConfigDialog;
extern SConfigDialog g_ConfigDialog;

int create_configDialog( void );

#endif // __CONFIGDIALOG_H__
