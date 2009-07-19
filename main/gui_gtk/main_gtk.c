/***************************************************************************
                          main_gtk.c  -  description
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

#define NAME	"Mupen 64"
#define VERSION	"0.5"

#include "../winlnxdefs.h"
#include "../guifuncs.h"
#include "main_gtk.h"
#include "aboutdialog.h"
#include "configdialog.h"
#include "rombrowser.h"
#include "romproperties.h"
#include "config.h"
#include "messagebox.h"
#include "translate.h"	// translation
#include "support.h"	// create_pixmap_d()
#include "vcrcomp_dialog.h"

#include "../plugin.h"
#include "../rom.h"
#include "../../r4300/r4300.h"
#include "../../r4300/recomph.h"
#include "../../memory/memory.h"
#include "../savestates.h"
#include "../vcr.h"
#include "../vcr_compress.h"

#ifdef DBG
#include <glib.h>
#include "../../debugger/debugger.h"
#endif

#include <gtk/gtk.h>

#include <SDL.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>	// POSIX Thread library
#include <signal.h>	// signals
#include <sys/stat.h>

#ifdef WITH_HOME
#include <dirent.h>
#endif

/** icons **/
#include "icons/stop.xpm"
#include "icons/configure.xpm"
#include "icons/fullscreen.xpm"
#include "icons/open.xpm"
#include "icons/pause.xpm"
#include "icons/play.xpm"

/** function prototypes **/
static void *emulationThread( void *_arg );
static void sighandler( int signal, siginfo_t *info, void *context ); // signal handler
void callback_startEmulation( GtkWidget *widget, gpointer data );
void callback_stopEmulation( GtkWidget *widget, gpointer data );
static int create_mainWindow( void );

/** globals **/
char		g_WorkingDir[PATH_MAX];
SMainWindow	g_MainWindow;
static int	g_EmulationRunning = 0;		// core running?
static pthread_t	g_EmulationThread = 0;	// core thread handle
#ifdef DBG
static int	g_DebuggerEnabled = 0;		// wether the debugger is enabled or not
#endif
int	g_GuiEnabled = 1;			// wether the gui is enabled or not
static int	g_Fullscreen = 0;			// wether fullscreen was enabled via command line
static char	*g_Filename = 0;			// filename to load & run at startup (if given at command line)

extern int *autoinc_save_slot;

/** status bar **/
typedef struct
{
	const char *name;
	gint        id;
	GtkWidget  *bar;
} statusBarSection;

static statusBarSection statusBarSections[] = {
	{ "status", -1, NULL },
	{ "num_roms", -1, NULL },
	{ NULL, -1, NULL }
};

/*********************************************************************************************************
 * exported gui funcs
 */
char *
get_currentpath()
{
	return g_WorkingDir;
}

char *get_savespath()
{
   static char path[PATH_MAX];
   strcpy(path, get_currentpath());
   strcat(path, "save/");
   return path;
}

void
display_loading_progress( int p )
{
	char buf[300];

	if (!g_GuiEnabled)
	{
		printf( "Loading Rom %d%%\r", p );
		fflush( stdout );
		if (p == 100)
			printf( "\n" );
		return;
	}

	statusbar_message( "status", tr("Loading Rom: %d%%"), p );

	if( p == 100 )
	{
		statusbar_message( "status", tr("Rom \"%s\" loaded."), ROM_HEADER->nom );
		snprintf( buf, 300, NAME" v"VERSION" - %s", ROM_HEADER->nom );
		gtk_window_set_title( GTK_WINDOW(g_MainWindow.window), buf );
	}

	// update status bar
	while( g_main_iteration( FALSE ) );
}

void
display_MD5calculating_progress( int p )
{
}

static float VILimit = 60.0;
static double VILimitMilliseconds = 1000.0/60.0;

int GetVILimit()
{
    switch (ROM_HEADER->Country_code&0xFF)
    {
      case 0x44:
	   case 0x46:
	   case 0x49:
	   case 0x50:
	   case 0x53:
	   case 0x55:
	   case 0x58:
	   case 0x59:
	     return 50;
	     break;
	   case 0x37:
	   case 0x41:
	   case 0x45:
	   case 0x4a:
	     return 60;
	     break;  
	   default:
         return 60;  
    }
}

void InitTimer() {
   VILimit = GetVILimit();
   VILimitMilliseconds = (double) 1000.0/VILimit; 
	 printf("init timer!\n");
}

unsigned int gettimeofday_msec()
{
	struct timeval tv;
	unsigned int foo;
	
	gettimeofday(&tv, NULL);
	foo = ((tv.tv_sec % 1000000) * 1000) + (tv.tv_usec / 1000);
	//fprintf(stderr, "time: %d\n", foo);
	return foo;
}

void new_frame() {
/*
   unsigned int CurrentFPSTime,Dif;
   float FPS;
   static unsigned int LastFPSTime;
   static unsigned int CounterTime;
   static int Fps_Counter=0;

   if(Fps_Counter == 0)
     {
	LastFPSTime = gettimeofday_msec();
	CounterTime = gettimeofday_msec();
     }

   Fps_Counter++;

   CurrentFPSTime = gettimeofday_msec();
   Dif = CurrentFPSTime - LastFPSTime;
   if (Dif) {
      if (CurrentFPSTime - CounterTime > 1000 ) {
	 FPS = (float) (Fps_Counter * 1000.0 / (CurrentFPSTime - CounterTime));
	 CounterTime = CurrentFPSTime ;
	 Fps_Counter = 0;
      }
   }
   LastFPSTime = CurrentFPSTime ;*/
}

extern int m_currentVI;
void new_vi() {
   int Dif;
   unsigned int CurrentFPSTime;
   static unsigned int LastFPSTime = 0;
   static unsigned int CounterTime = 0;
   static unsigned int CalculatedTime ;
   static int VI_Counter = 0;
   long time;

   start_section(IDLE_SECTION);
   VI_Counter++;

	m_currentVI++;
	if(VCR_isRecording())
		VCR_setLengthVIs(m_currentVI/*VCR_getLengthVIs() + 1*/);

   if(LastFPSTime == 0)
     {
	LastFPSTime = gettimeofday_msec();
	CounterTime = gettimeofday_msec();
	return;
     }
   CurrentFPSTime = gettimeofday_msec();

   Dif = CurrentFPSTime - LastFPSTime;

   if (Dif <  VILimitMilliseconds ) {
      CalculatedTime = CounterTime + (double)VILimitMilliseconds * (double)VI_Counter;
      time = (int)(CalculatedTime - CurrentFPSTime);
      if (time>0) {
         usleep(time * 1000);
      }
      CurrentFPSTime = CurrentFPSTime + time;
   }


   if (CurrentFPSTime - CounterTime >= 1000.0 ) {
      CounterTime = gettimeofday_msec();
      VI_Counter = 0 ;
   }

   LastFPSTime = CurrentFPSTime ;
   end_section(IDLE_SECTION);
}

int
ask_bad()
{
	int button;

	if (!g_GuiEnabled)
	{
		printf( "The rom you are trying to load is probably a bad dump!\n"
		        "Be warned that this will probably give unexpected results.\n" );
		return 1;
	}

	button = messagebox( tr("Bad dump?"), MB_YESNO | MB_ICONQUESTION,
		   tr("The rom you are trying to load is probably a bad dump!\n"
		   "Be warned that this will probably give unexpected results.\n"
		   "Do you still want to run it?") );

	if( button == 1 ) // yes
		return 1;
	else
		return 0;
}

int
ask_hack()
{
	int button;

	if (!g_GuiEnabled)
	{
		printf( "The rom you are trying to load is probably a hack!\n"
		        "Be warned that this will probably give unexpected results.\n" );
		return 1;
	}

	button = messagebox( "Hack?", MB_YESNO | MB_ICONQUESTION,
		   tr("The rom you are trying to load is probably a hack!\n"
		   "Be warned that this will probably give unexpected results.\n"
		   "Do you still want to run it?") );

	if( button == 1 ) // yes
		return 1;
	else
		return 0;
}

