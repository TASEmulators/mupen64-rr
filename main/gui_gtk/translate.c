/***************************************************************************
                          translate.c  -  description
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

#include "translate.h"

#include "main_gtk.h"

#include <gtk/gtk.h>

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// types
typedef struct
{
	char *orig;				// original, english string
	char *trans;			// translated string to find entry
} translation_t;

typedef struct
{
	char *name;
	GList *translations;
} language_t;

// globals
static GList      *g_LanguageGList = NULL;
static language_t *g_Language = NULL;

// static functions
static char *
trim( char *str )
{
	char *p;

	// leading whitespace
	p = str;
	while( *p == ' ' || *p == '\n' || *p == '\r' || *p == '\t' )
		p++;
	strcpy( str, p );

	// trailing whitespace
	p = str + strlen( str ) - 1;
	while( *p == ' ' || *p == '\n' || *p == '\r' || *p == '\t' )
		p--;
	p[1] = '\0';

	return str;
}

static language_t *
tr_load_language( const char *filename )
{
	FILE *f;
	language_t *lang;
	translation_t *trans;
	char line[2048];
	char langname[200];
	char *p, *p2;

	f = fopen( filename, "r" );
	if( !f )
		return NULL;

	lang = (language_t *)malloc( sizeof( language_t ) );
	memset( lang, 0, sizeof( language_t ) );

	while( !feof( f ) )
	{
		if( !fgets( line, 2048, f ) )
			continue;

		trim( line );

		if( line[0] == ';' || line[0] == '\0' )
			continue;

		if( line[0] == '[' && line[strlen(line)-1] == ']' )
		{
			strcpy( langname, line + 1 );
			langname[strlen(langname)-1] = '\0';
#ifdef _GTK2
		     {
			char *langname_utf8 = g_convert(langname,-1,"UTF-8","ISO-8859-1",NULL,NULL,NULL);
			strcpy(langname, langname_utf8);
			g_free(langname_utf8);
		     }
#endif
			lang->name = malloc( strlen( langname ) + 1 );
			strcpy( lang->name, langname );
			continue;
		}

		p = strchr( line, '=' );
		if( p )
		{
			// replace "\\n" by '\n'
			while( (p2 = strstr( line, "\\n" )) )
			{
				*p2 = '\n';	// replace '\\' by '\n'
				p2++;
				strcpy( p2, p2 + 1 );	// remove 'n'
			}

			p = strchr( line, '=' );	// line may have changed
			*p = '\0'; p++;
			trim( line );
			trim( p );

			if( strlen( line ) == 0 || strlen( p ) == 0 )
				continue;

			trans = (translation_t *)malloc( sizeof( translation_t ) );

			trans->orig = strdup( line );
#ifdef _GTK2
		        trans->trans = g_convert(p, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
#else
			trans->trans = strdup( p );
#endif
		   
			lang->translations = g_list_append( lang->translations, trans );
			continue;
		}
	}

	fclose( f );
	return lang;
}

// functions
void
tr_load_languages( void )
{
	int i, j;
	char langdir[PATH_MAX], filename[PATH_MAX];
	const char *p;
	DIR *dir;
	struct dirent *de;
	language_t *lang;
	translation_t *trans;

	// free list
	for( i = 0; i < g_list_length( g_LanguageGList ); i++ )
	{
		lang = (language_t *)g_list_nth_data( g_LanguageGList, i );
		for( j = 0; j < g_list_length( lang->translations ); j++ )
		{
			trans = g_list_nth_data( lang->translations, j );
			free( trans->orig );
			free( trans->trans );
			free( trans );
		}
		g_list_free( lang->translations );
		free( lang );
	}
	g_list_free( g_LanguageGList );
	g_LanguageGList = NULL;

	// list languages
	snprintf( langdir, PATH_MAX, "%slang/", g_WorkingDir );
	dir = opendir( langdir );
	if( !dir )
		return;

	while( (de = readdir( dir )) )
	{
		p = strrchr( de->d_name, '.' );
		if( !p )
			continue;
		if( strcmp( p, ".lng" ) )
			continue;

		snprintf( filename, PATH_MAX, "%s%s", langdir, de->d_name );
		lang = tr_load_language( filename );
		if( lang )
		{
			g_LanguageGList = g_list_append( g_LanguageGList, lang );
		}
	}

	closedir( dir );
}

GList *
tr_language_list( void )
{
	int i;
	language_t *lang;
	GList *list = NULL;

	for( i = 0; i < g_list_length( g_LanguageGList ); i++ )
	{
		lang = g_list_nth_data( g_LanguageGList, i );
		list = g_list_append( list, lang->name );
	}

	return list;
}

int
tr_set_language( const char *name )
{
	language_t *lang;
	int i;

	for( i = 0; i < g_list_length( g_LanguageGList ); i++ )
	{
		lang = (language_t *)g_list_nth_data( g_LanguageGList, i );
		if( !strcasecmp( name, lang->name ) )
		{
			g_Language = lang;
			return 0;
		}
	}

	g_Language = NULL;
	return -1;
}

const char *
tr( const char *text )
{
	int i;
	translation_t *trans;
	const char *ret = text;

	if( g_Language )
		for( i = 0; i < g_list_length( g_Language->translations ); i++ )
		{
			trans = g_list_nth_data( g_Language->translations, i );
			if( !strcasecmp( text, trans->orig ) )
			{
				ret = trans->trans;
				break;
			}
		}

	return ret;
}
