/***************************************************************************
                          vcrcomp_dialog.c  -  description
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

#include "vcrcomp_dialog.h"

#include "main_gtk.h"
#include "config.h"
#include "messagebox.h"
#include "support.h"
#include "translate.h"

#include "../vcr_compress.h"

#include <gtk/gtk.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum EditMode
{
	VideoCodec,
	AudioCodec
};

/** globals **/
SVcrCompDialog g_VcrCompDialog;
static int     m_EditMode = VideoCodec;

static int
selectedVideoCodecIndex()
{
	int i;
	const char *selectedCodec = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_VcrCompDialog.videoCodecCombo)->entry) );

	for (i = 0; i < VCRComp_numVideoCodecs(); i++)
	{
		if (!strcmp( selectedCodec, VCRComp_videoCodecName( i ) ))
			return i;
	}

	return -1;
}

static int
selectedVideoAttribIndex()
{
	int i, codec = selectedVideoCodecIndex();
	const char *selectedAttrib = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_VcrCompDialog.videoAttribCombo)->entry) );

	for (i = 0; i < VCRComp_numVideoCodecAttribs( codec ); i++)
	{
		if (!strcmp( selectedAttrib, VCRComp_videoCodecAttribName( codec, i ) ))
			return i;
	}

	return -1;
}



static int
selectedAudioCodecIndex()
{
	int i;
	const char *selectedCodec = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_VcrCompDialog.audioCodecCombo)->entry) );

	for (i = 0; i < VCRComp_numAudioCodecs(); i++)
	{
		if (!strcmp( selectedCodec, VCRComp_audioCodecName( i ) ))
			return i;
	}

	return -1;
}

static int
selectedAudioAttribIndex()
{
	int i, codec = selectedAudioCodecIndex();
	const char *selectedAttrib = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_VcrCompDialog.audioAttribCombo)->entry) );

	for (i = 0; i < VCRComp_numAudioCodecAttribs( codec ); i++)
	{
		if (!strcmp( selectedAttrib, VCRComp_audioCodecAttribName( codec, i ) ))
			return i;
	}

	return -1;
}

/*****************************************************************************
 * change dialogs
 */
struct
{
	GtkWidget	*dialog;
	GtkWidget	*entry;
} m_IntegerDialog;

struct
{
	GtkWidget	*dialog;
	GtkWidget	*entry;
} m_StringDialog;

struct
{
	GtkWidget	*dialog;
	GtkWidget	*combo;
} m_SelectDialog;


static gint delete_question_event(GtkWidget *widget, GdkEvent *event, gpointer data);
static void callback_videoAttribChanged( GtkWidget *widget, gpointer data );
static void callback_audioAttribChanged( GtkWidget *widget, gpointer data );

static void
callback_integerChangeOkClicked( GtkWidget *widget, gpointer data )
{
	const char *val;
	int ci, ai;

	val = gtk_entry_get_text( GTK_ENTRY(m_IntegerDialog.entry) );

	if (m_EditMode == VideoCodec)
	{
		ci = selectedVideoCodecIndex();
		ai = selectedVideoAttribIndex();
		VCRComp_videoCodecAttribSetValue( ci, ai, val );
		callback_videoAttribChanged( NULL, NULL );
	}
	else if (m_EditMode == AudioCodec)
	{
		ci = selectedAudioCodecIndex();
		ai = selectedAudioAttribIndex();
		VCRComp_audioCodecAttribSetValue( ci, ai, val );
		callback_audioAttribChanged( NULL, NULL );
	}

	gtk_widget_hide( m_IntegerDialog.dialog );
}

static void
callback_stringChangeOkClicked( GtkWidget *widget, gpointer data )
{
	const char *val;
	int ci, ai;

	val = gtk_entry_get_text( GTK_ENTRY(m_StringDialog.entry) );

	if (m_EditMode == VideoCodec)
	{
		ci = selectedVideoCodecIndex();
		ai = selectedVideoAttribIndex();
		VCRComp_videoCodecAttribSetValue( ci, ai, val );
		callback_videoAttribChanged( NULL, NULL );
	}
	else if (m_EditMode == AudioCodec)
	{
		ci = selectedAudioCodecIndex();
		ai = selectedAudioAttribIndex();
		VCRComp_audioCodecAttribSetValue( ci, ai, val );
		callback_audioAttribChanged( NULL, NULL );
	}

	gtk_widget_hide( m_StringDialog.dialog );
}