void
warn_savestate_from_another_rom()
{

	if (!g_GuiEnabled)
	{
		printf( "You're trying to load a save state from either another rom\n"
		        "or another dump.\n" );
		return;
	}

	messagebox(tr("Wrong save state"), MB_OK,
		   tr("You're trying to load a save state from either another rom\n"
		      "or another dump.") );
}

void
warn_savestate_not_exist()
{
	if (!g_GuiEnabled)
	{
		printf( "The save state you're trying to load doesn't exist\n" );
		return;
	}

	messagebox(tr("Save state not exist"), MB_OK,
		   tr("The save state you're trying to load doesn't exist"));
}

void
display_status(const char* status)
{
	printf("%s\r", status);
}

/*********************************************************************************************************
 * internal gui funcs
 */
// status bar
void
statusbar_message( const char *section, const char *fmt, ... )
{
	va_list ap;
	char buf[2049];
	int i;

	va_start( ap, fmt );
	vsnprintf( buf, 2048, fmt, ap );
	va_end( ap );

	for( i = 0; statusBarSections[i].name; i++ )
		if( !strcasecmp( section, statusBarSections[i].name ) )
		{
			gtk_statusbar_pop( GTK_STATUSBAR(statusBarSections[i].bar), statusBarSections[i].id );
			gtk_statusbar_push( GTK_STATUSBAR(statusBarSections[i].bar), statusBarSections[i].id, buf );
			return;
		}

#ifdef _DEBUG
	printf( "statusbar_message(): unknown section '%s'!\n", section );
#endif
}

void
open_rom( const char *filename )
{
	if( g_EmulationThread )
	{
		if( messagebox( tr("Stop Emulation?"), MB_YESNO | MB_ICONQUESTION, tr("Emulation is running. Do you want to\nstop it and load the selected rom?") ) == 2 )
			return;
		callback_stopEmulation( NULL, NULL );
	}

	if( ROM_HEADER )
	{
		free( ROM_HEADER );
		ROM_HEADER = NULL;
	}
	if( rom )
	{
		free( rom );
		rom = NULL;
	}

	if( !fill_header( filename ) )
	{
		messagebox( tr("Error"), MB_OK | MB_ICONERROR, tr("Couldn't load Rom!") );
		return;
	}

	if( rom_read( filename ) )
	{
		messagebox( tr("Error"), MB_OK | MB_ICONERROR, tr("Couldn't load Rom!") );
		return;
	}
	InitTimer();
}

void
run_rom( void )
{
	callback_startEmulation( NULL, NULL );
}

/*********************************************************************************************************
 * callbacks
 */
/** rom **/
// open rom
static void
callback_openRomFileSelected( GtkWidget *widget, gpointer data )
{
	const gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

	gtk_widget_hide( GTK_WIDGET(data) );
	// really hide dialog (let gtk work)
	while( g_main_iteration( FALSE ) );

	open_rom( filename );
}

static void
callback_openRom( GtkWidget *widget, gpointer data )
{
	GtkWidget *file_selector;

	if( g_EmulationThread )
	{
		if( messagebox( tr("Stop Emulation?"), MB_YESNO | MB_ICONQUESTION, tr("Emulation is running. Do you want to\nstop it and load a rom?") ) == 2 )
			return;
		callback_stopEmulation( NULL, NULL );
	}

	/* Create the selector */
	file_selector = gtk_file_selection_new( tr("Open Rom...") );

	gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
				GTK_SIGNAL_FUNC(callback_openRomFileSelected), (gpointer)file_selector );

	/* Ensure that the dialog box is destroyed when the user clicks a button. */
	gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
					GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

	gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked",
					GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

	/* Display that dialog */
	gtk_widget_show( file_selector );
}

// close rom
static void
callback_closeRom( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		if( messagebox( tr("Stop Emulation?"), MB_YESNO | MB_ICONQUESTION, tr("Emulation is running. Do you want to\nstop it and close the rom?") ) == 2 )
			return;
		callback_stopEmulation( NULL, NULL );
	}

	if( ROM_HEADER )
	{
		free( ROM_HEADER );
		ROM_HEADER = NULL;
	}
	if( rom )
	{
		free( rom );
		rom = NULL;
	}

	statusbar_message( "status", tr("Rom closed.") );
	gtk_window_set_title( GTK_WINDOW(g_MainWindow.window), NAME" v"VERSION );
}

// language selected
static void
callback_languageSelected( GtkWidget *widget, gpointer data )
{
	const char *name;
	widget = data; // ToDo: find out why this is needed

	if( !GTK_CHECK_MENU_ITEM(widget)->active )
		return;
	name = gtk_object_get_data( GTK_OBJECT(widget), "lang name" );
	tr_set_language( name );
	config_put_string( "Language", name );

	// recreate gui
	gtk_signal_disconnect_by_func( GTK_OBJECT(g_MainWindow.window),
	                               GTK_SIGNAL_FUNC(gtk_main_quit), NULL );
	gtk_widget_destroy( g_MainWindow.window );
	gtk_widget_destroy( g_AboutDialog.dialog );
	gtk_widget_destroy( g_ConfigDialog.dialog );
	create_mainWindow();
	create_aboutDialog();
	create_configDialog();
	gtk_widget_show_all( g_MainWindow.window );
}

/** emulation **/
// start emulation
static void
callback_startEmulation( GtkWidget *widget, gpointer data )
{
	if( !g_EmulationThread )
	{
		// check if rom is loaded
		if( !rom )
		{
			if( messagebox( tr("No Rom Loaded"), MB_YESNO | MB_ICONQUESTION, tr("There is no Rom loaded.\nDo you want to load one?") ) == 1 )
				callback_openRom( NULL, NULL );
			return;
		}

		// spawn emulation thread
		if( pthread_create( &g_EmulationThread, NULL, emulationThread, NULL ) )
		{
			g_EmulationThread = 0;
			messagebox( tr("Error"), MB_OK | MB_ICONERROR, tr("Couldn't spawn core thread!") );
			return;
		}
		pthread_detach( g_EmulationThread );
		statusbar_message( "status", tr("Emulation started (PID: %d)"), g_EmulationThread );
	}
}

// pause/continue emulation
static void
callback_pauseContinueEmulation( GtkWidget *widget, gpointer data )
{
	if( !g_EmulationThread )
		return;

	if( g_EmulationRunning )
	{
		if( pthread_kill( g_EmulationThread, SIGSTOP ) )
		{
			messagebox( tr("Error"), MB_OK | MB_ICONERROR, tr("Couldn't pause core thread: %s!"), strerror( errno ) );
			return;
		}
		g_EmulationRunning = 0;
		statusbar_message( "status", tr("Emulation paused.") );
	}
	else
	{
		if( pthread_kill( g_EmulationThread, SIGCONT ) )
		{
			messagebox( tr("Error"), MB_OK | MB_ICONERROR, tr("Couldn't continue core thread: %s!"), strerror( errno ) );
			return;
		}
		g_EmulationRunning = 1;
		statusbar_message( "status", tr("Emulation continued.") );
	}
}

// stop emulation
static void
callback_stopEmulation( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		statusbar_message( "status", tr("Stopping emulation.") );
		if( !g_EmulationRunning )
			pthread_kill( g_EmulationThread, SIGCONT );
		stop_it();
		while( g_EmulationThread )
			if( gtk_main_iteration() )
				break;
		statusbar_message( "status", tr("Emulation stopped.") );
	}
}

// Save State
static void
callback_Save( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
		savestates_job |= SAVESTATE;
}

