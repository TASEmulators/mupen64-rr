/***************************************************************************
                          configdialog.c  -  description
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

#include "configdialog.h"

#include "main_gtk.h"
#include "rombrowser.h"
#include "config.h"
#include "dirbrowser.h"
#include "messagebox.h"
#include "support.h"
#include "translate.h"

#include "../winlnxdefs.h"
#include "../plugin.h"

#include <gtk/gtk.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// icons
#include "icons/graphics.xpm"
#include "icons/sound.xpm"
#include "icons/input.xpm"
#include "icons/rsp.xpm"

// there's got to be a better idea than this...
int autoinc_slot = 0;
int *autoinc_save_slot = &autoinc_slot;

/** globals **/
SConfigDialog g_ConfigDialog;
static int g_RefreshRomBrowser; // refresh the rombrowser when ok is clicked?
                                // (true if rom directories were changed)

/** callbacks **/
// gfx
static void
callback_configGfx( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.gfxCombo)->entry) );

	if( name )
		plugin_exec_config( name );
}

static void
callback_testGfx( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.gfxCombo)->entry) );

	if( name )
		plugin_exec_test( name );
}

static void
callback_aboutGfx( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.gfxCombo)->entry) );

	if( name )
		plugin_exec_about( name );
}

// audio
static void
callback_configAudio( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.audioCombo)->entry) );

	if( name )
		plugin_exec_config( name );
}

static void
callback_testAudio( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.audioCombo)->entry) );

	if( name )
		plugin_exec_test( name );
}

static void
callback_aboutAudio( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.audioCombo)->entry) );

	if( name )
		plugin_exec_about( name );
}

// input
static void
callback_configInput( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.inputCombo)->entry) );

	if( name )
		plugin_exec_config( name );
}

static void
callback_testInput( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.inputCombo)->entry) );

	if( name )
		plugin_exec_test( name );
}

static void
callback_aboutInput( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.inputCombo)->entry) );

	if( name )
		plugin_exec_about( name );
}

// RSP
static void
callback_configRSP( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.RSPCombo)->entry) );

	if( name )
		plugin_exec_config( name );
}

static void
callback_testRSP( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.RSPCombo)->entry) );

	if( name )
		plugin_exec_test( name );
}

static void
callback_aboutRSP( GtkWidget *widget, gpointer data )
{
	const char *name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.RSPCombo)->entry) );

	if( name )
		plugin_exec_about( name );
}

// rombrowser
static void
callback_romDirectorySelected( const gchar *dirname )
{
	GList *dir = NULL, *list;
	GtkWidget *item;

	// check if the directory is already in the list
	list = GTK_LIST(g_ConfigDialog.romDirectoryList)->children;
	while( list )
	{
		GtkListItem *item = GTK_LIST_ITEM(list->data);
		GtkLabel *label = GTK_LABEL(GTK_BIN(item)->child);
		char *text = NULL;

		gtk_label_get( label, &text );

		if( text != NULL )
			if( !strcmp( dirname, text ) )
			{
				messagebox( tr("Already in list"), MB_OK | MB_ICONINFORMATION, tr("The directory you selected is already in the list.") );
				return;
			}

		list = list->next;
	}

	item = gtk_list_item_new_with_label( dirname );
	gtk_widget_show( item );
	dir = g_list_append( dir, item );
	gtk_list_append_items( GTK_LIST(g_ConfigDialog.romDirectoryList), dir );
	g_RefreshRomBrowser = 1;
}

static void
callback_romDirectoryAdd( GtkWidget *widget, gpointer data )
{
	GtkWidget *dirbrowser;

        typedef void (*callback)(gchar*);
	dirbrowser = create_dir_browser( (gchar*)tr("Select Rom Directory"), g_WorkingDir, GTK_SELECTION_SINGLE, (callback)callback_romDirectorySelected );

	/* Display that dialog */
	gtk_widget_show( dirbrowser );
}