static void
callback_selectChangeOkClicked( GtkWidget *widget, gpointer data )
{
	const char *val;
	int ci, ai;

	val = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(m_SelectDialog.combo)->entry) );
	if (m_EditMode == VideoCodec)
	{
		ci = selectedVideoCodecIndex();
		ai = selectedVideoAttribIndex();
		VCRComp_videoCodecAttribSetValue( ci, ai, val );
		callback_videoAttribChanged( NULL, NULL );
	}
	else if (m_EditMode == AudioCodec)
	{
		ci = selectedAudioCodecIndex();
		ai = selectedAudioAttribIndex();
		VCRComp_audioCodecAttribSetValue( ci, ai, val );
		callback_audioAttribChanged( NULL, NULL );
	}

	gtk_widget_hide( m_SelectDialog.dialog );
}

static void
callback_changeDialogHidden( GtkWidget *widget, gpointer data )
{
	gtk_grab_remove( widget );
}

static void
create_changeDialogs()
{
	GtkWidget *button;

	// integer dialog
	m_IntegerDialog.dialog = gtk_dialog_new();
	gtk_container_set_border_width( GTK_CONTAINER(m_IntegerDialog.dialog), 10 );
	gtk_window_set_title( GTK_WINDOW(m_IntegerDialog.dialog), tr("Change") );
	gtk_signal_connect(GTK_OBJECT(m_IntegerDialog.dialog), "delete_event",
				GTK_SIGNAL_FUNC(delete_question_event), (gpointer)NULL );
	gtk_signal_connect(GTK_OBJECT(m_IntegerDialog.dialog), "hide",
				GTK_SIGNAL_FUNC(callback_changeDialogHidden), (gpointer)NULL );

	m_IntegerDialog.entry = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_IntegerDialog.dialog)->vbox), m_IntegerDialog.entry, TRUE, TRUE, 0 );

	button = gtk_button_new_with_label( tr("Ok") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_IntegerDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_integerChangeOkClicked), (gpointer)NULL );

	button = gtk_button_new_with_label( tr("Cancel") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_IntegerDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
	gtk_signal_connect_object( GTK_OBJECT(button), "clicked",
	                GTK_SIGNAL_FUNC(gtk_widget_hide), GTK_OBJECT(m_IntegerDialog.dialog) );


	// string dialog
	m_StringDialog.dialog = gtk_dialog_new();
	gtk_container_set_border_width( GTK_CONTAINER(m_StringDialog.dialog), 10 );
	gtk_window_set_title( GTK_WINDOW(m_StringDialog.dialog), tr("Change") );
	gtk_signal_connect(GTK_OBJECT(m_StringDialog.dialog), "delete_event",
				GTK_SIGNAL_FUNC(delete_question_event), (gpointer)NULL );
	gtk_signal_connect(GTK_OBJECT(m_StringDialog.dialog), "hide",
				GTK_SIGNAL_FUNC(callback_changeDialogHidden), (gpointer)NULL );

	m_StringDialog.entry = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_StringDialog.dialog)->vbox), m_StringDialog.entry, TRUE, TRUE, 0 );

	button = gtk_button_new_with_label( tr("Ok") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_StringDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_stringChangeOkClicked), (gpointer)NULL );

	button = gtk_button_new_with_label( tr("Cancel") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_StringDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
	gtk_signal_connect_object( GTK_OBJECT(button), "clicked",
	                GTK_SIGNAL_FUNC(gtk_widget_hide), GTK_OBJECT(m_StringDialog.dialog) );


	// select dialog
	m_SelectDialog.dialog = gtk_dialog_new();
	gtk_container_set_border_width( GTK_CONTAINER(m_SelectDialog.dialog), 10 );
	gtk_window_set_title( GTK_WINDOW(m_SelectDialog.dialog), tr("Change") );
	gtk_signal_connect(GTK_OBJECT(m_SelectDialog.dialog), "delete_event",
				GTK_SIGNAL_FUNC(delete_question_event), (gpointer)NULL );
	gtk_signal_connect(GTK_OBJECT(m_SelectDialog.dialog), "hide",
				GTK_SIGNAL_FUNC(callback_changeDialogHidden), (gpointer)NULL );

	m_SelectDialog.combo = gtk_combo_new();
	gtk_combo_set_value_in_list( GTK_COMBO(m_SelectDialog.combo), TRUE, FALSE );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_SelectDialog.dialog)->vbox), m_SelectDialog.combo, TRUE, TRUE, 0 );

	button = gtk_button_new_with_label( tr("Ok") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_SelectDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_selectChangeOkClicked), (gpointer)NULL );

	button = gtk_button_new_with_label( tr("Cancel") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_SelectDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
	gtk_signal_connect_object( GTK_OBJECT(button), "clicked",
	                GTK_SIGNAL_FUNC(gtk_widget_hide), GTK_OBJECT(m_SelectDialog.dialog) );
}