// Save As
static void
callback_saveAsFileSelected( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		const gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		savestates_select_filename( filename );
		savestates_job |= SAVESTATE;
	}
}

static void
callback_SaveAs( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		GtkWidget *file_selector;

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Save as...") );

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
				GTK_SIGNAL_FUNC(callback_saveAsFileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
					GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked",
					GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
}

// Restore
static void
callback_Restore( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
		savestates_job |= LOADSTATE;
}

// Load
static void
callback_loadFileSelected( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		const gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		savestates_select_filename( filename );
		savestates_job |= LOADSTATE;
	}
}

static void
callback_Load( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		GtkWidget *file_selector;

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Load...") );

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
				GTK_SIGNAL_FUNC(callback_loadFileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
					GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked",
					GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
}

/** Slot **/
static void
callback_Default( GtkWidget *widget, gpointer data )
{
	savestates_select_slot( 0 );
}

static void
callback_slot( GtkWidget *widget, gpointer data )
{
	data = widget; // dunno why this is necessary
	savestates_select_slot( (int)data );
}

/** configuration **/
// configure
static void
callback_configure( GtkWidget *widget, gpointer data )
{
	if( !g_EmulationRunning )
		gtk_widget_show_all( g_ConfigDialog.dialog );
}

// configure gfx
static void
callback_configureVideo( GtkWidget *widget, gpointer data )
{
	/*if( !g_EmulationRunning )
	{*/
		char *name;

		name = plugin_name_by_filename( config_get_string( "Gfx Plugin", "" ) );
		if( name )
			plugin_exec_config( name );
		else
			if( messagebox( tr("No plugin!"), MB_YESNO | MB_ICONQUESTION, tr("No graphics plugin selected! Do you\nwant to select one?") ) == 1 )
			{
				gtk_widget_show_all( g_ConfigDialog.dialog );
				gtk_notebook_set_page( GTK_NOTEBOOK(g_ConfigDialog.notebook),
										gtk_notebook_page_num( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configPlugins ) );
			}
	/*}*/
}

// configure audio
static void
callback_configureAudio( GtkWidget *widget, gpointer data )
{
	/*if( !g_EmulationRunning )
	{*/
		char *name;

		name = plugin_name_by_filename( config_get_string( "Audio Plugin", "" ) );
		if( name )
			plugin_exec_config( name );
		else
			if( messagebox( tr("No plugin!"), MB_YESNO | MB_ICONQUESTION, tr("No audio plugin selected! Do you\nwant to select one?") ) == 1 )
			{
				gtk_widget_show_all( g_ConfigDialog.dialog );
				gtk_notebook_set_page( GTK_NOTEBOOK(g_ConfigDialog.notebook),
										gtk_notebook_page_num( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configPlugins ) );
			}
	/*}*/
}

// configure input
static void
callback_configureInput( GtkWidget *widget, gpointer data )
{
	/*if( !g_EmulationRunning )
	{*/
		char *name;

		name = plugin_name_by_filename( config_get_string( "Input Plugin", "" ) );
		if( name )
			plugin_exec_config( name );
		else
			if( messagebox( tr("No plugin!"), MB_YESNO | MB_ICONQUESTION, tr("No input plugin selected! Do you\nwant to select one?") ) == 1 )
			{
				gtk_widget_show_all( g_ConfigDialog.dialog );
				gtk_notebook_set_page( GTK_NOTEBOOK(g_ConfigDialog.notebook),
										gtk_notebook_page_num( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configPlugins ) );
			}
	/*}*/
}

// configure RSP
static void
callback_configureRSP( GtkWidget *widget, gpointer data )
{
	/*if( !g_EmulationRunning )
	{*/
		char *name;

		name = plugin_name_by_filename( config_get_string( "RSP Plugin", "" ) );
		if( name )
			plugin_exec_config( name );
		else
			if( messagebox( tr("No plugin!"), MB_YESNO | MB_ICONQUESTION, tr("No RSP plugin selected! Do you\nwant to select one?") ) == 1 )
			{
				gtk_widget_show_all( g_ConfigDialog.dialog );
				gtk_notebook_set_page( GTK_NOTEBOOK(g_ConfigDialog.notebook),
										gtk_notebook_page_num( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configPlugins ) );
			}
	/*}*/
}

// full screen
static void
callback_fullScreen( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		changeWindow();
	}
}


/** VCR **/
#ifdef VCR_SUPPORT
static void
callback_vcrStartRecord_fileSelected( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		if (VCR_startRecord( filename, false, NULL, NULL ) < 0)
			messagebox( tr( "VCR" ), MB_ICONERROR | MB_OK,
			            tr( "Couldn't start recording." ) );
	}
}


static void
callback_vcrStartRecord( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		GtkWidget *file_selector;

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Save .rec file") );

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
				GTK_SIGNAL_FUNC(callback_vcrStartRecord_fileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
					GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked",
					GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
	// else maybe display messagebox
}


static void
callback_vcrStopRecord( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		if (VCR_stopRecord() < 0)
			messagebox( tr( "VCR" ), MB_ICONERROR | MB_OK,
			            tr( "Couldn't stop recording." ) );
	}
}


static void
callback_vcrStartPlayback_fileSelected( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		if (VCR_startPlayback( filename, NULL, NULL ) < 0)
			messagebox( tr( "VCR" ), MB_ICONERROR | MB_OK,
			            tr( "Couldn't start playback." ) );
	}
}


static void
callback_vcrStartPlayback( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		GtkWidget *file_selector;

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Load .rec file") );

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
				GTK_SIGNAL_FUNC(callback_vcrStartPlayback_fileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
					GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked",
					GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
	// else maybe display messagebox
}


static void
callback_vcrStopPlayback( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		if (VCR_stopPlayback() < 0)
			messagebox( tr( "VCR" ), MB_ICONERROR | MB_OK,
			            tr( "Couldn't stop playback." ) );
	}
}


static char m_startCaptureRecFilename[PATH_MAX];

static void
callback_vcrStartCapture_aviFileSelected( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		if (VCR_startCapture( m_startCaptureRecFilename, filename ) < 0)
			messagebox( tr( "VCR" ), MB_ICONERROR | MB_OK,
			            tr( "Couldn't start capturing." ) );
	}
}

static void
callback_vcrStartCapture_recFileSelected( GtkWidget *widget, gpointer data )
{
	GtkWidget *file_selector;

	if( g_EmulationThread )
	{
		gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		strncpy( m_startCaptureRecFilename, filename, PATH_MAX );

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Save .avi file") );

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
				GTK_SIGNAL_FUNC(callback_vcrStartCapture_aviFileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
					GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked",
					GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
}


static void
callback_vcrStartCapture( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		GtkWidget *file_selector;

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Load .rec file") );

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
				GTK_SIGNAL_FUNC(callback_vcrStartCapture_recFileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
					GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked",
					GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
	// else maybe display messagebox
}


static void
callback_vcrStopCapture( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		if (VCR_stopCapture() < 0)
			messagebox( tr( "VCR" ), MB_ICONERROR | MB_OK,
			            tr( "Couldn't stop capturing." ) );
	}
}


static void
callback_vcrSetup( GtkWidget *widget, gpointer data )
{
	gtk_widget_show_all( g_VcrCompDialog.dialog );
}
#endif // VCR_SUPPORT

/** debugger **/
#ifdef DBG
// show
static void
callback_debuggerEnableToggled( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		if( messagebox( tr("Restart Emulation?"), MB_YESNO | MB_ICONQUESTION, tr("Emulation needs to be restarted in order\nto activate the debugger. Do you want\nthis to happen?") ) == 1 )
		{
			callback_stopEmulation( NULL, NULL );
			g_DebuggerEnabled = GTK_CHECK_MENU_ITEM(widget)->active;
			callback_startEmulation( NULL, NULL );
		}
		return;
	}

	g_DebuggerEnabled = GTK_CHECK_MENU_ITEM(widget)->active;
}
#endif // DBG

/** help **/
// about
static void
callback_aboutMupen( GtkWidget *widget, gpointer data )
{
	if( !g_EmulationRunning )
		gtk_widget_show_all( g_AboutDialog.dialog );
}

/** misc **/
// hide on delete
static gint
callback_mainWindowDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	int i, w, h;

	gtk_widget_hide( widget );

	// save configuration
	w = g_MainWindow.window->allocation.width;
	h = g_MainWindow.window->allocation.height;
	if (w != 0 && h != 0)
	{
		config_put_number( "MainWindow Width", w );
		config_put_number( "MainWindow Height", h );
	}
	for( i = 0; i < 5; i++ )
	{
		w = gtk_clist_get_column_widget( GTK_CLIST(g_MainWindow.romCList), i )->allocation.width;
		if (w != 0)
		{
			char buf[30];
			sprintf( buf, "RomBrowser ColWidth[%d]", i );
			config_put_number( buf, w );
		}
	}

	config_write();


	gtk_main_quit();

	return TRUE; // undeleteable
}

