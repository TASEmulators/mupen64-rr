/***************************************************************************
                          rombrowser.c  -  description
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

#include "rombrowser.h"
#include "romproperties.h"
#include "main_gtk.h"
#include "config.h"
#include "support.h"
#include "translate.h"

#include "../rom.h"
#include "../mupenIniApi.h"
#include "../../memory/memory.h"	// sl()

#include <gtk/gtk.h>
#include <zlib.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

// icons
#include "icons/australia.xpm"
#include "icons/europe.xpm"
#include "icons/france.xpm"
#include "icons/germany.xpm"
#include "icons/italy.xpm"
#include "icons/japan.xpm"
#include "icons/spain.xpm"
#include "icons/usa.xpm"
#include "icons/n64cart.xpm"

// rom file extensions
static const char *g_romFileExtensions[] = {
	".rom", ".v64", ".z64", ".gz", ".zip", ".n64", NULL
};

/*********************************************************************************************************
 * globals
 */
GList *g_RomList = NULL;
static GList *g_RomListCache = NULL; // roms in cache
static GtkWidget *australia, *europe, *france, *germany, *italy, *japan, *spain, *usa, *n64cart;
static GtkWidget *playRomItem;
static GtkWidget *romPropertiesItem;
static GtkWidget *refreshRomBrowserItem;
static int g_iNumRoms = 0;
static gint g_iSortColumn = 0; // sort column
static GtkSortType g_SortType = GTK_SORT_ASCENDING; // sort type (ascending/descending)

// function to fill a SRomEntry structure
static void
romentry_fill( SRomEntry *entry )
{
	// fill in defaults
	if( strlen( entry->cName ) == 0 )
	{
		if( strlen( entry->info.cGoodName ) == 0 )
			strcpy( entry->cName, entry->info.cName );
		else
			strcpy( entry->cName, entry->info.cGoodName );
	}

	entry->flag = n64cart;
	sprintf( entry->cSize, "%.1f MBits", (float)(entry->info.iSize / (float)0x20000) );
	switch( entry->info.cCountry )
	{
	case 0:		/* Demo */
		strcpy( entry->cCountry, tr("Demo") );
		break;

	case '7':
		strcpy( entry->cCountry, tr("Beta") );
		break;

	case 0x41:
		strcpy( entry->cCountry, tr("USA/Japan") );
		entry->flag = usa;	// FixMe: USA/Japan flag
		break;

	case 0x44:	/* Germany */
		strcpy( entry->cCountry, tr("Germany") );
		entry->flag = germany;
		break;

	case 0x45:	/* USA */
		strcpy( entry->cCountry, tr("USA") );
		entry->flag = usa;
		break;

	case 0x46:	/* France */
		strcpy( entry->cCountry, tr("France") );
		entry->flag = france;
		break;

	case 'I':	/* Italy */
		strcpy( entry->cCountry, tr("Italy") );
		entry->flag = italy;
		break;

	case 0x4A:	/* Japan */
		strcpy( entry->cCountry, tr("Japan") );
		entry->flag = japan;
		break;

	case 'S':	/* Spain */
		strcpy( entry->cCountry, tr("Spain") );
		entry->flag = spain;
		break;

	case 0x55: case 0x59:		/* Australia */
		sprintf( entry->cCountry, tr("Australia (0x%2.2X)"), entry->info.cCountry );
		entry->flag = australia;
		break;

	case 0x50: case 0x58: case 0x20:
	case 0x21: case 0x38: case 0x70:
		sprintf( entry->cCountry, tr("Europe (0x%02X)"), entry->info.cCountry );
		entry->flag = europe;
		break;

	default:
		sprintf( entry->cCountry, tr("Unknown (0x%02X)"), entry->info.cCountry );
		break;
	}
}

/*********************************************************************************************************
 * cache functions
 */