/*****************************************************************************
 ** callbacks
 **/

static void
callback_videoAttribChanged( GtkWidget *widget, gpointer data )
{
	int ci, ai;

	ci = selectedVideoCodecIndex();
	ai = selectedVideoAttribIndex();

	gtk_entry_set_text( GTK_ENTRY(g_VcrCompDialog.videoAttribEntry),
	                    VCRComp_videoCodecAttribValue( ci, ai ) );
}

static void
callback_videoCodecChanged( GtkWidget *widget, gpointer data )
{
	int codec = selectedVideoCodecIndex();
	int i;

	if (g_VcrCompDialog.videoAttribGList != 0)
	{
		g_list_free( g_VcrCompDialog.videoAttribGList );
		g_VcrCompDialog.videoAttribGList = 0;
	}

	for (i = 0; i < VCRComp_numVideoCodecAttribs( codec ); i++)
		g_VcrCompDialog.videoAttribGList = g_list_append( g_VcrCompDialog.videoAttribGList,
		                                                  (gchar*)VCRComp_videoCodecAttribName( codec, i ) );

	if (g_VcrCompDialog.videoAttribGList == 0)
	{
		gtk_widget_set_sensitive( g_VcrCompDialog.videoAttribCombo, FALSE );
		gtk_widget_set_sensitive( g_VcrCompDialog.videoAttribEntry, FALSE );
		gtk_widget_set_sensitive( g_VcrCompDialog.videoAttribChangeButton, FALSE );
		gtk_entry_set_text( GTK_ENTRY(g_VcrCompDialog.videoAttribEntry), "" );
	}
	else
	{
		gtk_widget_set_sensitive( g_VcrCompDialog.videoAttribCombo, TRUE );
		gtk_widget_set_sensitive( g_VcrCompDialog.videoAttribEntry, TRUE );
		gtk_widget_set_sensitive( g_VcrCompDialog.videoAttribChangeButton, TRUE );
		gtk_combo_set_popdown_strings( GTK_COMBO(g_VcrCompDialog.videoAttribCombo), g_VcrCompDialog.videoAttribGList );
		callback_videoAttribChanged( NULL, NULL );
	}
}