/*********************************************************************************************************
 * gui creation
 */
// create a menu item with an accelerator
static GtkWidget *
menu_item_new_with_accelerator( GtkAccelGroup *group, const char *label )
{
	GtkWidget *item;
	gint key;

	item = gtk_menu_item_new_with_label( "" );
	key = gtk_label_parse_uline( GTK_LABEL(GTK_BIN(item)->child), label );
//  gtk_widget_add_accelerator( item, "activate_item", group, key, GDK_MOD1_MASK, 0 );

	return item;
}

// static widgets to change their state from emulation thread
static GtkWidget       *slotDefaultItem;
static GtkWidget       *slotItem[9];

// menuBar
static int
create_menuBar( void )
{
	GtkWidget	*fileMenu;
	GtkWidget	*fileMenuItem;
	GtkWidget	*emulationMenu;
	GtkWidget	*emulationMenuItem;
	GtkWidget	*optionsMenu;
	GtkWidget	*optionsMenuItem;
#ifdef VCR_SUPPORT
	GtkWidget	*vcrMenu;
	GtkWidget	*vcrMenuItem;
#endif
#ifdef DBG
	GtkWidget	*debuggerMenu;
	GtkWidget	*debuggerMenuItem;
#endif
	GtkWidget	*helpMenu;
	GtkWidget	*helpMenuItem;

	GtkWidget	*fileLoadRomItem;
	GtkWidget	*fileCloseRomItem;
	GtkWidget	*fileSeparator1;
	GtkWidget	*fileLanguageItem;
	GtkWidget	*fileLanguageMenu;
	GtkWidget	*fileSeparator2;
	GtkWidget	*fileExitItem;

	GtkWidget	*emulationStartItem;
	GtkWidget	*emulationPauseContinueItem;
	GtkWidget	*emulationStopItem;
	GtkWidget	*emulationSeparator1;
	GtkWidget	*emulationSaveItem;
	GtkWidget	*emulationSaveAsItem;
	GtkWidget	*emulationRestoreItem;
	GtkWidget	*emulationLoadItem;
	GtkWidget	*emulationSeparator2;
	GtkWidget	*emulationSlotItem;
	GtkWidget	*emulationSlotMenu;

	GtkWidget	*slotSeparator;

	GtkWidget	*optionsConfigureItem;
	GtkWidget	*optionsSeparator1;
	GtkWidget	*optionsVideoItem;
	GtkWidget	*optionsAudioItem;
	GtkWidget	*optionsInputItem;
	GtkWidget	*optionsRSPItem;
	GtkWidget	*optionsSeparator2;
	GtkWidget	*optionsFullScreenItem;

#ifdef VCR_SUPPORT
	GtkWidget	*vcrStartRecordItem;
	GtkWidget	*vcrStopRecordItem;
	GtkWidget	*vcrSeparator1;
	GtkWidget	*vcrStartPlaybackItem;
	GtkWidget	*vcrStopPlaybackItem;
	GtkWidget	*vcrSeparator2;
	GtkWidget	*vcrStartCaptureItem;
	GtkWidget	*vcrStopCaptureItem;
	GtkWidget	*vcrSeparator3;
	GtkWidget	*vcrSetupItem;
#endif

#ifdef DBG
	GtkWidget	*debuggerEnableItem;
#endif

	GtkWidget	*helpAboutItem;

	GtkAccelGroup *menuAccelGroup;
	GtkAccelGroup *fileAccelGroup;
	GtkAccelGroup *emulationAccelGroup;
	GtkAccelGroup *optionsAccelGroup;
#ifdef VCR_SUPPORT
	GtkAccelGroup *vcrAccelGroup;
#endif
#ifdef DBG
	GtkAccelGroup *debuggerAccelGroup;
#endif
	GtkAccelGroup *helpAccelGroup;

	GSList *group = NULL;
	GSList *slot_group = NULL;
	GList *langList;
	char *language;
	int i, lang_found;
	char buffer[1000];

	// menubar
	g_MainWindow.menuBar = gtk_menu_bar_new();
	gtk_box_pack_start( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.menuBar, FALSE, FALSE, 0 );
	menuAccelGroup = gtk_accel_group_new();

	// file menu
	fileAccelGroup = gtk_accel_group_new();
	fileMenu = gtk_menu_new();
	fileMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_File") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(fileMenuItem), fileMenu );
	fileLoadRomItem = menu_item_new_with_accelerator( fileAccelGroup, tr("_Open Rom...") );
	fileCloseRomItem = menu_item_new_with_accelerator( fileAccelGroup, tr("_Close Rom") );
	fileSeparator1 = gtk_menu_item_new();
	fileLanguageItem = menu_item_new_with_accelerator( fileAccelGroup, tr("_Language") );
#ifndef _GTK2
	gtk_menu_item_configure( GTK_MENU_ITEM( fileLanguageItem ), 0, 1 );
#endif
	fileSeparator2 = gtk_menu_item_new();
	fileExitItem = menu_item_new_with_accelerator( fileAccelGroup, tr("_Exit") );
	gtk_menu_append( GTK_MENU(fileMenu), fileLoadRomItem );
	gtk_menu_append( GTK_MENU(fileMenu), fileCloseRomItem );
	gtk_menu_append( GTK_MENU(fileMenu), fileSeparator1 );
	gtk_menu_append( GTK_MENU(fileMenu), fileLanguageItem );
	gtk_menu_append( GTK_MENU(fileMenu), fileSeparator2 );
	gtk_menu_append( GTK_MENU(fileMenu), fileExitItem );

	gtk_signal_connect_object( GTK_OBJECT(fileLoadRomItem), "activate",
			GTK_SIGNAL_FUNC(callback_openRom), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(fileCloseRomItem), "activate",
			GTK_SIGNAL_FUNC(callback_closeRom), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(fileExitItem), "activate",
			GTK_SIGNAL_FUNC(gtk_main_quit), (gpointer)NULL );

	// language menu
	fileLanguageMenu = gtk_menu_new();
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(fileLanguageItem), fileLanguageMenu );

	langList = tr_language_list();
	lang_found = 0;
	for( i = 0; i < g_list_length( langList ); i++ )
	{
		const char *confLang = config_get_string( "Language", "English" );
		language = g_list_nth_data( langList, i );
		fileLanguageItem = gtk_radio_menu_item_new_with_label( group, language );
		gtk_object_set_data( GTK_OBJECT(fileLanguageItem), "lang name", language );
		group = gtk_radio_menu_item_group( GTK_RADIO_MENU_ITEM(fileLanguageItem) );
		if( !strcasecmp( language, confLang ) )
		{
			gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(fileLanguageItem), 1 );
		        tr_set_language(language);
			lang_found = 1;
		}
	        if (!lang_found && !strcasecmp( language, "English" ))
	                gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(fileLanguageItem), 1 );

		gtk_signal_connect_object( GTK_OBJECT(fileLanguageItem), "toggled",
				GTK_SIGNAL_FUNC(callback_languageSelected), (gpointer)NULL );
		gtk_menu_append( GTK_MENU(fileLanguageMenu), fileLanguageItem );
	}
	if( !lang_found )
	{
		tr_set_language( "English" );
		// ToDo: update selected radio menu item
	}

	// emulation menu
	emulationAccelGroup = gtk_accel_group_new();
	emulationMenu = gtk_menu_new();
	emulationMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_Emulation") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(emulationMenuItem), emulationMenu );
	emulationStartItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("_Start") );
	emulationPauseContinueItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Pause/_Continue") );
	emulationStopItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Sto_p") );
	emulationSeparator1 = gtk_menu_item_new();
	emulationSaveItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Save State") );
	emulationSaveAsItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Save State As") );
	emulationRestoreItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Restore State") );
	emulationLoadItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Load State") );
	emulationSeparator2 = gtk_menu_item_new();
	emulationSlotItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Current save state") );

	gtk_menu_append( GTK_MENU(emulationMenu), emulationStartItem );
	gtk_menu_append( GTK_MENU(emulationMenu), emulationPauseContinueItem );
	gtk_menu_append( GTK_MENU(emulationMenu), emulationStopItem );
	gtk_menu_append( GTK_MENU(emulationMenu), emulationSeparator1 );
	gtk_menu_append( GTK_MENU(emulationMenu), emulationSaveItem );
	gtk_menu_append( GTK_MENU(emulationMenu), emulationSaveAsItem );
	gtk_menu_append( GTK_MENU(emulationMenu), emulationRestoreItem );
	gtk_menu_append( GTK_MENU(emulationMenu), emulationLoadItem );
	gtk_menu_append( GTK_MENU(emulationMenu), emulationSeparator2 );
	gtk_menu_append( GTK_MENU(emulationMenu), emulationSlotItem);

	gtk_signal_connect_object( GTK_OBJECT(emulationStartItem), "activate",
			GTK_SIGNAL_FUNC(callback_startEmulation), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationPauseContinueItem), "activate",
			GTK_SIGNAL_FUNC(callback_pauseContinueEmulation), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationStopItem), "activate",
			GTK_SIGNAL_FUNC(callback_stopEmulation), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationSaveItem), "activate",
			GTK_SIGNAL_FUNC(callback_Save), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationSaveAsItem), "activate",
			GTK_SIGNAL_FUNC(callback_SaveAs), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationRestoreItem), "activate",
			GTK_SIGNAL_FUNC(callback_Restore), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationLoadItem), "activate",
			GTK_SIGNAL_FUNC(callback_Load), (gpointer)NULL );

	// slot menu
	emulationSlotMenu = gtk_menu_new();
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(emulationSlotItem), emulationSlotMenu );
	slotDefaultItem = gtk_radio_menu_item_new_with_label( slot_group, tr("Default") );
	slot_group = gtk_radio_menu_item_group( GTK_RADIO_MENU_ITEM(slotDefaultItem) );
	slotSeparator = gtk_menu_item_new();
	for (i = 0; i < 9; i++)
	{
		snprintf( buffer, 1000, tr( "Slot %d" ), i+1 );
		slotItem[i] = gtk_radio_menu_item_new_with_label( slot_group, buffer );
		slot_group = gtk_radio_menu_item_group( GTK_RADIO_MENU_ITEM(slotItem[i]) );
	}

	gtk_menu_append( GTK_MENU(emulationSlotMenu), slotDefaultItem );
	gtk_menu_append( GTK_MENU(emulationSlotMenu), slotSeparator );
	for (i = 0; i < 9; i++)
		gtk_menu_append( GTK_MENU(emulationSlotMenu), slotItem[i] );

	gtk_signal_connect_object( GTK_OBJECT(slotDefaultItem), "activate",
			GTK_SIGNAL_FUNC(callback_Default), (gpointer)NULL );

	for (i = 0; i < 9; i++)
		gtk_signal_connect_object( GTK_OBJECT(slotItem[i]), "activate",
				GTK_SIGNAL_FUNC(callback_slot), (gpointer)i );

	// options menu
	optionsMenu = gtk_menu_new();
	optionsMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_Options") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(optionsMenuItem), optionsMenu );
	optionsConfigureItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_Configure...") );
	optionsSeparator1 = gtk_menu_item_new();
	optionsVideoItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_Video Settings...") );
	optionsAudioItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_Audio Settings...") );
	optionsInputItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_Input Settings...") );
	optionsRSPItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_RSP Settings...") );
	optionsSeparator2 = gtk_menu_item_new();
	optionsFullScreenItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_Full Screen") );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsConfigureItem );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsSeparator1 );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsVideoItem );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsAudioItem );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsInputItem );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsRSPItem );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsSeparator2 );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsFullScreenItem );

	gtk_signal_connect_object( GTK_OBJECT(optionsConfigureItem), "activate",
			GTK_SIGNAL_FUNC(callback_configure), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(optionsVideoItem), "activate",
			GTK_SIGNAL_FUNC(callback_configureVideo), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(optionsAudioItem), "activate",
			GTK_SIGNAL_FUNC(callback_configureAudio), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(optionsInputItem), "activate",
			GTK_SIGNAL_FUNC(callback_configureInput), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(optionsRSPItem), "activate",
			GTK_SIGNAL_FUNC(callback_configureRSP), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(optionsFullScreenItem), "activate",
			GTK_SIGNAL_FUNC(callback_fullScreen), (gpointer)NULL );

	// vcr menu
