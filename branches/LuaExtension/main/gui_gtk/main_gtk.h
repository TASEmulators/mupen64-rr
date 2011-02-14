/***************************************************************************
                          main_gtk.h  -  description
                             -------------------
    begin                : Fri Nov 8 2002
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

#ifndef __MAIN_GTK_H__
#define __MAIN_GTK_H__

#include <gtk/gtk.h>

typedef struct
{
	GtkWidget	*window;
	GtkWidget	*toplevelVBox;	// vbox containing menubar, toolbar, rombrowser, statusbar

	GtkWidget	*menuBar;

	GtkWidget	*toolBar;

	GtkWidget	*romScrolledWindow, *romCList;

	GtkWidget	*statusBarHBox;
} SMainWindow;

extern SMainWindow g_MainWindow;
extern char g_WorkingDir[];

void statusbar_message( const char *section, const char *fmt, ... );
void open_rom( const char *filename );
void run_rom( void );

#endif // __MAIN_GTK_H__
