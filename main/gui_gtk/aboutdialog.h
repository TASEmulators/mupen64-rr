/***************************************************************************
                          aboutdialog.h  -  description
                             -------------------
    begin                : Mon Nov 11 2002
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

#ifndef __ABOUTDIALOG_H__
#define __ABOUTDIALOG_H__

#include <gtk/gtk.h>

typedef struct
{
	GtkWidget *dialog;
} SAboutDialog;
extern SAboutDialog g_AboutDialog;

int create_aboutDialog( void );

#endif // __ABOUTDIALOG_H__