#ifdef VCR_SUPPORT
	vcrAccelGroup = gtk_accel_group_new();
	vcrMenu = gtk_menu_new();
	vcrMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_VCR") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(vcrMenuItem), vcrMenu );
	vcrStartRecordItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Start _Record...") );
	vcrStopRecordItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Stop Record") );
	vcrSeparator1 = gtk_menu_item_new();
	vcrStartPlaybackItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Start _Playback...") );
	vcrStopPlaybackItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Stop Playback") );
	vcrSeparator2 = gtk_menu_item_new();
	vcrStartCaptureItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Start _Capture...") );
	vcrStopCaptureItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Stop Capture") );
	vcrSeparator3 = gtk_menu_item_new();
	vcrSetupItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Configure Codec...") );

	gtk_menu_append( GTK_MENU(vcrMenu), vcrStartRecordItem );
	gtk_menu_append( GTK_MENU(vcrMenu), vcrStopRecordItem );
	gtk_menu_append( GTK_MENU(vcrMenu), vcrSeparator1 );
	gtk_menu_append( GTK_MENU(vcrMenu), vcrStartPlaybackItem );
	gtk_menu_append( GTK_MENU(vcrMenu), vcrStopPlaybackItem );
	gtk_menu_append( GTK_MENU(vcrMenu), vcrSeparator2 );
	gtk_menu_append( GTK_MENU(vcrMenu), vcrStartCaptureItem );
	gtk_menu_append( GTK_MENU(vcrMenu), vcrStopCaptureItem );
	gtk_menu_append( GTK_MENU(vcrMenu), vcrSeparator3 );
	gtk_menu_append( GTK_MENU(vcrMenu), vcrSetupItem );

	gtk_signal_connect_object( GTK_OBJECT(vcrStartRecordItem), "activate",
			GTK_SIGNAL_FUNC(callback_vcrStartRecord), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(vcrStopRecordItem), "activate",
			GTK_SIGNAL_FUNC(callback_vcrStopRecord), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(vcrStartPlaybackItem), "activate",
			GTK_SIGNAL_FUNC(callback_vcrStartPlayback), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(vcrStopPlaybackItem), "activate",
			GTK_SIGNAL_FUNC(callback_vcrStopPlayback), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(vcrStartCaptureItem), "activate",
			GTK_SIGNAL_FUNC(callback_vcrStartCapture), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(vcrStopCaptureItem), "activate",
			GTK_SIGNAL_FUNC(callback_vcrStopCapture), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(vcrSetupItem), "activate",
			GTK_SIGNAL_FUNC(callback_vcrSetup), (gpointer)NULL );