static void
callback_romDirectoryRemove( GtkWidget *widget, gpointer data )
{
	GList *list, *clear_list = NULL;

	list = GTK_LIST(g_ConfigDialog.romDirectoryList)->selection;

	if( !list )
		return;

	while( list )
	{
		GtkListItem *item = GTK_LIST_ITEM(list->data);
		GtkWidget *child = GTK_BIN(item)->child;

		clear_list = g_list_append( clear_list, child );
		list = list->next;
	}

	gtk_list_remove_items( GTK_LIST(g_ConfigDialog.romDirectoryList), clear_list );
	g_RefreshRomBrowser = 1;
}

static void
callback_romDirectoryRemoveAll( GtkWidget *widget, gpointer data )
{
	gtk_list_clear_items( GTK_LIST(g_ConfigDialog.romDirectoryList), 0, -1 );
	g_RefreshRomBrowser = 1;
}

// ok/cancel button
static void
callback_okClicked( GtkWidget *widget, gpointer data )
{
	const char *filename, *name;
	GList *romDirGList;
	int i;

	// save configuration
	name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.gfxCombo)->entry) );
	if( name )
	{
		filename = plugin_filename_by_name( name );
		if( filename )
			config_put_string( "Gfx Plugin", filename );
	}
	name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.audioCombo)->entry) );
	if( name )
	{
		filename = plugin_filename_by_name( name );
		if( filename )
			config_put_string( "Audio Plugin", filename );
	}
	name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.inputCombo)->entry) );
	if( name )
	{
		filename = plugin_filename_by_name( name );
		if( filename )
			config_put_string( "Input Plugin", filename );
	}
	name = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.RSPCombo)->entry) );
	if( name )
	{
		filename = plugin_filename_by_name( name );
		if( filename )
			config_put_string( "RSP Plugin", filename );
	}

	i = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.romDirsScanRecCheckButton) );
	if( i != config_get_bool( "RomDirsScanRecursive", FALSE ) )
		g_RefreshRomBrowser = 1;
	config_put_bool( "RomDirsScanRecursive", i );
   
        i = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.noAudioDelayCheckButton) );
	config_put_bool( "NoAudioDelay", i );
   
        i = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.noCompiledJumpCheckButton) );
	config_put_bool( "NoCompiledJump", i );
   
        autoinc_slot = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.autoincSaveSlotCheckButton) );
        config_put_bool( "AutoIncSaveSlot", autoinc_slot );

	romDirGList = GTK_LIST(g_ConfigDialog.romDirectoryList)->children;
	i = 0;
	while( romDirGList )
	{
		GtkListItem *item = GTK_LIST_ITEM(romDirGList->data);
		GtkLabel *label = GTK_LABEL(GTK_BIN(item)->child);
		char *text = NULL;

		gtk_label_get( label, &text );

		if( text != NULL )
		{
			char buf[30];
			sprintf( buf, "RomDirectory[%d]", i++ );
			config_put_string( buf, text );
		}

		romDirGList = romDirGList->next;
	}
	config_put_number( "NumRomDirs", i );

	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.coreInterpreterCheckButton) ) )
		config_put_number( "Core", CORE_INTERPRETER );
	else if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.coreDynaRecCheckButton) ) )
		config_put_number( "Core", CORE_DYNAREC );
	else
		config_put_number( "Core", CORE_PURE_INTERPRETER );

	// refresh rom-browser
	if( g_RefreshRomBrowser )
		rombrowser_refresh();

	// hide dialog
	gtk_widget_hide( g_ConfigDialog.dialog );

	// write configuration
	config_write();
}

static void
callback_cancelClicked( GtkWidget *widget, gpointer data )
{
	// hide dialog
	gtk_widget_hide( g_ConfigDialog.dialog );
}

