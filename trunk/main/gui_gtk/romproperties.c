/***************************************************************************
                          romproperties.c  -  description
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

#include "romproperties.h"

#include "main_gtk.h"
#include "rombrowser.h"
#include "translate.h"

#include "../../memory/memory.h"	// sl()

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

/*********************************************************************************************************
 * globals
 */
SRomPropertiesDialog g_RomPropDialog;
static SRomEntry *g_RomEntry;

/*********************************************************************************************************
 * callbacks
 */
static void
callback_okClicked( GtkWidget *widget, gpointer data )
{
	char crc_code[200];

	gtk_widget_hide( g_RomPropDialog.dialog );
	gtk_grab_remove( g_RomPropDialog.dialog );

	/* save properties */
	strcpy( g_RomEntry->info.cComments, gtk_entry_get_text( GTK_ENTRY(g_RomPropDialog.commentsEntry) ) );

	/* save cache and ini */
	rombrowser_writeCache();
	if( !g_RomEntry->iniEntry )
	{
		sprintf( crc_code, "%08X-%08X-C%02X", sl(g_RomEntry->info.iCRC1), sl(g_RomEntry->info.iCRC2), g_RomEntry->info.cCountry );
		g_RomEntry->iniEntry = ini_search_by_CRC( crc_code );
	}
	if( g_RomEntry->iniEntry )
	{
		strcpy( g_RomEntry->iniEntry->comments, g_RomEntry->info.cComments );
		ini_updateFile(1);
	}

	// update rombrowser
	gtk_clist_set_text( GTK_CLIST(g_MainWindow.romCList), gtk_clist_find_row_from_data( GTK_CLIST(g_MainWindow.romCList), g_RomEntry ),
											3, g_RomEntry->info.cComments );
}

static void
callback_cancelClicked( GtkWidget *widget, gpointer data )
{
	gtk_widget_hide( g_RomPropDialog.dialog );
	gtk_grab_remove( g_RomPropDialog.dialog );
}

// calculate md5
static void
callback_calculateMd5Clicked( GtkWidget *widget, gpointer data )
{
}

// hide on delete
static gint
delete_question_event( GtkWidget *widget, GdkEvent *event, gpointer data )
{
	gtk_widget_hide( widget );
	gtk_grab_remove( g_RomPropDialog.dialog );

	return TRUE; // undeleteable
}


/*********************************************************************************************************
 * show dialog
 */
void
show_romPropDialog( SRomEntry *entry )
{
	char ini_code[200];

	// fill dialog
	gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.romNameEntry), entry->cName );
	gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.sizeEntry), entry->cSize );
	gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.countryEntry), entry->cCountry );
	sprintf( ini_code, "%08X-%08X-C%02X", sl(entry->info.iCRC1), sl(entry->info.iCRC2), entry->info.cCountry );
	gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.iniCodeEntry), ini_code );
	gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.md5Entry), entry->info.cMD5 );
	gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.commentsEntry), entry->info.cComments );

	g_RomEntry = entry;

	// show dialog
	gtk_widget_show_all( g_RomPropDialog.dialog );
	gtk_grab_add( g_RomPropDialog.dialog );
}

/*********************************************************************************************************
 * dialog creation
 */
int
create_romPropDialog( void )
{
	GtkWidget *button_ok, *button_cancel;
	GtkWidget *frame;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *button;

	g_RomPropDialog.dialog = gtk_dialog_new();
	gtk_container_set_border_width( GTK_CONTAINER(g_RomPropDialog.dialog), 10 );
	gtk_window_set_title( GTK_WINDOW(g_RomPropDialog.dialog), tr("Rom Properties") );
	gtk_signal_connect(GTK_OBJECT(g_RomPropDialog.dialog), "delete_event",
				GTK_SIGNAL_FUNC(delete_question_event), (gpointer)NULL );

	// rom info
	frame = gtk_frame_new( tr("Rom Info") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->vbox), frame, TRUE, TRUE, 0 );

	table = gtk_table_new( 7, 3, FALSE );
	gtk_container_set_border_width( GTK_CONTAINER(table), 10 );
	gtk_table_set_col_spacings( GTK_TABLE(table), 10 );
	gtk_container_add( GTK_CONTAINER(frame), table );

	label = gtk_label_new( tr("Rom Name") );
	g_RomPropDialog.romNameEntry = gtk_entry_new();
	gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.romNameEntry), FALSE );
	gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 1, 0, 1 );
	gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.romNameEntry, 1, 3, 0, 1 );

	label = gtk_label_new( tr("Size") );
	g_RomPropDialog.sizeEntry = gtk_entry_new();
	gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.sizeEntry), FALSE );
	gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 1, 1, 2 );
	gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.sizeEntry, 1, 3, 1, 2 );

	label = gtk_label_new( tr("Country") );
	g_RomPropDialog.countryEntry = gtk_entry_new();
	gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.countryEntry), FALSE );
	gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 1, 2, 3 );
	gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.countryEntry, 1, 3, 2, 3 );

	label = gtk_label_new( tr("Ini Code") );
	g_RomPropDialog.iniCodeEntry = gtk_entry_new();
	gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.iniCodeEntry), FALSE );
	gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 1, 3, 4 );
	gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.iniCodeEntry, 1, 3, 3, 4 );

	label = gtk_label_new( tr("MD5 Checksum") );
	g_RomPropDialog.md5Entry = gtk_entry_new();
	gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.md5Entry), FALSE );
	button = gtk_button_new_with_label( tr("Calculate") );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_calculateMd5Clicked), (gpointer)NULL );
	gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 1, 4, 5 );
	gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.md5Entry, 1, 2, 4, 5 );
	gtk_table_attach_defaults( GTK_TABLE(table), button, 2, 3, 4, 5 );

	frame = gtk_frame_new( tr("Comments") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->vbox), frame, TRUE, TRUE, 0 );

	g_RomPropDialog.commentsEntry = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY(g_RomPropDialog.commentsEntry), 199 );

	gtk_container_add( GTK_CONTAINER(frame), g_RomPropDialog.commentsEntry );

	// ok/cancel button
	button_ok = gtk_button_new_with_label( tr("Ok") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->action_area), button_ok, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT(button_ok), "clicked",
				GTK_SIGNAL_FUNC(callback_okClicked), (gpointer)NULL );

	button_cancel = gtk_button_new_with_label( tr("Cancel") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->action_area), button_cancel, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT(button_cancel), "clicked",
				GTK_SIGNAL_FUNC(callback_cancelClicked), (gpointer)NULL );

	return 0;
}