static void
callback_videoAttribChangeClicked( GtkWidget *widget, gpointer data )
{
	int ci, ai, i;
	GList *list = 0;
	char buf[200];

	ci = selectedVideoCodecIndex();
	ai = selectedVideoAttribIndex();
	sprintf( buf, tr("Change %s"), VCRComp_videoCodecAttribName( ci, ai ) );

	m_EditMode = VideoCodec;
	switch (VCRComp_videoCodecAttribKind( ci, ai ))
	{
	case ATTRIB_INTEGER:
		gtk_window_set_title( GTK_WINDOW(m_IntegerDialog.dialog), buf );
		gtk_entry_set_text( GTK_ENTRY(m_IntegerDialog.entry),
		                    VCRComp_videoCodecAttribValue( ci, ai ) );
		gtk_widget_show_all( m_IntegerDialog.dialog );
		gtk_grab_add( m_IntegerDialog.dialog );
		break;

	case ATTRIB_STRING:
		gtk_window_set_title( GTK_WINDOW(m_StringDialog.dialog), buf );
		gtk_entry_set_text( GTK_ENTRY(m_StringDialog.entry),
		                    VCRComp_videoCodecAttribValue( ci, ai ) );
		gtk_widget_show_all( m_StringDialog.dialog );
		gtk_grab_add( m_StringDialog.dialog );
		break;

	case ATTRIB_SELECT:
		gtk_window_set_title( GTK_WINDOW(m_SelectDialog.dialog), buf );
		for (i = 0; i < VCRComp_numVideoCodecAttribOptions( ci, ai ); i++)
			list = g_list_append( list, (gchar*)VCRComp_videoCodecAttribOption( ci, ai, i ) );
		gtk_combo_set_popdown_strings( GTK_COMBO(m_SelectDialog.combo), list );
		gtk_widget_show_all( m_SelectDialog.dialog );
		gtk_grab_add( m_SelectDialog.dialog );
		break;
	}
}


static void
callback_audioAttribChanged( GtkWidget *widget, gpointer data )
{
	int ci, ai;

	ci = selectedAudioCodecIndex();
	ai = selectedAudioAttribIndex();

	gtk_entry_set_text( GTK_ENTRY(g_VcrCompDialog.audioAttribEntry),
	                    VCRComp_audioCodecAttribValue( ci, ai ) );
}

static void
callback_audioCodecChanged( GtkWidget *widget, gpointer data )
{
	int codec = selectedAudioCodecIndex();
	int i;

	if (g_VcrCompDialog.audioAttribGList != 0)
	{
		g_list_free( g_VcrCompDialog.audioAttribGList );
		g_VcrCompDialog.audioAttribGList = 0;
	}

	for (i = 0; i < VCRComp_numAudioCodecAttribs( codec ); i++)
		g_VcrCompDialog.audioAttribGList = g_list_append( g_VcrCompDialog.audioAttribGList,
		                                                  (gchar*)VCRComp_audioCodecAttribName( codec, i ) );

	if (g_VcrCompDialog.audioAttribGList == 0)
	{
		gtk_widget_set_sensitive( g_VcrCompDialog.audioAttribCombo, FALSE );
		gtk_widget_set_sensitive( g_VcrCompDialog.audioAttribEntry, FALSE );
		gtk_widget_set_sensitive( g_VcrCompDialog.audioAttribChangeButton, FALSE );
		gtk_entry_set_text( GTK_ENTRY(g_VcrCompDialog.audioAttribEntry), "" );
	}
	else
	{
		gtk_widget_set_sensitive( g_VcrCompDialog.audioAttribCombo, TRUE );
		gtk_widget_set_sensitive( g_VcrCompDialog.audioAttribEntry, TRUE );
		gtk_widget_set_sensitive( g_VcrCompDialog.audioAttribChangeButton, TRUE );
		gtk_combo_set_popdown_strings( GTK_COMBO(g_VcrCompDialog.audioAttribCombo), g_VcrCompDialog.audioAttribGList );
		callback_audioAttribChanged( NULL, NULL );
	}
}