#endif // VCR_SUPPORT

	// debugger menu
#ifdef DBG
	debuggerAccelGroup = gtk_accel_group_new();
	debuggerMenu = gtk_menu_new();
	debuggerMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_Debugger") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(debuggerMenuItem), debuggerMenu );
	debuggerEnableItem = menu_item_new_with_accelerator( debuggerAccelGroup, tr("_Enable") );
	gtk_menu_append( GTK_MENU(debuggerMenu), debuggerEnableItem );

	gtk_signal_connect_object( GTK_OBJECT(debuggerEnableItem), "toggled",
			GTK_SIGNAL_FUNC(callback_debuggerEnableToggled), (gpointer)NULL );
#endif // DBG

	// help menu
	helpAccelGroup = gtk_accel_group_new();
	helpMenu = gtk_menu_new();
	helpMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_Help") );
	gtk_menu_item_right_justify( GTK_MENU_ITEM(helpMenuItem) );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(helpMenuItem), helpMenu );
	helpAboutItem = menu_item_new_with_accelerator( helpAccelGroup, tr("_About...") );
	gtk_menu_append( GTK_MENU(helpMenu), helpAboutItem );

	gtk_signal_connect_object( GTK_OBJECT(helpAboutItem), "activate",
			GTK_SIGNAL_FUNC(callback_aboutMupen), (gpointer)NULL );

	// add menus to menubar
	gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), fileMenuItem );
	gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), emulationMenuItem );
	gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), optionsMenuItem );
#ifdef VCR_SUPPORT
	gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), vcrMenuItem );
#endif
#ifdef DBG
	gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), debuggerMenuItem );
#endif
	gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), helpMenuItem );

	// add accelerators to window
/*	gtk_menu_set_accel_group( GTK_MENU(fileMenu), menuAccelGroup );
	gtk_menu_set_accel_group( GTK_MENU(fileMenu), fileAccelGroup );
	gtk_menu_set_accel_group( GTK_MENU(emulationMenu), emulationAccelGroup );
	gtk_menu_set_accel_group( GTK_MENU(optionsMenu), optionsAccelGroup );
#ifdef VCR_SUPPORT
	gtk_menu_set_accel_group( GTK_MENU(vcrMenu), vcrAccelGroup );
#endif
#ifdef DBG
	gtk_menu_set_accel_group( GTK_MENU(debuggerMenu), debuggerAccelGroup );
#endif
	gtk_menu_set_accel_group( GTK_MENU(helpMenu), helpAccelGroup );*/

	return 0;
}

// toolbar
static int
create_toolBar( void )
{
	GtkWidget	*openPixmap;
	GtkWidget	*playPixmap;
	GtkWidget	*pausePixmap;
	GtkWidget	*stopPixmap;
	GtkWidget	*fullscreenPixmap;
	GtkWidget	*configurePixmap;

#ifdef _GTK2
	g_MainWindow.toolBar = gtk_toolbar_new();
	gtk_toolbar_set_orientation( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_ORIENTATION_HORIZONTAL );
	gtk_toolbar_set_style( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_ICONS );
	gtk_toolbar_set_tooltips( GTK_TOOLBAR(g_MainWindow.toolBar), TRUE );
#else
	g_MainWindow.toolBar = gtk_toolbar_new( GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_ICONS );
	gtk_toolbar_set_button_relief( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_RELIEF_NONE );
	gtk_toolbar_set_space_style( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_SPACE_LINE );
#endif
	gtk_box_pack_start( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.toolBar, FALSE, FALSE, 0 );

	// load icons from memory
	openPixmap = create_pixmap_d( g_MainWindow.window, open_xpm );
	playPixmap = create_pixmap_d( g_MainWindow.window, play_xpm );
	pausePixmap = create_pixmap_d( g_MainWindow.window, pause_xpm );
	stopPixmap = create_pixmap_d( g_MainWindow.window, stop_xpm );
	fullscreenPixmap = create_pixmap_d( g_MainWindow.window, fullscreen_xpm );
	configurePixmap = create_pixmap_d( g_MainWindow.window, configure_xpm );

	// add icons to toolbar
	gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),
				NULL,
				tr("Open Rom"),
				"",
				openPixmap,
				GTK_SIGNAL_FUNC(callback_openRom),
				NULL );
	gtk_toolbar_append_space( GTK_TOOLBAR(g_MainWindow.toolBar) );
	gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),
				NULL,
				tr("Start Emulation"),
				"",
				playPixmap,
				GTK_SIGNAL_FUNC(callback_startEmulation),
				NULL );
	gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),
				NULL,
				tr("Pause/continue Emulation"),
				"",
				pausePixmap,
				GTK_SIGNAL_FUNC(callback_pauseContinueEmulation),
				NULL );
	gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),
				NULL,
				tr("Stop Emulation"),
				"",
				stopPixmap,
				GTK_SIGNAL_FUNC(callback_stopEmulation),
				NULL );
	gtk_toolbar_append_space( GTK_TOOLBAR(g_MainWindow.toolBar) );
	gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),
				NULL,
				tr("Fullscreen"),
				"",
				fullscreenPixmap,
				GTK_SIGNAL_FUNC(callback_fullScreen),
				NULL );
	gtk_toolbar_append_space( GTK_TOOLBAR(g_MainWindow.toolBar) );
	gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),
				NULL,
				tr("Configure"),
				"",
				configurePixmap,
				GTK_SIGNAL_FUNC(callback_configure),
				NULL );

	return 0;
}

// status bar
static int
create_statusBar( void )
{
	int i;

	// create status bar
	g_MainWindow.statusBarHBox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.statusBarHBox, FALSE, FALSE, 0 );

	// request context ids
	for( i = 0; statusBarSections[i].name; i++ )
	{
		statusBarSections[i].bar = gtk_statusbar_new();
		gtk_box_pack_start( GTK_BOX(g_MainWindow.statusBarHBox), statusBarSections[i].bar, (i == 0) ? TRUE : FALSE, TRUE, 0 );
		statusBarSections[i].id = gtk_statusbar_get_context_id( GTK_STATUSBAR(statusBarSections[i].bar), statusBarSections[i].name );
	}

	return 0;
}

// main window
static int
create_mainWindow( void )
{
	int width, height;

	width = config_get_number( "MainWindow Width", 600 );
	height = config_get_number( "MainWindow Height", 400 );

	// window
	g_MainWindow.window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW(g_MainWindow.window), NAME" v"VERSION );
	gtk_window_set_default_size( GTK_WINDOW(g_MainWindow.window), width, height  );
	gtk_signal_connect(GTK_OBJECT(g_MainWindow.window), "delete_event",
				GTK_SIGNAL_FUNC(callback_mainWindowDeleteEvent), (gpointer)NULL );
//	gtk_signal_connect( GTK_OBJECT(g_MainWindow.window), "destroy",
//				GTK_SIGNAL_FUNC(gtk_main_quit), NULL );

	// toplevel vbox
	g_MainWindow.toplevelVBox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER(g_MainWindow.window), g_MainWindow.toplevelVBox );

	// menu
	create_menuBar();

	// toolbar
	create_toolBar();

	// rombrowser
	create_romBrowser();

	// rom properties
	create_romPropDialog();

	// statusbar
	create_statusBar();

	// fill rom browser
	rombrowser_refresh();
	return 0;
}