// when the window is shown
static void
callback_dialogShow( GtkWidget *widget, gpointer data )
{
	int i;
	char *name;

	// fill in configuration
	callback_romDirectoryRemoveAll( NULL, NULL );
	for( i = 0; i < config_get_number( "NumRomDirs", 0 ); i++ )
	{
		char buf[30];
		sprintf( buf, "RomDirectory[%d]", i );
		callback_romDirectorySelected( config_get_string( buf, "" ) );
	}

	name = plugin_name_by_filename( config_get_string( "Gfx Plugin", "" ) );
	if( name )
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.gfxCombo)->entry), name );

	name = plugin_name_by_filename( config_get_string( "Audio Plugin", "" ) );
	if( name )
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.audioCombo)->entry), name );

	name = plugin_name_by_filename( config_get_string( "Input Plugin", "" ) );
	if( name )
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.inputCombo)->entry), name );

	name = plugin_name_by_filename( config_get_string( "RSP Plugin", "" ) );
	if( name )
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.RSPCombo)->entry), name );

	switch (config_get_number( "Core", CORE_DYNAREC ))
	{
	case CORE_INTERPRETER:
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.coreInterpreterCheckButton), 1 );
		break;

	case CORE_DYNAREC:
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.coreDynaRecCheckButton), 1 );
		break;

	case CORE_PURE_INTERPRETER:
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.corePureInterpCheckButton), 1 );
		break;
	}

	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.romDirsScanRecCheckButton), config_get_bool( "RomDirsScanRecursive", FALSE ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.noAudioDelayCheckButton), config_get_bool( "NoAudioDelay", FALSE ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.noCompiledJumpCheckButton), config_get_bool( "NoCompiledJump", FALSE ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.autoincSaveSlotCheckButton), config_get_bool( "AutoIncSaveSlot", FALSE ) );

	g_RefreshRomBrowser = 0;
}

// hide on delete
static gint
delete_question_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_widget_hide( widget );

	return TRUE; // undeleteable
}