static void
callback_audioAttribChangeClicked( GtkWidget *widget, gpointer data )
{
	int ci, ai, i;
	GList *list = 0;
	char buf[200];

	ci = selectedAudioCodecIndex();
	ai = selectedAudioAttribIndex();
	sprintf( buf, tr("Change %s"), VCRComp_audioCodecAttribName( ci, ai ) );

	m_EditMode = AudioCodec;
	switch (VCRComp_audioCodecAttribKind( ci, ai ))
	{
	case ATTRIB_INTEGER:
		gtk_window_set_title( GTK_WINDOW(m_IntegerDialog.dialog), buf );
		gtk_entry_set_text( GTK_ENTRY(m_IntegerDialog.entry),
		                    VCRComp_audioCodecAttribValue( ci, ai ) );
		gtk_widget_show_all( m_IntegerDialog.dialog );
		gtk_grab_add( m_IntegerDialog.dialog );
		break;

	case ATTRIB_STRING:
		gtk_window_set_title( GTK_WINDOW(m_StringDialog.dialog), buf );
		gtk_entry_set_text( GTK_ENTRY(m_StringDialog.entry),
		                    VCRComp_audioCodecAttribValue( ci, ai ) );
		gtk_widget_show_all( m_StringDialog.dialog );
		gtk_grab_add( m_StringDialog.dialog );
		break;

	case ATTRIB_SELECT:
		gtk_window_set_title( GTK_WINDOW(m_SelectDialog.dialog), buf );
		for (i = 0; i < VCRComp_numAudioCodecAttribOptions( ci, ai ); i++)
			list = g_list_append( list, (gchar*)VCRComp_audioCodecAttribOption( ci, ai, i ) );
		gtk_combo_set_popdown_strings( GTK_COMBO(m_SelectDialog.combo), list );
		gtk_widget_show_all( m_SelectDialog.dialog );
		gtk_grab_add( m_SelectDialog.dialog );
		break;
	}
}




// ok/cancel button
static void
callback_okClicked( GtkWidget *widget, gpointer data )
{
	int vci, aci;

	vci = selectedVideoCodecIndex();
	aci = selectedAudioCodecIndex();
	VCRComp_selectVideoCodec( vci );
	VCRComp_selectAudioCodec( aci );

	config_put_string( "VCR Video Codec", VCRComp_videoCodecName( vci ) );
	config_put_string( "VCR Audio Codec", VCRComp_audioCodecName( aci ) );

	// hide dialog
	gtk_widget_hide( g_VcrCompDialog.dialog );
}

static void
callback_cancelClicked( GtkWidget *widget, gpointer data )
{
	// hide dialog
	gtk_widget_hide( g_VcrCompDialog.dialog );
}

// when the window is shown
static void
callback_dialogShow( GtkWidget *widget, gpointer data )
{
	if (g_VcrCompDialog.videoCodecGList)
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(g_VcrCompDialog.videoCodecCombo)->entry),
		                    config_get_string( "VCR Video Codec", "XviD" ) );
	if (g_VcrCompDialog.audioCodecGList)
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(g_VcrCompDialog.audioCodecCombo)->entry),
		                    config_get_string( "VCR Audio Codec", VCRComp_audioCodecName( 0 ) ) );
}

// hide on delete
static gint
delete_question_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_widget_hide( widget );

	return TRUE; // undeleteable
}


/*****************************************************************************
 ** create dialog
 **/
