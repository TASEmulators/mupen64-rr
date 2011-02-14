/***************************************************************************
                          translate.h  -  description
                             -------------------
    begin                : Sun Nov 17 2002
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

#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__

#include <gtk/gtk.h>

void        tr_load_languages( void );						// load languages
GList      *tr_language_list( void );							// list of languages
int         tr_set_language( const char *name );	// set language to name
const char *tr( const char *text );								// translate text

#endif // __TRANSLATE_H__