/*********************************************************************************************************
 * sdl event filter
 */
static int
sdl_event_filter( const SDL_Event *event )
{
	switch( event->type )
	{
	case SDL_KEYDOWN:
		switch( event->key.keysym.sym )
		{
		case SDLK_F5:
			savestates_job |= SAVESTATE;
			break;

		case SDLK_F7:
			savestates_job |= LOADSTATE;
			break;

		case SDLK_ESCAPE:
			stop_it();
			break;

		case SDLK_F1:
			changeWindow();
			break;

		default:
			switch (event->key.keysym.unicode)
			{
			case '0':
				if (g_GuiEnabled)
					gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(slotDefaultItem), TRUE );
				else
					printf( "Using default save slot...\n" );
				savestates_select_slot( 0 );
				break;

			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (g_GuiEnabled)
					gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(slotItem[event->key.keysym.unicode - '1']), TRUE );
				else
					printf( "Using save slot %d...\n", event->key.keysym.unicode - '0' );
				savestates_select_slot( event->key.keysym.unicode - '0' );
				break;

			default:
				keyDown( 0, event->key.keysym.sym );
			}
		}
		return 0;
		break;

	case SDL_KEYUP:
		switch( event->key.keysym.sym )
		{
		case SDLK_ESCAPE:
			break;

		case SDLK_F1:
			break;
		default:
			keyUp( 0, event->key.keysym.sym );
		}
		return 0;
		break;
	}

	return 1;
}

/*********************************************************************************************************
 * emulation thread - runs the core
 */
static void *
emulationThread( void *_arg )
{
	const char *gfx_plugin, *audio_plugin, *input_plugin, *RSP_plugin;
	struct sigaction sa;

	// install signal handler
	memset( &sa, 0, sizeof( struct sigaction ) );
	sa.sa_sigaction = sighandler;
	sa.sa_flags = SA_SIGINFO;
	sigaction( SIGSEGV, &sa, NULL );
	sigaction( SIGILL, &sa, NULL );
	sigaction( SIGFPE, &sa, NULL );
	sigaction( SIGCHLD, &sa, NULL );

	dynacore = config_get_number( "Core", CORE_DYNAREC );
	g_EmulationRunning = 1;

        no_audio_delay = config_get_bool("NoAudioDelay", FALSE);
        no_compiled_jump = config_get_bool("NoCompiledJump", FALSE);
   
	// init sdl
	SDL_Init( SDL_INIT_VIDEO );
//	SDL_SetVideoMode( 10, 10, 0, 0 );
	SDL_ShowCursor( 0 );
	SDL_EnableKeyRepeat( 0, 0 );

	SDL_SetEventFilter( sdl_event_filter );
	SDL_EnableUNICODE(1);

	// plugins
	if (g_GuiEnabled)
	{
		gfx_plugin = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.gfxCombo)->entry) );
		audio_plugin = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.audioCombo)->entry) );
		input_plugin = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.inputCombo)->entry) );
		RSP_plugin = gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(g_ConfigDialog.RSPCombo)->entry) );
	}
	else
	{
		gfx_plugin = plugin_name_by_filename( config_get_string( "Gfx Plugin", "" ) );
		audio_plugin = plugin_name_by_filename( config_get_string( "Audio Plugin", "" ) );
		input_plugin = plugin_name_by_filename( config_get_string( "Input Plugin", "" ) );
		RSP_plugin = plugin_name_by_filename( config_get_string( "RSP Plugin", "" ) );
	}

	// run core
	init_memory();
	plugin_load_plugins( gfx_plugin, audio_plugin, input_plugin, RSP_plugin );
	romOpen_gfx();
	romOpen_audio();
	romOpen_input();

	if (g_Fullscreen)
		changeWindow();

#ifdef DBG
	if( g_DebuggerEnabled )
		init_debugger();
#endif
	if (!g_GuiEnabled && g_Fullscreen)
		changeWindow();
	go();	/* core func */
	romClosed_RSP();
	romClosed_input();
	romClosed_audio();
	romClosed_gfx();
	closeDLL_RSP();
	closeDLL_input();
	closeDLL_audio();
	closeDLL_gfx();
	free_memory();

	// clean up
	g_EmulationRunning = 0;
	g_EmulationThread = 0;

	SDL_Quit();

	if (g_Filename != 0)
	{
		// the following doesn't work - it wouldn't exit immediately but when the next event is
		// recieved (i.e. mouse movement)
/*		gdk_threads_enter();
		gtk_main_quit();
		gdk_threads_leave();*/
	}

	return NULL;
}

/*********************************************************************************************************
 * signal handler
 */
static void
sighandler( int signal, siginfo_t *info, void *context )
{
	if( info->si_pid == g_EmulationThread )
	{
		switch( signal )
		{
		case SIGSEGV:
			messagebox( "Error", MB_OK | MB_ICONERROR,
									"The core thread recieved a SIGSEGV signal.\n"
									"This means it tried to access protected memory.\n"
									"Maybe you have set a wrong ucode for one of the plugins!" );
			printf( "SIGSEGV in core thread caught:\n" );
			printf( "\terrno = %d (%s)\n", info->si_errno, strerror( info->si_errno ) );
			printf( "\taddress = 0x%08X\n", (unsigned int)info->si_addr );
#ifdef SEGV_MAPERR
			switch( info->si_code )
			{
			case SEGV_MAPERR: printf( "                address not mapped to object\n" ); break;
			case SEGV_ACCERR: printf( "                invalid permissions for mapped object\n" ); break;
			}
#endif
			break;
		case SIGILL:
			printf( "SIGILL in core thread caught:\n" );
			printf( "\terrno = %d (%s)\n", info->si_errno, strerror( info->si_errno ) );
			printf( "\taddress = 0x%08X\n", (unsigned int)info->si_addr );
#ifdef ILL_ILLOPC
			switch( info->si_code )
			{
			case ILL_ILLOPC: printf( "\tillegal opcode\n" ); break;
			case ILL_ILLOPN: printf( "\tillegal operand\n" ); break;
			case ILL_ILLADR: printf( "\tillegal addressing mode\n" ); break;
			case ILL_ILLTRP: printf( "\tillegal trap\n" ); break;
			case ILL_PRVOPC: printf( "\tprivileged opcode\n" ); break;
			case ILL_PRVREG: printf( "\tprivileged register\n" ); break;
			case ILL_COPROC: printf( "\tcoprocessor error\n" ); break;
			case ILL_BADSTK: printf( "\tinternal stack error\n" ); break;
			}
#endif
			break;
		case SIGFPE:
			printf( "SIGFPE in core thread caught:\n" );
			printf( "\terrno = %d (%s)\n", info->si_errno, strerror( info->si_errno ) );
			printf( "\taddress = 0x%08X\n", (unsigned int)info->si_addr );
			switch( info->si_code )
			{
			case FPE_INTDIV: printf( "\tinteger divide by zero\n" ); break;
			case FPE_INTOVF: printf( "\tinteger overflow\n" ); break;
			case FPE_FLTDIV: printf( "\tfloating point divide by zero\n" ); break;
			case FPE_FLTOVF: printf( "\tfloating point overflow\n" ); break;
			case FPE_FLTUND: printf( "\tfloating point underflow\n" ); break;
			case FPE_FLTRES: printf( "\tfloating point inexact result\n" ); break;
			case FPE_FLTINV: printf( "\tfloating point invalid operation\n" ); break;
			case FPE_FLTSUB: printf( "\tsubscript out of range\n" ); break;
			}
			break;
		default:
			printf( "Signal number %d in core thread caught:\n", signal );
			printf( "\terrno = %d (%s)\n", info->si_errno, strerror( info->si_errno ) );
		}
		pthread_cancel( g_EmulationThread );
		g_EmulationThread = 0;
		g_EmulationRunning = 0;
	}
	else
	{
		printf( "Signal number %d caught:\n", signal );
		printf( "\terrno = %d (%s)\n", info->si_errno, strerror( info->si_errno ) );
		exit( EXIT_FAILURE );
	}
}