int
create_vcrCompDialog( void )
{
	int i;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget *attribFrame, *attribVbox;
	GtkWidget *hbox;

	g_VcrCompDialog.videoAttribGList = 0;
	g_VcrCompDialog.videoCodecGList = 0;
	for (i = 0; i < VCRComp_numVideoCodecs(); i++)
		g_VcrCompDialog.videoCodecGList = g_list_append( g_VcrCompDialog.videoCodecGList,
		                                                 (gchar*)VCRComp_videoCodecName( i ) );
	g_VcrCompDialog.audioAttribGList = 0;
	g_VcrCompDialog.audioCodecGList = 0;
	for (i = 0; i < VCRComp_numAudioCodecs(); i++)
		g_VcrCompDialog.audioCodecGList = g_list_append( g_VcrCompDialog.audioCodecGList,
		                                                 (gchar*)VCRComp_audioCodecName( i ) );

	// create window
	g_VcrCompDialog.dialog = gtk_dialog_new();
	gtk_container_set_border_width( GTK_CONTAINER(g_VcrCompDialog.dialog), 10 );
	gtk_window_set_title( GTK_WINDOW(g_VcrCompDialog.dialog), tr("Configure VCR") );
	gtk_signal_connect( GTK_OBJECT(g_VcrCompDialog.dialog), "show",
				GTK_SIGNAL_FUNC(callback_dialogShow), (gpointer)NULL );
	gtk_signal_connect(GTK_OBJECT(g_VcrCompDialog.dialog), "delete_event",
				GTK_SIGNAL_FUNC(delete_question_event), (gpointer)NULL );

	// create ok/cancel button
	button = gtk_button_new_with_label( tr("Ok") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_VcrCompDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_okClicked), (gpointer)NULL );

	button = gtk_button_new_with_label( tr("Cancel") );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_VcrCompDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(callback_cancelClicked), (gpointer)NULL );

	// create notebook
	g_VcrCompDialog.notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos( GTK_NOTEBOOK(g_VcrCompDialog.notebook), GTK_POS_TOP );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_VcrCompDialog.dialog)->vbox), g_VcrCompDialog.notebook, TRUE, TRUE, 0 );

	// create video page
	label = gtk_label_new( tr("Video") );
	g_VcrCompDialog.videoPage = gtk_vbox_new( FALSE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(g_VcrCompDialog.videoPage), 10 );
	gtk_notebook_append_page( GTK_NOTEBOOK(g_VcrCompDialog.notebook), g_VcrCompDialog.videoPage, label );

	label = gtk_label_new( tr("Select codec:") );
	gtk_box_pack_start( GTK_BOX(g_VcrCompDialog.videoPage), label, FALSE, FALSE, 0 );

	g_VcrCompDialog.videoCodecCombo = gtk_combo_new();
	if (g_VcrCompDialog.videoCodecGList)
		gtk_combo_set_popdown_strings( GTK_COMBO(g_VcrCompDialog.videoCodecCombo), g_VcrCompDialog.videoCodecGList );
	else
		gtk_widget_set_sensitive( g_VcrCompDialog.videoCodecCombo, FALSE );
	gtk_combo_set_value_in_list( GTK_COMBO(g_VcrCompDialog.videoCodecCombo), TRUE, FALSE );
	gtk_box_pack_start( GTK_BOX(g_VcrCompDialog.videoPage), g_VcrCompDialog.videoCodecCombo, FALSE, FALSE, 0 );

	attribFrame = gtk_frame_new( tr("Codec attributes:") );
	gtk_box_pack_start( GTK_BOX(g_VcrCompDialog.videoPage), attribFrame, FALSE, FALSE, 0 );
	attribVbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER(attribFrame), attribVbox );

	g_VcrCompDialog.videoAttribCombo = gtk_combo_new();
	gtk_combo_set_value_in_list( GTK_COMBO(g_VcrCompDialog.videoAttribCombo), TRUE, FALSE );
	gtk_box_pack_start( GTK_BOX(attribVbox), g_VcrCompDialog.videoAttribCombo, FALSE, FALSE, 0 );

	hbox = gtk_hbox_new( FALSE, 5 );
	label = gtk_label_new( tr("Current value:") );
	gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0 );
	g_VcrCompDialog.videoAttribEntry = gtk_entry_new();
	gtk_entry_set_editable( GTK_ENTRY(g_VcrCompDialog.videoAttribEntry), FALSE );
	gtk_box_pack_start( GTK_BOX(hbox), g_VcrCompDialog.videoAttribEntry, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(attribVbox), hbox, FALSE, FALSE, 0 );

	g_VcrCompDialog.videoAttribChangeButton = gtk_button_new_with_label( tr("Change") );
	gtk_box_pack_start( GTK_BOX(attribVbox), g_VcrCompDialog.videoAttribChangeButton, TRUE, TRUE, 0 );

	gtk_signal_connect( GTK_OBJECT(GTK_COMBO(g_VcrCompDialog.videoCodecCombo)->entry), "changed",
				GTK_SIGNAL_FUNC(callback_videoCodecChanged), (gpointer)NULL );
	gtk_signal_connect( GTK_OBJECT(GTK_COMBO(g_VcrCompDialog.videoAttribCombo)->entry), "changed",
				GTK_SIGNAL_FUNC(callback_videoAttribChanged), (gpointer)NULL );
	gtk_signal_connect( GTK_OBJECT(g_VcrCompDialog.videoAttribChangeButton), "clicked",
				GTK_SIGNAL_FUNC(callback_videoAttribChangeClicked), (gpointer)NULL );

	callback_videoCodecChanged( NULL, NULL );


	// create audio page
	label = gtk_label_new( tr("Audio") );
	g_VcrCompDialog.audioPage = gtk_vbox_new( FALSE, 5 );
	gtk_container_set_border_width( GTK_CONTAINER(g_VcrCompDialog.audioPage), 10 );
	gtk_notebook_append_page( GTK_NOTEBOOK(g_VcrCompDialog.notebook), g_VcrCompDialog.audioPage, label );

	label = gtk_label_new( tr("Select codec:") );
	gtk_box_pack_start( GTK_BOX(g_VcrCompDialog.audioPage), label, FALSE, FALSE, 0 );

	g_VcrCompDialog.audioCodecCombo = gtk_combo_new();
	if (g_VcrCompDialog.audioCodecGList)
		gtk_combo_set_popdown_strings( GTK_COMBO(g_VcrCompDialog.audioCodecCombo), g_VcrCompDialog.audioCodecGList );
	else
		gtk_widget_set_sensitive( g_VcrCompDialog.audioCodecCombo, FALSE );
	gtk_combo_set_value_in_list( GTK_COMBO(g_VcrCompDialog.audioCodecCombo), TRUE, FALSE );
	gtk_box_pack_start( GTK_BOX(g_VcrCompDialog.audioPage), g_VcrCompDialog.audioCodecCombo, FALSE, FALSE, 0 );

	attribFrame = gtk_frame_new( tr("Codec attributes:") );
	gtk_box_pack_start( GTK_BOX(g_VcrCompDialog.audioPage), attribFrame, FALSE, FALSE, 0 );
	attribVbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER(attribFrame), attribVbox );

	g_VcrCompDialog.audioAttribCombo = gtk_combo_new();
	gtk_combo_set_value_in_list( GTK_COMBO(g_VcrCompDialog.audioAttribCombo), TRUE, FALSE );
	gtk_box_pack_start( GTK_BOX(attribVbox), g_VcrCompDialog.audioAttribCombo, FALSE, FALSE, 0 );

	hbox = gtk_hbox_new( FALSE, 5 );
	label = gtk_label_new( tr("Current value:") );
	gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0 );
	g_VcrCompDialog.audioAttribEntry = gtk_entry_new();
	gtk_entry_set_editable( GTK_ENTRY(g_VcrCompDialog.audioAttribEntry), FALSE );
	gtk_box_pack_start( GTK_BOX(hbox), g_VcrCompDialog.audioAttribEntry, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(attribVbox), hbox, FALSE, FALSE, 0 );

	g_VcrCompDialog.audioAttribChangeButton = gtk_button_new_with_label( tr("Change") );
	gtk_box_pack_start( GTK_BOX(attribVbox), g_VcrCompDialog.audioAttribChangeButton, TRUE, TRUE, 0 );

	gtk_signal_connect( GTK_OBJECT(GTK_COMBO(g_VcrCompDialog.audioCodecCombo)->entry), "changed",
				GTK_SIGNAL_FUNC(callback_audioCodecChanged), (gpointer)NULL );
	gtk_signal_connect( GTK_OBJECT(GTK_COMBO(g_VcrCompDialog.audioAttribCombo)->entry), "changed",
				GTK_SIGNAL_FUNC(callback_audioAttribChanged), (gpointer)NULL );
	gtk_signal_connect( GTK_OBJECT(g_VcrCompDialog.audioAttribChangeButton), "clicked",
				GTK_SIGNAL_FUNC(callback_audioAttribChangeClicked), (gpointer)NULL );

	callback_audioCodecChanged( NULL, NULL );


	// create dialogs
	create_changeDialogs();

	return 0;
}