/** create dialog **/
int
create_configDialog( void )
{
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *button, *button_config, *button_test, *button_about;
	GtkWidget *icon;
	GtkWidget *vbox;
	GtkWidget *hbox1, *hbox2;

	// search plugins
	g_ConfigDialog.gfxPluginGList = NULL;
	g_ConfigDialog.audioPluginGList = NULL;
	g_ConfigDialog.inputPluginGList = NULL;
        g_ConfigDialog.RSPPluginGList = NULL;

	plugin_scan_directory( g_WorkingDir );
   
	while( plugin_type() != -1 )
		switch( plugin_type() )
		{
		case PLUGIN_TYPE_GFX:
			g_ConfigDialog.gfxPluginGList = g_list_append( g_ConfigDialog.gfxPluginGList, plugin_next() );
			break;
		case PLUGIN_TYPE_AUDIO:
			g_ConfigDialog.audioPluginGList = g_list_append( g_ConfigDialog.audioPluginGList, plugin_next() );
			break;
		case PLUGIN_TYPE_CONTROLLER:
			g_ConfigDialog.inputPluginGList = g_list_append( g_ConfigDialog.inputPluginGList, plugin_next() );
			break;
		case PLUGIN_TYPE_RSP:
			g_ConfigDialog.RSPPluginGList = g_list_append( g_ConfigDialog.RSPPluginGList, plugin_next() );
			break;
		}

	// create window
	g_ConfigDialog.dialog = gtk_dialog_new();
	gtk_container_set_border_width( GTK_CONTAINER(g_ConfigDialog.dialog), 10 );
	gtk_window_set_title( GTK_WINDOW(g_ConfigDialog.dialog), tr("Configure") );
	gtk_signal_connect( GTK_OBJECT(g_ConfigDialog.dialog), "show",
				GTK_SIGNAL_FUNC(callback_dialogShow), (gpointer)NULL );
	gtk_signal_connect(GTK_OBJECT(g_ConfigDialog.dialog), "delete_event",
				GTK_SIGNAL_FUNC(delete_question_event), (gpointer)NULL );

	// create notebook
	g_ConfigDialog.notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos( GTK_NOTEBOOK(g_ConfigDialog.notebook), GTK_POS_TOP );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_ConfigDialog.dialog)->vbox), g_ConfigDialog.notebook, TRUE, TRUE, 0 );

	// create ok/cancel button
	button = gtk_button_new_with_label( tr("Ok") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_ConfigDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_okClicked), (gpointer)NULL );

	button = gtk_button_new_with_label( tr("Cancel") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_ConfigDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_cancelClicked), (gpointer)NULL );

	// create mupen configuration page
	label = gtk_label_new( "Mupen64" );
	g_ConfigDialog.configMupen = gtk_vbox_new( FALSE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(g_ConfigDialog.configMupen), 10 );
	gtk_notebook_append_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configMupen, label );

	frame = gtk_frame_new( tr("CPU Core") );
	gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configMupen), frame, FALSE, FALSE, 0 );
	vbox = gtk_vbox_new( TRUE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
	gtk_container_add( GTK_CONTAINER(frame), vbox );

	g_ConfigDialog.coreInterpreterCheckButton = gtk_radio_button_new_with_label(
							NULL,
							tr("Interpreter") );
	g_ConfigDialog.coreDynaRecCheckButton = gtk_radio_button_new_with_label(
							gtk_radio_button_group( GTK_RADIO_BUTTON(g_ConfigDialog.coreInterpreterCheckButton) ),
							tr("Dynamic Recompiler") );
	g_ConfigDialog.corePureInterpCheckButton = gtk_radio_button_new_with_label(
							gtk_radio_button_group( GTK_RADIO_BUTTON(g_ConfigDialog.coreInterpreterCheckButton) ),
							tr("Pure Interpreter") );
	gtk_box_pack_start( GTK_BOX(vbox), g_ConfigDialog.coreInterpreterCheckButton, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(vbox), g_ConfigDialog.coreDynaRecCheckButton, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(vbox), g_ConfigDialog.corePureInterpCheckButton, FALSE, FALSE, 0 );
   
        // special core options
	g_ConfigDialog.noAudioDelayCheckButton = gtk_check_button_new_with_label("No audio delay (it can fix some compatibility issues \n but audio won't be synchronised with video)");
        g_ConfigDialog.noCompiledJumpCheckButton = gtk_check_button_new_with_label("No compiled jump (improves compatibility at the cost of some speed)");
        g_ConfigDialog.autoincSaveSlotCheckButton = gtk_check_button_new_with_label("Auto increment save slot");
        gtk_box_pack_start(GTK_BOX(g_ConfigDialog.configMupen), g_ConfigDialog.noAudioDelayCheckButton, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(g_ConfigDialog.configMupen), g_ConfigDialog.noCompiledJumpCheckButton, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(g_ConfigDialog.configMupen), g_ConfigDialog.autoincSaveSlotCheckButton, FALSE, FALSE, 0);
        
   
///////////
// disable dynarec checkbutton
///////////
	//gtk_widget_set_sensitive( g_ConfigDialog.coreDynaRecCheckButton, FALSE );

	// create plugin config page
	label = gtk_label_new( tr("Plugins") );
	g_ConfigDialog.configPlugins = gtk_vbox_new( FALSE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(g_ConfigDialog.configPlugins), 10 );
	gtk_notebook_append_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configPlugins, label );

	// gfx plugin
	frame = gtk_frame_new( tr("Gfx Plugin") );
	gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configPlugins), frame, FALSE, FALSE, 0 );

	vbox = gtk_vbox_new( TRUE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
	hbox1 = gtk_hbox_new( FALSE, 5 );
	hbox2 = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER(frame), vbox );
	gtk_box_pack_start( GTK_BOX(vbox), hbox1, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(vbox), hbox2, FALSE, FALSE, 0 );

	icon = create_pixmap_d( g_ConfigDialog.dialog, graphics_xpm );

	g_ConfigDialog.gfxCombo = gtk_combo_new();
	if( g_ConfigDialog.gfxPluginGList )
		gtk_combo_set_popdown_strings( GTK_COMBO(g_ConfigDialog.gfxCombo), g_ConfigDialog.gfxPluginGList );
	else
		gtk_widget_set_sensitive( GTK_WIDGET(g_ConfigDialog.gfxCombo), FALSE );
	gtk_combo_set_value_in_list( GTK_COMBO(g_ConfigDialog.gfxCombo), TRUE, FALSE );
	gtk_entry_set_editable( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.gfxCombo)->entry), FALSE );

	button_config = gtk_button_new_with_label( tr("Config") );
	button_test = gtk_button_new_with_label( tr("Test") );
	button_about = gtk_button_new_with_label( tr("About") );

	gtk_box_pack_start( GTK_BOX(hbox1), icon, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.gfxCombo, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_config, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_test, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_about, TRUE, TRUE, 0 );

	gtk_signal_connect( GTK_OBJECT(button_config), "clicked",
				GTK_SIGNAL_FUNC(callback_configGfx), (gpointer) NULL );
	gtk_signal_connect( GTK_OBJECT(button_test), "clicked",
				GTK_SIGNAL_FUNC(callback_testGfx), (gpointer) NULL );
	gtk_signal_connect( GTK_OBJECT(button_about), "clicked",
				GTK_SIGNAL_FUNC(callback_aboutGfx), (gpointer) NULL );

	// audio plugin
	frame = gtk_frame_new( tr("Audio Plugin") );
	gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configPlugins), frame, FALSE, FALSE, 0 );

	vbox = gtk_vbox_new( TRUE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
	hbox1 = gtk_hbox_new( FALSE, 5 );
	hbox2 = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER(frame), vbox );
	gtk_box_pack_start( GTK_BOX(vbox), hbox1, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(vbox), hbox2, FALSE, FALSE, 0 );

	icon = create_pixmap_d( g_ConfigDialog.dialog, sound_xpm );

	g_ConfigDialog.audioCombo = gtk_combo_new();
	if( g_ConfigDialog.audioPluginGList )
		gtk_combo_set_popdown_strings( GTK_COMBO(g_ConfigDialog.audioCombo), g_ConfigDialog.audioPluginGList );
	else
		gtk_widget_set_sensitive( GTK_WIDGET(g_ConfigDialog.audioCombo), FALSE );
	gtk_combo_set_value_in_list( GTK_COMBO(g_ConfigDialog.audioCombo), TRUE, FALSE );
	gtk_entry_set_editable( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.audioCombo)->entry), FALSE );

	button_config = gtk_button_new_with_label( tr("Config") );
	button_test = gtk_button_new_with_label( tr("Test") );
	button_about = gtk_button_new_with_label( tr("About") );

	gtk_box_pack_start( GTK_BOX(hbox1), icon, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.audioCombo, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_config, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_test, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_about, TRUE, TRUE, 0 );

	gtk_signal_connect( GTK_OBJECT(button_config), "clicked",
				GTK_SIGNAL_FUNC(callback_configAudio), (gpointer) NULL );
	gtk_signal_connect( GTK_OBJECT(button_test), "clicked",
				GTK_SIGNAL_FUNC(callback_testAudio), (gpointer) NULL );
	gtk_signal_connect( GTK_OBJECT(button_about), "clicked",
				GTK_SIGNAL_FUNC(callback_aboutAudio), (gpointer) NULL );

	// input plugin
	frame = gtk_frame_new( tr("Input Plugin") );
	gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configPlugins), frame, FALSE, FALSE, 0 );

	vbox = gtk_vbox_new( TRUE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
	hbox1 = gtk_hbox_new( FALSE, 5 );
	hbox2 = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER(frame), vbox );
	gtk_box_pack_start( GTK_BOX(vbox), hbox1, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(vbox), hbox2, FALSE, FALSE, 0 );

	icon = create_pixmap_d( g_ConfigDialog.dialog, input_xpm );

	g_ConfigDialog.inputCombo = gtk_combo_new();
	if( g_ConfigDialog.inputPluginGList )
		gtk_combo_set_popdown_strings( GTK_COMBO(g_ConfigDialog.inputCombo), g_ConfigDialog.inputPluginGList );
	else
		gtk_widget_set_sensitive( GTK_WIDGET(g_ConfigDialog.inputCombo), FALSE );
	gtk_combo_set_value_in_list( GTK_COMBO(g_ConfigDialog.inputCombo), TRUE, FALSE );
	gtk_entry_set_editable( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.inputCombo)->entry), FALSE );

	button_config = gtk_button_new_with_label( tr("Config") );
	button_test = gtk_button_new_with_label( tr("Test") );
	button_about = gtk_button_new_with_label( tr("About") );

	gtk_box_pack_start( GTK_BOX(hbox1), icon, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.inputCombo, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_config, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_test, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_about, TRUE, TRUE, 0 );

	gtk_signal_connect( GTK_OBJECT(button_config), "clicked",
				GTK_SIGNAL_FUNC(callback_configInput), (gpointer) NULL );
	gtk_signal_connect( GTK_OBJECT(button_test), "clicked",
				GTK_SIGNAL_FUNC(callback_testInput), (gpointer) NULL );
	gtk_signal_connect( GTK_OBJECT(button_about), "clicked",
				GTK_SIGNAL_FUNC(callback_aboutInput), (gpointer) NULL );

	// RSP plugin
	frame = gtk_frame_new( tr("RSP Plugin") );
	gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configPlugins), frame, FALSE, FALSE, 0 );

	vbox = gtk_vbox_new( TRUE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
	hbox1 = gtk_hbox_new( FALSE, 5 );
	hbox2 = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER(frame), vbox );
	gtk_box_pack_start( GTK_BOX(vbox), hbox1, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(vbox), hbox2, FALSE, FALSE, 0 );

	icon = create_pixmap_d( g_ConfigDialog.dialog, rsp_xpm );

	g_ConfigDialog.RSPCombo = gtk_combo_new();
	if( g_ConfigDialog.RSPPluginGList )
		gtk_combo_set_popdown_strings( GTK_COMBO(g_ConfigDialog.RSPCombo), g_ConfigDialog.RSPPluginGList );
	else
		gtk_widget_set_sensitive( GTK_WIDGET(g_ConfigDialog.RSPCombo), FALSE );
	gtk_combo_set_value_in_list( GTK_COMBO(g_ConfigDialog.RSPCombo), TRUE, FALSE );
	gtk_entry_set_editable( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.RSPCombo)->entry), FALSE );

	button_config = gtk_button_new_with_label( tr("Config") );
	button_test = gtk_button_new_with_label( tr("Test") );
	button_about = gtk_button_new_with_label( tr("About") );

	gtk_box_pack_start( GTK_BOX(hbox1), icon, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.RSPCombo, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_config, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_test, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox2), button_about, TRUE, TRUE, 0 );

	gtk_signal_connect( GTK_OBJECT(button_config), "clicked",
				GTK_SIGNAL_FUNC(callback_configRSP), (gpointer) NULL );
	gtk_signal_connect( GTK_OBJECT(button_test), "clicked",
				GTK_SIGNAL_FUNC(callback_testRSP), (gpointer) NULL );
	gtk_signal_connect( GTK_OBJECT(button_about), "clicked",
				GTK_SIGNAL_FUNC(callback_aboutRSP), (gpointer) NULL );

	// create rombrowser config page
	label = gtk_label_new( tr("Rom Browser") );
	g_ConfigDialog.configRomBrowser = gtk_vbox_new( FALSE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(g_ConfigDialog.configRomBrowser), 10 );
	gtk_notebook_append_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configRomBrowser, label );

	frame = gtk_frame_new( tr("Rom Directories") );
	gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configRomBrowser), frame, TRUE, TRUE, 0 );

	hbox1 = gtk_hbox_new( FALSE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(hbox1), 10 );
	gtk_container_add( GTK_CONTAINER(frame), hbox1 );

	g_ConfigDialog.romDirectoryList = gtk_list_new();
	gtk_list_set_selection_mode( GTK_LIST(g_ConfigDialog.romDirectoryList), GTK_SELECTION_MULTIPLE );
	gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.romDirectoryList, TRUE, TRUE, 0 );

	vbox = gtk_vbox_new( TRUE, 5 );
	gtk_box_pack_start( GTK_BOX(hbox1), vbox, FALSE, FALSE, 0 );

	button = gtk_button_new_with_label( tr("Add") );
	gtk_box_pack_start( GTK_BOX(vbox), button, FALSE, FALSE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_romDirectoryAdd), (gpointer) NULL );

	button = gtk_button_new_with_label( tr("Remove") );
	gtk_box_pack_start( GTK_BOX(vbox), button, FALSE, FALSE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_romDirectoryRemove), (gpointer) NULL );

	button = gtk_button_new_with_label( tr("Remove All") );
	gtk_box_pack_start( GTK_BOX(vbox), button, FALSE, FALSE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_romDirectoryRemoveAll), (gpointer) NULL );

	g_ConfigDialog.romDirsScanRecCheckButton = gtk_check_button_new_with_label( tr("Recursively scan rom directories") );
	gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configRomBrowser), g_ConfigDialog.romDirsScanRecCheckButton, FALSE, FALSE, 0 );
	

	// init widgets (load configuration into widgets)
	callback_dialogShow( NULL, NULL );

	return 0;
}