void
rombrowser_readCache( void )
{
	char filename[PATH_MAX];
	gzFile *f;
	int i;
	SRomEntry *entry;

	snprintf( filename, PATH_MAX, "%srombrowser.cache", g_WorkingDir );
	f = gzopen( filename, "rb" );
	if( !f )
		return;

	// free old list
	for( i = 0; i < g_list_length( g_RomListCache ); i++ )
	{
		entry = (SRomEntry *)g_list_nth_data( g_RomListCache, i );
		free( entry );
	}
	g_list_free( g_RomListCache );
	g_RomListCache = NULL;

	// number of entries
	gzread( f, &g_iNumRoms, sizeof( g_iNumRoms ) );

	// entries
	for( i = 0; i < g_iNumRoms; i++ )
	{
		entry = malloc( sizeof( SRomEntry ) );
		if( !entry )
		{
			printf( "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
			continue;
		}
		gzread( f, entry->cFilename, sizeof( char ) * PATH_MAX );
		gzread( f, &entry->info, sizeof( entry->info ) );

		if( access( entry->cFilename, F_OK ) < 0 )
			continue;

		romentry_fill( entry );

		// append to list
		g_RomListCache = g_list_append( g_RomListCache, entry );
	}

	gzclose( f );
}

void
rombrowser_writeCache( void )
{
	char filename[PATH_MAX];
	gzFile *f;
	int i;
	SRomEntry *entry;

	snprintf( filename, PATH_MAX, "%srombrowser.cache", g_WorkingDir );
	f = gzopen( filename, "wb" );
	if( !f )
		return;

	// number of entries
	gzwrite( f, &g_iNumRoms, sizeof( g_iNumRoms ) );

	// entries
	for( i = 0; i < g_iNumRoms; i++ )
	{
		entry = (SRomEntry *)g_list_nth_data( g_RomList, i );
		gzwrite( f, entry->cFilename, sizeof( char ) * PATH_MAX );
		gzwrite( f, &entry->info, sizeof( entry->info ) );
	}

	gzclose( f );

	// update cache list
	rombrowser_readCache();
}

/*********************************************************************************************************
 * rombrowser functions
 */
// compare functions
gint
rombrowser_compare( GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2 )
{
	int d, ret;
	SRomEntry *entry1, *entry2;
	GtkCListRow *row1 = (GtkCListRow *)ptr1;
	GtkCListRow *row2 = (GtkCListRow *)ptr2;

	entry1 = (SRomEntry *)row1->data;
	entry2 = (SRomEntry *)row2->data;

	switch( g_iSortColumn )
	{
	case 0:		// Name
		ret = strcasecmp( entry1->cName, entry2->cName );
		break;
	case 1:		// Country
		ret = strcasecmp( entry1->cCountry, entry2->cCountry );
		break;
	case 2:		// Size
		d = entry1->info.iSize - entry2->info.iSize;
		if( d < 0 )
			ret = 1;
		else if( d > 0 )
			ret = -1;
		else
			ret = 0;
		break;
	case 3:		// Comments
		return strcasecmp( entry1->info.cComments, entry2->info.cComments );
		break;
	case 4:		// File Name
		ret = strcmp( entry1->cFilename, entry2->cFilename );
		break;
	default:
		ret = 0;
	};

	if( g_SortType == GTK_SORT_ASCENDING )
		return ret;
	else
		return -ret;
}

// scan dir for roms
static void
scan_dir( const char *dirname )
{
	DIR *dir;
	int rom_size, i;
	char filename[PATH_MAX];
	char real_path[PATH_MAX];
	char crc_code[200];
	const char *p;
	struct dirent *de;
	struct stat sb;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
        gchar *line[5];
	SRomEntry *entry, *cacheEntry;
	int found;

	dir = opendir( dirname );
	if( !dir )
	{
		// error
		printf( "Couldn't open directory '%s': %s\n", dirname, strerror( errno ) );
		return;
	}

	while( (de = readdir( dir )) )
	{
		if( de->d_name[0] == '.' ) // .., . or hidden file
			continue;
		snprintf( filename, PATH_MAX, "%s%s", dirname, de->d_name );
	   
		if( config_get_bool( "RomDirsScanRecursive", FALSE ) )
		{
			// use real path (maybe it's a link)
			if( !realpath( filename, real_path ) )
				strcpy( real_path, filename );

			if( stat( real_path, &sb ) == -1 )
			{
				// Display error?
				continue;
			}

			if( S_ISDIR(sb.st_mode) )
			{
				strncat( filename, "/", PATH_MAX );
				scan_dir( filename );
				continue;
			}
		}
	   
		// check file extension
		p = strrchr( filename, '.' );
		if( !p )
			continue;
		for( i = 0; g_romFileExtensions[i]; i++ )
		{
			if( !strcasecmp( p, g_romFileExtensions[i] ) )
				break;
		}
		if( !g_romFileExtensions[i] )
			continue;
	   
		entry = malloc( sizeof( SRomEntry ) );
		if( !entry )
		{
			fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
			continue;
		}
	   
		memset( entry, 0, sizeof( SRomEntry ) );
		strcpy( entry->cFilename, filename );
	   
		// search cache
		found = 0;
		for( i = 0; i < g_list_length( g_RomListCache ); i++ )
		{
			cacheEntry = (SRomEntry *)g_list_nth_data( g_RomListCache, i );
			if( !strcmp( entry->cFilename, cacheEntry->cFilename ) )
			{
				memcpy( &entry->info, &cacheEntry->info, sizeof( cacheEntry->info ) );
				found = 1;
				break;
			}
		}
	   
		if( !found )
		{
			// load rom header
			rom_size = fill_header( entry->cFilename );
			if( !rom_size )
			{
				free( entry );
				continue;
			}

			// fill entry info struct
			entry->info.iSize = rom_size;
			entry->info.iManufacturer = ROM_HEADER->Manufacturer_ID;
			entry->info.sCartID = ROM_HEADER->Cartridge_ID;
			entry->info.cCountry = ROM_HEADER->Country_code;
			entry->info.iCRC1 = ROM_HEADER->CRC1;
			entry->info.iCRC2 = ROM_HEADER->CRC2;

			sprintf( crc_code, "%08X-%08X-C%02X", sl(entry->info.iCRC1), sl(entry->info.iCRC2), entry->info.cCountry );
			entry->iniEntry = ini_search_by_CRC( crc_code );
		   
			if( entry->iniEntry )
			{
				strcpy( entry->info.cComments, entry->iniEntry->comments );
				strcpy( entry->info.cGoodName, entry->iniEntry->goodname );
				strcpy( entry->cName, entry->iniEntry->goodname );
			}
		}
	   
		romentry_fill( entry );
	   
		line[0] = entry->cName;
		line[1] = entry->cCountry;
		line[2] = entry->cSize;
		line[3] = entry->info.cComments;
		line[4] = entry->cFilename;
	   
		gtk_clist_append( GTK_CLIST(g_MainWindow.romCList), line );
	   
		gtk_pixmap_get( GTK_PIXMAP(entry->flag), &pixmap, &mask );
		gtk_clist_set_pixtext( GTK_CLIST(g_MainWindow.romCList), g_iNumRoms, 0,
					entry->cName, 10, pixmap, mask );
	   
		gtk_clist_set_row_data( GTK_CLIST(g_MainWindow.romCList), g_iNumRoms, (gpointer)entry );
	   
		g_RomList = g_list_append( g_RomList, entry );
		g_iNumRoms ++;
	}
	closedir( dir );
}

// list roms
void
rombrowser_refresh( void )
{
	int i;
	SRomEntry *entry;

	// clear list
	for( i = g_iNumRoms - 1; i >= 0; i-- )
	{
		entry = (SRomEntry *)gtk_clist_get_row_data( GTK_CLIST(g_MainWindow.romCList), i );
		free( entry );
		gtk_clist_remove( GTK_CLIST(g_MainWindow.romCList), i );
	}

	g_iNumRoms = 0;
	g_list_free( g_RomList );
	g_RomList = NULL;

	// browse rom dirs
	for( i = 0; i < config_get_number( "NumRomDirs", 0 ); i++ )
	{
		char buf[30];
		const char *dir;
		sprintf( buf, "RomDirectory[%d]", i );
		dir = config_get_string( buf, "" );
		statusbar_message( "status", tr("Scanning directory '%s'..."), dir );
		scan_dir( dir );
	}
	statusbar_message( "status", tr("%d Rom Directories scanned, %d Roms found"), i, g_iNumRoms );

	// save cache
	rombrowser_writeCache();

	// update status bar
	statusbar_message( "num_roms", tr("Total Roms: %d"), g_iNumRoms );

	// sort list
	gtk_clist_sort( GTK_CLIST(g_MainWindow.romCList) );
}

/*********************************************************************************************************
 * callbacks
 */
// column clicked (title) -> sort
static void
callback_columnClicked( GtkCList *clist, gint column, gpointer user_data )
{
	if( g_iSortColumn == column )
		g_SortType = (g_SortType == GTK_SORT_ASCENDING) ? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING;
	else
		g_iSortColumn = column;

	// sort list
	gtk_clist_sort( GTK_CLIST(g_MainWindow.romCList) );
}

// row double clicked -> play rom
static gboolean
callback_rowSelected( GtkWidget *widget, gint row, gint col, GdkEventButton *event, gpointer data )
{
	SRomEntry *entry;

	if( !event )
		return FALSE;

	if( event->type == GDK_2BUTTON_PRESS )
	{
		if( event->button == 1 )
		{
			entry = (SRomEntry *)gtk_clist_get_row_data( GTK_CLIST(g_MainWindow.romCList), row );

			open_rom( entry->cFilename );
			run_rom();

			return TRUE;
		}
	}

	return FALSE;
}

// right click -> show menu
gboolean
callback_buttonPressed( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
	if( event->type == GDK_BUTTON_PRESS )
	{
		if( event->button == 3 ) // right click
		{
			gint row, col;
			int active;

			if( (active = gtk_clist_get_selection_info( GTK_CLIST(widget), event->x, event->y, &row, &col )) )
			{
				gtk_clist_select_row( GTK_CLIST(widget), row, col );
				gtk_widget_set_sensitive( playRomItem, TRUE );
				gtk_widget_set_sensitive( romPropertiesItem, TRUE );
			}
			else
			{
				gtk_widget_set_sensitive( playRomItem, FALSE );
				gtk_widget_set_sensitive( romPropertiesItem, FALSE );
			}
			gtk_menu_popup( GTK_MENU(data), NULL, NULL, NULL, NULL,
 	                      event->button, event->time );

			return TRUE;
		}
	}

	return FALSE;
}

// play rom menu item
static void
callback_playRom( GtkWidget *widget, gpointer data )
{
	GList *list;
	SRomEntry *entry;

	list = GTK_CLIST(g_MainWindow.romCList)->selection;

	if( !list ) // should never happen since the item is only active when a row is selected
		return;

	entry = (SRomEntry *)gtk_clist_get_row_data( GTK_CLIST(g_MainWindow.romCList), (int)list->data );

	open_rom( entry->cFilename );
	run_rom();
}

// rom properties
static void
callback_romProperties( GtkWidget *widget, gpointer data )
{
	GList *list;
	SRomEntry *entry;

	list = GTK_CLIST(g_MainWindow.romCList)->selection;

	if( !list ) // should never happen since the item is only active when a row is selected
		return;

	entry = (SRomEntry *)gtk_clist_get_row_data( GTK_CLIST(g_MainWindow.romCList), (int)list->data );
	show_romPropDialog( entry );
}

// refresh rom browser
static void
callback_refreshRomBrowser( GtkWidget *widget, gpointer data )
{
	rombrowser_refresh();
}

/*********************************************************************************************************
 * gui functions
 */
// create rom browser
int
create_romBrowser( void )
{
	GtkWidget *rightClickMenu;
	GtkWidget *separatorItem;

	gchar *titles[] = {
		(gchar *)tr("Good Name"),
		(gchar *)tr("Country"),
		(gchar *)tr("Size"),
		(gchar *)tr("Comments"),
		(gchar *)tr("File Name")
	};

	// load images
	australia = create_pixmap_d( g_MainWindow.window, australia_xpm );
	europe = create_pixmap_d( g_MainWindow.window, europe_xpm );
	france = create_pixmap_d( g_MainWindow.window, france_xpm );
	germany = create_pixmap_d( g_MainWindow.window, germany_xpm );
	italy = create_pixmap_d( g_MainWindow.window, italy_xpm );
	japan = create_pixmap_d( g_MainWindow.window, japan_xpm );
	spain = create_pixmap_d( g_MainWindow.window, spain_xpm );
	usa = create_pixmap_d( g_MainWindow.window, usa_xpm );
	n64cart = create_pixmap_d( g_MainWindow.window, n64cart_xpm );

	// right-click menu
	rightClickMenu = gtk_menu_new();
	playRomItem = gtk_menu_item_new_with_label( tr("Play Rom") );
	romPropertiesItem = gtk_menu_item_new_with_label( tr("Rom Properties") );
	separatorItem = gtk_menu_item_new();
	refreshRomBrowserItem = gtk_menu_item_new_with_label( tr("Refresh") );

	gtk_menu_append( GTK_MENU(rightClickMenu), playRomItem );
	gtk_menu_append( GTK_MENU(rightClickMenu), romPropertiesItem );
	gtk_menu_append( GTK_MENU(rightClickMenu), separatorItem );
	gtk_menu_append( GTK_MENU(rightClickMenu), refreshRomBrowserItem );

	gtk_widget_show( rightClickMenu );
	gtk_widget_show( playRomItem );
	gtk_widget_show( romPropertiesItem );
	gtk_widget_show( separatorItem );
	gtk_widget_show( refreshRomBrowserItem );

	gtk_signal_connect( GTK_OBJECT(playRomItem), "activate",
				GTK_SIGNAL_FUNC(callback_playRom), (gpointer)NULL );
	gtk_signal_connect( GTK_OBJECT(romPropertiesItem), "activate",
				GTK_SIGNAL_FUNC(callback_romProperties), (gpointer)NULL );
	gtk_signal_connect( GTK_OBJECT(refreshRomBrowserItem), "activate",
				GTK_SIGNAL_FUNC(callback_refreshRomBrowser), (gpointer)NULL );

	// create the rombrowser
	g_MainWindow.romScrolledWindow = gtk_scrolled_window_new( NULL, NULL );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW (g_MainWindow.romScrolledWindow),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
	gtk_box_pack_start( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.romScrolledWindow, TRUE, TRUE, 0 );
	g_MainWindow.romCList = gtk_clist_new_with_titles( 5/*sizeof( titles )  / sizeof( titles[0] )*/, (gchar **)titles );
	gtk_container_add( GTK_CONTAINER(g_MainWindow.romScrolledWindow), g_MainWindow.romCList );

	gtk_clist_set_sort_column( GTK_CLIST(g_MainWindow.romCList), 0 );
	gtk_clist_set_sort_type( GTK_CLIST(g_MainWindow.romCList), GTK_SORT_ASCENDING );
	gtk_clist_set_compare_func( GTK_CLIST(g_MainWindow.romCList), rombrowser_compare );
	gtk_clist_set_column_justification( GTK_CLIST(g_MainWindow.romCList), 2, GTK_JUSTIFY_RIGHT );
	gtk_clist_set_column_width( GTK_CLIST(g_MainWindow.romCList), 0, config_get_number( "RomBrowser ColWidth[0]", 200 ) );
	gtk_clist_set_column_width( GTK_CLIST(g_MainWindow.romCList), 1, config_get_number( "RomBrowser ColWidth[1]", 50 ) );
	gtk_clist_set_column_width( GTK_CLIST(g_MainWindow.romCList), 2, config_get_number( "RomBrowser ColWidth[2]", 80 ) );
	gtk_clist_set_column_width( GTK_CLIST(g_MainWindow.romCList), 3, config_get_number( "RomBrowser ColWidth[3]", 90 ) );
	gtk_clist_set_column_width( GTK_CLIST(g_MainWindow.romCList), 4, config_get_number( "RomBrowser ColWidth[4]", 100 ) );

	gtk_signal_connect( GTK_OBJECT(g_MainWindow.romCList), "click-column",
				GTK_SIGNAL_FUNC(callback_columnClicked), (gpointer)NULL );
	gtk_signal_connect( GTK_OBJECT(g_MainWindow.romCList), "select-row",
				GTK_SIGNAL_FUNC(callback_rowSelected), (gpointer)NULL );
	gtk_signal_connect( GTK_OBJECT(g_MainWindow.romCList), "button-press-event",
				GTK_SIGNAL_FUNC(callback_buttonPressed), (gpointer)rightClickMenu );

	return 0;
}