/*********************************************************************************************************
 * main function
 */
int
main( int argc, char *argv[] )
{
	int i;
#ifdef VCR_SUPPORT
	const char *p;
#endif
#ifdef WITH_HOME
     {
	char temp[PATH_MAX], orig[PATH_MAX];
	FILE *src, *dest;
	struct dirent *entry;
	DIR *dir;
	
	strcpy(g_WorkingDir, getenv("HOME"));
	strcat(g_WorkingDir, "/.mupen64/");
	mkdir(g_WorkingDir, 0700);
	
	strcpy(temp, g_WorkingDir);
	strcat(temp, "mupen64.ini");
	dest = fopen(temp, "rb");
	if (dest == NULL)
	  {
	     unsigned char byte;
	     dest = fopen(temp, "wb");
	     strcpy(orig, WITH_HOME);
	     strcat(orig, "share/mupen64/mupen64.ini");
	     src = fopen(orig, "rb");
	     while(fread(&byte, 1, 1, src))
	       fwrite(&byte, 1, 1, dest);
	     fclose(src);
	     fclose(dest);
	  }
	else fclose(dest);
	
	strcpy(temp, g_WorkingDir);
	strcat(temp, "lang");
	strcpy(orig, WITH_HOME);
	strcat(orig, "share/mupen64/lang");
	symlink(orig, temp);
	
	/*strcpy(temp, g_WorkingDir);
	strcat(temp, "plugins");
	strcpy(orig, WITH_HOME);
	strcat(orig, "share/mupen64/plugins");
	symlink(orig, temp);*/
	
	strcpy(temp, g_WorkingDir);
	strcat(temp, "plugins");
	mkdir(temp, 0700);
	strcpy(orig, WITH_HOME);
	strcat(orig, "share/mupen64/plugins");
	dir = opendir(orig);
	while((entry = readdir(dir)) != NULL)
	  {
	     if(strcmp(entry->d_name + strlen(entry->d_name) - 3, ".so"))
	       {
		  strcpy(orig, WITH_HOME);
		  strcat(orig, "share/mupen64/plugins/");
		  strcat(orig, entry->d_name);
		  src = fopen(orig, "rb");
		  if(src == NULL) continue;
		  
		  strcpy(temp, g_WorkingDir);
		  strcat(temp, "plugins/");
		  strcat(temp, entry->d_name);
		  dest = fopen(temp, "rb");
		  if(dest == NULL)
		    {
		       unsigned char byte;
		       dest = fopen(temp, "wb");
		       while(fread(&byte, 1, 1, src))
			 fwrite(&byte, 1, 1, dest);
		       fclose(src);
		       fclose(dest);
		    }
		  else fclose(dest);
	       }
	     else
	       {
		  strcpy(temp, g_WorkingDir);
		  strcat(temp, "plugins/");
		  strcat(temp, entry->d_name);
		  strcpy(orig, WITH_HOME);
		  strcat(orig, "share/mupen64/plugins/");
		  strcat(orig, entry->d_name);
		  symlink(orig, temp);
	       }
	  }
	
	strcpy(temp, g_WorkingDir);
	strcat(temp, "save/");
	mkdir(temp, 0700);
	
	chdir(g_WorkingDir);
     }
#else

	if (argv[0][0] != '/')
	{
		getcwd( g_WorkingDir, 2000 );
		strcat( g_WorkingDir, "/" );
		strcat( g_WorkingDir, argv[0] );
	}
	else
		strcpy(g_WorkingDir, argv[0] );

	while (g_WorkingDir[strlen( g_WorkingDir ) - 1] != '/')
		g_WorkingDir[strlen( g_WorkingDir ) - 1] = '\0';
   
#endif

	int scanargs = 1;
	for (i = 1; i < argc; i++)
	{
		if (scanargs)
		{
			if (argv[i][0] == '-')
			{
 				if (!strcmp( argv[i], "--fullscreen" ))
					g_Fullscreen = 1;
//				else if (!strcmp( argv[i], "--nogui" ))
//					g_GuiEnabled = 0;
				else if (!strcmp( argv[i], "--" ))
					scanargs = 0;
				else
					printf( "Unknown argument: %s\n", argv[i] );
			}
			else
				g_Filename = argv[i];
		}
		else
			g_Filename = argv[i];
	}

/*	if (!g_GuiEnabled)
	{
		if (!g_Filename)
		{
			printf( "Usage: %s [--nogui] [--fullscreen] [rom_file]\n", argv[0] );
			return EXIT_FAILURE;
		}
		if( !fill_header( g_Filename ) )
		{
			printf( "Error: Couldn't load Rom!\n" );
			return EXIT_FAILURE;
		}
		if( rom_read( g_Filename ) )
		{
			printf( "Error: Couldn't load Rom!\n" );
			return EXIT_FAILURE;
		}
		config_read();
		emulationThread( NULL );

		return EXIT_SUCCESS;
	}*/

	// init gtk
	gtk_init( &argc, &argv );
#ifdef VCR_SUPPORT
	VCRComp_init();
	p = config_get_string( "VCR Video Codec", "XviD" );
	for (i = 0; i < VCRComp_numVideoCodecs(); i++)
	{
		if (!strcasecmp( VCRComp_videoCodecName( i ), p ))
		{
			VCRComp_selectVideoCodec( i );
			break;
		}
	}
	p = config_get_string( "VCR Audio Codec", VCRComp_audioCodecName( 0 ) );
	for (i = 0; i < VCRComp_numAudioCodecs(); i++)
	{
		if (!strcasecmp( VCRComp_audioCodecName( i ), p ))
		{
			VCRComp_selectAudioCodec( i );
			break;
		}
	}
#endif

	if (g_GuiEnabled)
		tr_load_languages();

	config_read();
        *autoinc_save_slot = config_get_bool( "AutoIncSaveSlot", FALSE );
	ini_openFile();

	if (g_GuiEnabled)
	{
	        GList *langList = tr_language_list();
	        char *language;
	        for( i = 0; i < g_list_length( langList ); i++ )
	        {
		   const char *confLang = config_get_string( "Language", "English" );
		   language = g_list_nth_data( langList, i );
		   if( !strcasecmp( language, confLang ) )
		     tr_set_language(language);
		}
	   
		rombrowser_readCache();

		create_mainWindow();
		create_configDialog();
#ifdef VCR_SUPPORT
		create_vcrCompDialog();
#endif
		create_aboutDialog();
		gtk_widget_show_all( g_MainWindow.window );
	}

	// install signal handler
/*	memset( &sa, 0, sizeof( struct sigaction ) );
	sa.sa_sigaction = sighandler;
	sa.sa_flags = SA_SIGINFO;
	if( sigaction( SIGSEGV, &sa, NULL ) ) printf( "sigaction(): %s\n", strerror( errno ) );
	if( sigaction( SIGILL, &sa, NULL ) ) printf( "sigaction(): %s\n", strerror( errno ) );
	if( sigaction( SIGFPE, &sa, NULL ) ) printf( "sigaction(): %s\n", strerror( errno ) );
	if( sigaction( SIGCHLD, &sa, NULL ) ) printf( "sigaction(): %s\n", strerror( errno ) );
*/
	// go!
	if (g_Filename)
	{
		// let gtk do initial tasks
		while( g_main_iteration( FALSE ) );

		// now load the rom
		open_rom( g_Filename );

		// and start emulation
		callback_startEmulation( 0, 0 );

		// and finally give control for this thread to gtk
	}
	gtk_main();

	callback_stopEmulation( NULL, NULL );

	ini_updateFile(1);
	ini_closeFile();

	return EXIT_SUCCESS;
}
