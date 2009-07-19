#ifdef GUI_SDL

#include "configdialog_sdl.h"

#include <stdarg.h>
#include <string.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_ttf.h"

#include "pad.h"	// pad image
#include "arial.ttf.h"	// arial font
#define FONT_SIZEPT 15

#define SCREEN_W	640
#define SCREEN_H	480

// callback structure
typedef struct
{
	void (*callback)(int _arg);
	int arg;
	int x;
	int y;
	int w;
	int h;
} SCallback;

// extern functions
extern void read_configuration( void ); // from plugin.c
extern int write_configuration( void ); // from plugin.c

// colors
static Uint32 u32black, u32gray, u32dark_gray;
static SDL_Color black      = { 0x00, 0x00, 0x00, 0 };
static SDL_Color gray       = { 0xAA, 0xAA, 0xAA, 0 };
static SDL_Color dark_gray  = { 0x66, 0x66, 0x66, 0 };

// button names
static const char *button_names[] = {
	"DPad R",	// R_DPAD
	"DPad L",	// L_DPAD
	"DPad D",	// D_DPAD
	"DPad U",	// U_DPAD
	"Start",	// START_BUTTON
	"Z Trig",	// Z_TRIG
	"B Button",	// B_BUTTON
	"A Button",	// A_BUTTON
	"C Button R",	// R_CBUTTON
	"C Button L",	// L_CBUTTON
	"C Button D",	// D_CBUTTON
	"C Button U",	// U_CBUTTON
	"R Trig",	// R_TRIG
	"L Trig",	// L_TRIG
	"Y Axis",	// Y_AXIS
	"X Axis"	// X_AXIS
};

// SDL axis names
static char axis_names[] = {
	'X', 'Y', 'Z', 'U', 'V', 'W'
};

// globals
static int keep_looping;	// set this to 0 to hide the config dialog
static SDL_Surface *screen = NULL;
static SDL_Thread  *thread = NULL;
static TTF_Font    *font   = NULL;
static SDL_Surface *pad    = NULL;
static SDL_Joystick *joy   = NULL; // currently opened joystick (to read events)

static int cont = 0;			// selected controller to configure
static SController config[4];	// configuration (copy)
static SController *orig_cont;	// original controllers from plugin.c (to save configuration)

// flush event queue
static inline void
SDL_FlushEventQueue( void )
{
	SDL_Event waste;

	while( SDL_PollEvent( &waste ) == 1 );
}
//#define SDL_FlushEventQueue()

// callback functions
static void controller_tab_clicked( int _arg );
static void button_clicked( int _arg );
static void checkbutton_clicked( int _arg );
static void device_clicked( int _arg );
static void pad_button_clicked( int _arg );

// callbacks
static SCallback callback[] = {
	{ controller_tab_clicked, 0,  10, 10, 155, 20 },
	{ controller_tab_clicked, 1, 165, 10, 155, 20 },
	{ controller_tab_clicked, 2, 320, 10, 155, 20 },
	{ controller_tab_clicked, 3, 475, 10, 155, 20 },

	{ button_clicked, 0, 150, 445, 90, 25 },			// ok
	{ button_clicked, 1, 380, 445, 90, 25 },			// cancel

	{ checkbutton_clicked, 0,  20, 45, 90, 25 },		// plugged
	{ checkbutton_clicked, 1, 120, 45, 90, 25 },		// mempak

	{ device_clicked, 0, 360, 45, 260, 25 },			// device

	{ pad_button_clicked, R_DPAD,       110, 245, 50, 20 },
	{ pad_button_clicked, L_DPAD,       110, 195, 50, 20 },
	{ pad_button_clicked, D_DPAD,       110, 220, 50, 20 },
	{ pad_button_clicked, U_DPAD,       110, 170, 50, 20 },
	{ pad_button_clicked, START_BUTTON, 300,  65, 40, 20 },
	{ pad_button_clicked, Z_TRIG,       120, 290, 50, 20 },
	{ pad_button_clicked, B_BUTTON,     475, 230, 70, 20 },
	{ pad_button_clicked, A_BUTTON,     475, 325, 70, 20 },
	{ pad_button_clicked, L_CBUTTON,    475, 130, 80, 20 },
	{ pad_button_clicked, U_CBUTTON,    475, 155, 80, 20 },
	{ pad_button_clicked, R_CBUTTON,    475, 180, 80, 20 },
	{ pad_button_clicked, D_CBUTTON,    475, 205, 80, 20 },
	{ pad_button_clicked, R_TRIG,       395, 105, 50, 20 },
	{ pad_button_clicked, L_TRIG,       155, 105, 50, 20 },
	{ pad_button_clicked, Y_AXIS,       220, 398, 50, 20 },
	{ pad_button_clicked, X_AXIS,       280, 398, 50, 20 },

	{ checkbutton_clicked, 2,       340, 398, 110, 25 },		// enable mouse

	{ NULL, 0, 0, 0, 0, 0 }
};

// render text
static inline SDL_Surface *
render_text( SDL_Color fg, SDL_Color bg, const char *fmt, ... )
{
	va_list ap;
	char buf[2049];

	va_start( ap, fmt );
	vsnprintf( buf, 2048, fmt, ap );
	va_end( ap );

	if( *buf == '\0' )
		return NULL;

	return( TTF_RenderText_Shaded( font, buf, fg, bg ) );
}

// write text
static inline void
write_text( SDL_Surface *dst, int x, int y, SDL_Color fg, SDL_Color bg, const char *fmt, ... )
{
	SDL_Surface *text;
	SDL_Rect dstrect;
	va_list ap;
	char buf[2049];

	va_start( ap, fmt );
	vsnprintf( buf, 2048, fmt, ap );
	va_end( ap );

	if( *buf == '\0' )
		return;

	text = render_text( fg, bg, buf );
	if( text == NULL )
	{
		fprintf( stderr, "["PLUGIN_NAME"]: Couldn't render text: %s\n", SDL_GetError() );
		return;
	}

	dstrect.x = x;
	dstrect.y = y;
	dstrect.w = text->w;
	dstrect.h = text->h;
	SDL_BlitSurface( text, NULL, dst, &dstrect );
	SDL_FreeSurface( text );
}

// controller tab clicked
static void
controller_tab_clicked( int _arg )
{
#ifdef _DEBUG
	printf( "controller_tab_clicked(): controller = %d\n", _arg );
#endif
	cont = _arg;
}

// button clicked (ok/cancel)
static void
button_clicked( int _arg )
{
	int i;

	if( _arg == 0 )
	{
#ifdef _DEBUG
		printf( "OK clicked!\n" );
#endif
		// save configuration
		for( i = 0; i < 4; i++ )
		{
			orig_cont[i].device = config[i].device;
			orig_cont[i].mouse = config[i].mouse;
			memcpy( orig_cont[i].axis, config[i].axis, sizeof( SAxisMap ) * 2 );
			memcpy( orig_cont[i].button, config[i].button, sizeof( SButtonMap ) * 14 );
			memcpy( &orig_cont[i].control, &config[i].control, sizeof( CONTROL ) );
		}
		write_configuration();
	}
	else if( _arg == 1 )
	{
#ifdef _DEBUG
		printf( "Cancel clicked!\n" );
#endif
	}

	// close dialog
	keep_looping = 0;
}

// plugged/mempak checkbutton clicked
static void
checkbutton_clicked( int _arg )
{
	if( _arg == 0 )
	{
		config[cont].control.Present ^= 1;
	}
	else if( _arg == 1 )
	{
		if( config[cont].control.Plugin == PLUGIN_MEMPAK )
			config[cont].control.Plugin = PLUGIN_NONE;
		else
			config[cont].control.Plugin = PLUGIN_MEMPAK;
	}
	else if( _arg == 2 )
	{
		if (config[cont].mouse == 0)
		{
			if (config[0].mouse || config[1].mouse || config[2].mouse || config[3].mouse)
			{
				// error
			}
		}
		config[cont].mouse ^= 1;
	}
}

// device box clicked
static void
device_clicked( int _arg )
{
	config[cont].device++;
	if( config[cont].device == SDL_NumJoysticks() )
		config[cont].device = DEVICE_NONE;
}

// open/close joystick
static inline void
close_joystick( void )
{
	if( joy )
	{
		SDL_JoystickClose( joy );
		joy = NULL;
	}
}

static inline void
open_joystick( void )
{
	close_joystick();
	if( config[cont].device >= 0 )
	{
		joy = SDL_JoystickOpen( config[cont].device );
		if( !joy )
		{
			fprintf( stderr, "["PLUGIN_NAME"]: Couldn't open joystick #%d (%s): %s\n",
					config[cont].device, SDL_JoystickName( config[cont].device ), SDL_GetError() );
		}
	}
}

// display a small dialog
static void
display_dialog( const char *caption, const char *line1, const char *line2 )
{
	SDL_Rect dstrect, dstrect2;

	// draw the dialog
	dstrect.w = 300; dstrect.h = 100;
	dstrect.x = (screen->w - dstrect.w) / 2;
	dstrect.y = (screen->h - dstrect.h) / 2;
	SDL_FillRect( screen, &dstrect, u32black );
	dstrect2.x = dstrect.x + 2; dstrect2.y = dstrect.y + 2;
	dstrect2.w = dstrect.w - 4;
	dstrect2.h = 25;
	SDL_FillRect( screen, &dstrect2, u32gray );
	write_text( screen, dstrect2.x + 100, dstrect2.y, black, gray, caption );
	dstrect2.y += 26;
	dstrect2.h = dstrect.h - 30;
	SDL_FillRect( screen, &dstrect2, u32gray );
	write_text( screen, dstrect2.x + 10, dstrect2.y, black, gray, line1 );
	write_text( screen, dstrect2.x + 10, dstrect2.y + 20, black, gray, line2 );

	SDL_Flip( screen );
}

// pad button clicked (read assignment)
static void
pad_button_clicked( int _arg )
{
	SDL_Event event;
	int waitevent, axis;
	char text[2000];
	SDLKey key_a = 0;
	int button_a = 0;
	int hat_pos_a = 0;

	open_joystick();

	SDL_FlushEventQueue();
	if( _arg == Y_AXIS || _arg == X_AXIS )
	{
		// read key/button a
		sprintf( text, "a key/button for '%s'",
				(_arg == X_AXIS) ? "left" : "up" );
		axis = _arg - Y_AXIS;
		display_dialog( button_names[_arg], "Move any axis or press", text );
		waitevent = 1;
		while( waitevent )
		{
			if( SDL_WaitEvent( &event ) == 0 )
			{
				fprintf( stderr, "["PLUGIN_NAME"]: SDL_WaitEvent(): %s\n", SDL_GetError() );
				close_joystick();
				return;
			}

			switch( event.type )
			{
			case SDL_KEYDOWN:
				if( event.key.keysym.sym == SDLK_ESCAPE )
				{
					close_joystick();
					return;
				}
				key_a = event.key.keysym.sym;
				waitevent = 0;
				break;

			case SDL_JOYAXISMOTION:
				if( event.jaxis.which == config[cont].device )
				{
					if( event.jaxis.value >= 15000 || event.jaxis.value <= -15000 )
					{
						config[cont].axis[axis].axis = event.jaxis.axis;
						close_joystick();
						return;
					}
				}
				break;

			case SDL_JOYBUTTONDOWN:
				if( event.jbutton.which == config[cont].device )
				{
					button_a = event.jbutton.button;
					waitevent = 0;
				}
				break;

			case SDL_JOYHATMOTION:
				if( event.jhat.value == SDL_HAT_UP || event.jhat.value == SDL_HAT_DOWN ||
						event.jhat.value == SDL_HAT_LEFT || event.jhat.value == SDL_HAT_RIGHT )
				{
					hat_pos_a = event.jhat.value;
					waitevent = 0;
				}
				break;
			}
		}

		// read key/button b
		sprintf( text, "Press a key/button for '%s'",
				(_arg == X_AXIS) ? "right" : "down" );
		display_dialog( button_names[_arg], text, "" );

		SDL_FlushEventQueue();
		waitevent = 1;
		while( waitevent )
		{
			if( SDL_WaitEvent( &event ) == 0 )
			{
				fprintf( stderr, "["PLUGIN_NAME"]: SDL_WaitEvent(): %s\n", SDL_GetError() );
				close_joystick();
				return;
			}

			switch( event.type )
			{
			case SDL_KEYDOWN:
				if( event.key.keysym.sym == SDLK_ESCAPE )
				{
					close_joystick();
					return;
				}

				config[cont].axis[axis].key_a = key_a;
				config[cont].axis[axis].key_b = event.key.keysym.sym;
				waitevent = 0;
				break;
			case SDL_JOYBUTTONDOWN:
				if( event.jbutton.which == config[cont].device )
				{
					config[cont].axis[axis].button_a = button_a;
					config[cont].axis[axis].button_b = event.jbutton.button;
					waitevent = 0;
				}
				break;

			case SDL_JOYHATMOTION:
				if( event.jhat.value == SDL_HAT_UP || event.jhat.value == SDL_HAT_DOWN ||
				    event.jhat.value == SDL_HAT_LEFT || event.jhat.value == SDL_HAT_RIGHT )
				{
					config[cont].axis[_arg].hat = event.jhat.hat;
					config[cont].axis[_arg].hat_pos_a = hat_pos_a;
					config[cont].axis[_arg].hat_pos_b = event.jhat.value;
					waitevent = 0;
				}
				break;
			}
		}
	}
	else
	{
		display_dialog( button_names[_arg], "Press a key/button or", "move any axis..." );

		waitevent = 1;
		while( waitevent )
		{
			if( SDL_WaitEvent( &event ) == 0 )
			{
				fprintf( stderr, "["PLUGIN_NAME"]: SDL_WaitEvent(): %s\n", SDL_GetError() );
				close_joystick();
				return;
			}

			switch( event.type )
			{
			case SDL_KEYDOWN:
				if( event.key.keysym.sym == SDLK_ESCAPE )
				{
					close_joystick();
					return;
				}
				config[cont].button[_arg].key = event.key.keysym.sym;
				waitevent = 0;
				break;

			case SDL_JOYBUTTONDOWN:
				if( event.jbutton.which == config[cont].device )
				{
					config[cont].button[_arg].button = event.jbutton.button;
					waitevent = 0;
				}
				break;

			case SDL_JOYAXISMOTION:
				if( event.jaxis.which == config[cont].device )
				{
					if( event.jaxis.value >= 15000 )
					{
						config[cont].button[_arg].axis = event.jaxis.axis;
						config[cont].button[_arg].axis_dir = 1;
						waitevent = 0;
					}
					else if( event.jaxis.value <= -15000 )
					{
						config[cont].button[_arg].axis = event.jaxis.axis;
						config[cont].button[_arg].axis_dir = -1;
						waitevent = 0;
					}
				}
				break;

			case SDL_JOYHATMOTION:
				if( event.jhat.which == config[cont].device )
				{
					if( event.jhat.value == SDL_HAT_UP || event.jhat.value == SDL_HAT_DOWN ||
					    event.jhat.value == SDL_HAT_LEFT || event.jhat.value == SDL_HAT_RIGHT )
					{
						config[cont].button[_arg].hat = event.jhat.hat;
						config[cont].button[_arg].hat_pos = event.jhat.value;
						waitevent = 0;
					}
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if( config[cont].mouse )
				{
					config[cont].button[_arg].mouse = event.button.button;
					waitevent = 0;
				}
				break;
			}
		}
	}

	close_joystick();
}

// display button mapping in a tool-tip
#define HAT_POS_NAME( hat )				\
       ((hat == SDL_HAT_UP) ? "Up" :		\
       ((hat == SDL_HAT_DOWN) ? "Down" :	\
       ((hat == SDL_HAT_LEFT) ? "Left" :	\
       ((hat == SDL_HAT_RIGHT) ? "Right" :	\
         "None"))))

static void
display_button_map( int button, int x, int y )
{
	SDL_Rect dstrect, dstrect2;
	const char *key_a, *key_b;
	char button_a[200], button_b[200], axis[200], hat[200], hat_pos_a[200], hat_pos_b[200], mbutton[200];
	int a;

	// draw the dialog
	dstrect.w = 300; dstrect.h = 140;
	if( screen->w - x >= dstrect.w )
		dstrect.x = x;
	else
		dstrect.x = x - 10 - dstrect.w;
	if( screen->h - y >= dstrect.h )
		dstrect.y = y;
	else
		dstrect.y = y - 10 - dstrect.h;

	SDL_FillRect( screen, &dstrect, u32black );
	dstrect2.x = dstrect.x + 2; dstrect2.y = dstrect.y + 2;
	dstrect2.w = dstrect.w - 4;
	dstrect2.h = 25;
	SDL_FillRect( screen, &dstrect2, u32gray );
	write_text( screen, dstrect2.x + 100, dstrect2.y, black, gray, button_names[button] );
	dstrect2.y += 26;
	dstrect2.h = dstrect.h - 30;
	SDL_FillRect( screen, &dstrect2, u32gray );

	// write mapping
	if( button == Y_AXIS || button == X_AXIS )
	{
		a = button - Y_AXIS;

		key_a = (config[cont].axis[a].key_a == SDLK_UNKNOWN) ? "None" : SDL_GetKeyName( config[cont].axis[a].key_a );
		key_b = (config[cont].axis[a].key_b == SDLK_UNKNOWN) ? "None" : SDL_GetKeyName( config[cont].axis[a].key_b );

		if( config[cont].axis[a].button_a >= 0 )
			sprintf( button_a, "%d", config[cont].axis[a].button_a );
		else
			strcpy( button_a, "None" );

		if( config[cont].axis[a].button_b >= 0 )
			sprintf( button_b, "%d", config[cont].axis[a].button_b );
		else
			strcpy( button_b, "None" );

		if( config[cont].axis[a].axis >= 0 )
			sprintf( axis, "%c Axis", axis_names[config[cont].axis[a].axis] );
		else
			strcpy( axis, "None" );

		if( config[cont].axis[a].hat >= 0 )
		{
			sprintf( hat, "#%d", config[cont].axis[a].hat );
			strcpy( hat_pos_a, HAT_POS_NAME(config[cont].axis[a].hat_pos_a) );
			strcpy( hat_pos_b, HAT_POS_NAME(config[cont].axis[a].hat_pos_b) );
		}
		else
		{
			strcpy( hat, "None" );
			strcpy( hat_pos_a, "None" );
			strcpy( hat_pos_b, "None" );
		}

		if( button == Y_AXIS )
		{
			write_text( screen, dstrect2.x + 10, dstrect2.y,      black, gray, "Keys: Up = %s, Down = %s", key_a, key_b );
			write_text( screen, dstrect2.x + 10, dstrect2.y + 20, black, gray, "Buttons: Up = %s, Down = %s", button_a, button_b );
			write_text( screen, dstrect2.x + 10, dstrect2.y + 40, black, gray, "Hat: %s; Up = %s, Down = %s", hat, hat_pos_a, hat_pos_b );
		}
		else
		{
			write_text( screen, dstrect2.x + 10, dstrect2.y,      black, gray, "Keys: Left = %s, Right = %s", key_a, key_b );
			write_text( screen, dstrect2.x + 10, dstrect2.y + 20, black, gray, "Buttons: Left = %s, Right = %s", button_a, button_b );
			write_text( screen, dstrect2.x + 10, dstrect2.y + 40, black, gray, "Hat: %s; Left = %s, Right = %s", hat, hat_pos_a, hat_pos_b );
		}
		write_text( screen, dstrect2.x + 10, dstrect2.y + 60, black, gray, "Axis: %s", axis );
	}
	else
	{
		key_a = (config[cont].button[button].key == SDLK_UNKNOWN) ? "None" : SDL_GetKeyName( config[cont].button[button].key );

		if( config[cont].button[button].button >= 0 )
			sprintf( button_a, "%d", config[cont].button[button].button );
		else
			strcpy( button_a, "None" );

		if( config[cont].button[button].axis >= 0 )
			sprintf( axis, "%c Axis %c", axis_names[config[cont].button[button].axis],
					(config[cont].button[button].axis_dir > 0) ? '+' : '-' );
		else
			strcpy( axis, "None" );

		if( config[cont].button[button].hat >= 0 )
		{
			sprintf( hat, "#%d", config[cont].button[button].hat );
			strcpy( hat_pos_a, " " );
			strcat( hat_pos_a, HAT_POS_NAME(config[cont].button[button].hat_pos) );
		}
		else
		{
			strcpy( hat, "None" );
			strcpy( hat_pos_a, "" );
		}

		if( config[cont].button[button].mouse > 0 )
			sprintf( mbutton, "#%d", config[cont].button[button].mouse );
		else
			strcpy( mbutton, "None" );

		write_text( screen, dstrect2.x + 10, dstrect2.y,      black, gray, "Key: %s", key_a );
		write_text( screen, dstrect2.x + 10, dstrect2.y + 20, black, gray, "Button: %s", button_a );
		write_text( screen, dstrect2.x + 10, dstrect2.y + 40, black, gray, "Hat: %s%s", hat, hat_pos_a );
		write_text( screen, dstrect2.x + 10, dstrect2.y + 60, black, gray, "Axis: %s", axis );
		write_text( screen, dstrect2.x + 10, dstrect2.y + 80, black, gray, "Mouse Btn: %s", mbutton );
	}

	SDL_Flip( screen );
}

// thread
static int
configure_thread( void *_arg )
{
	int i, x, y, button;
	SDL_Rect dstrect, dstrect2;
	SDL_Event event;
	SDL_Surface *surface;

	keep_looping = 1;
	while( keep_looping )
	{
		// read mouse state
		SDL_PumpEvents();
		SDL_GetMouseState( &x, &y );

		// draw screen
		SDL_FillRect( screen, NULL, u32dark_gray );

		// draw the controller tab
		dstrect.x = 10;
		dstrect.y = 10;
		dstrect.w = (SCREEN_W - 20) / 4;
		dstrect.h = 25;
		for( i = 0; i < 4; i++ )
		{
			if( i == cont )
			{
				dstrect2.x = dstrect.x + 1; dstrect2.y = dstrect.y + 1;
				dstrect2.w = dstrect.w - 2; dstrect2.h = dstrect.h - 1;
				SDL_FillRect( screen, &dstrect, u32black );
				SDL_FillRect( screen, &dstrect2, u32gray );
			}
			else
			{
				SDL_FillRect( screen, &dstrect, u32dark_gray );
			}

			write_text( screen, dstrect.x + 32, dstrect.y + 2, black, (i == cont) ? gray : dark_gray, "Controller %d", i + 1 );

			dstrect.x += dstrect.w;
		}

		dstrect.x = 10;
		dstrect.y = 35;
		dstrect.w = SCREEN_W - 20;
		dstrect.h = SCREEN_H - 80;
		SDL_FillRect( screen, &dstrect, u32black );
		dstrect.x++; dstrect.y++; dstrect.w -= 2; dstrect.h -= 2;
		SDL_FillRect( screen, &dstrect, u32gray );

		// draw the pad
		dstrect.x = (SCREEN_W - pad->w) / 2;
		dstrect.y = (SCREEN_H - pad->h) / 2;
		dstrect.w = pad->w;
		dstrect.h = pad->h;
		SDL_BlitSurface( pad, NULL, screen, &dstrect );

		// draw the button names
		write_text( screen, 110, 245, black, gray, button_names[R_DPAD] );
		write_text( screen, 110, 195, black, gray, button_names[L_DPAD] );
		write_text( screen, 110, 220, black, gray, button_names[D_DPAD] );
		write_text( screen, 110, 170, black, gray, button_names[U_DPAD] );
		write_text( screen, 300,  65, black, gray, button_names[START_BUTTON] );
		write_text( screen, 120, 290, black, gray, button_names[Z_TRIG] );
		write_text( screen, 475, 230, black, gray, button_names[B_BUTTON] );
		write_text( screen, 475, 325, black, gray, button_names[A_BUTTON] );
		write_text( screen, 475, 130, black, gray, button_names[L_CBUTTON] );
		write_text( screen, 475, 155, black, gray, button_names[U_CBUTTON] );
		write_text( screen, 475, 180, black, gray, button_names[R_CBUTTON] );
		write_text( screen, 475, 205, black, gray, button_names[D_CBUTTON] );
		write_text( screen, 395, 105, black, gray, button_names[R_TRIG] );
		write_text( screen, 155, 105, black, gray, button_names[L_TRIG] );
		write_text( screen, 220, 398, black, gray, button_names[Y_AXIS] );
		write_text( screen, 280, 398, black, gray, button_names[X_AXIS] );

		// draw plugged checkbutton
		dstrect.x = 20; dstrect.y = 45; dstrect.w = 90; dstrect.h = 25;
		SDL_FillRect( screen, &dstrect, u32black );
		dstrect.x++; dstrect.y++; dstrect.w -= 2; dstrect.h -= 2;
		SDL_FillRect( screen, &dstrect, (config[cont].control.Present) ? u32gray : u32dark_gray );
		write_text( screen, dstrect.x + 13, dstrect.y, black, (config[cont].control.Present) ? gray : dark_gray, "Plugged" );

		// draw mempak checkbutton
		dstrect.x = 120; dstrect.y = 45; dstrect.w = 90; dstrect.h = 25;
		SDL_FillRect( screen, &dstrect, u32black );
		dstrect.x++; dstrect.y++; dstrect.w -= 2; dstrect.h -= 2;
		SDL_FillRect( screen, &dstrect, (config[cont].control.Plugin == PLUGIN_MEMPAK) ? u32gray : u32dark_gray );
		write_text( screen, dstrect.x + 12, dstrect.y, black, (config[cont].control.Plugin == PLUGIN_MEMPAK) ? gray : dark_gray, "Mem Pak" );

		// draw mouse checkbutton
		dstrect.x = 340; dstrect.y = 398; dstrect.w = 110; dstrect.h = 25;
		SDL_FillRect( screen, &dstrect, u32black );
		dstrect.x++; dstrect.y++; dstrect.w -= 2; dstrect.h -= 2;
		SDL_FillRect( screen, &dstrect, (config[cont].mouse) ? u32gray : u32dark_gray );
		write_text( screen, dstrect.x + 12, dstrect.y, black, (config[cont].mouse) ? gray : dark_gray, "Enable Mouse" );

		// draw device box
		write_text( screen, 300, 45, black, gray, "Device:" );
		dstrect.x = 360; dstrect.y = 45; dstrect.w = 260; dstrect.h = 25;
		SDL_FillRect( screen, &dstrect, u32black );
		dstrect.x++; dstrect.y++; dstrect.w -= 2; dstrect.h -= 2;
		SDL_FillRect( screen, &dstrect, u32dark_gray );
		if( config[cont].device == DEVICE_NONE )
			surface = render_text( black, dark_gray, "None" );
		else if( config[cont].device == DEVICE_KEYBOARD )
			surface = render_text( black, dark_gray, "Keyboard" );
		else
			surface = render_text( black, dark_gray, SDL_JoystickName( config[cont].device ) );

		dstrect.x += dstrect.w - surface->w - 5;
		dstrect.w = surface->w;
		dstrect.h = surface->h;
		SDL_BlitSurface( surface, NULL, screen, &dstrect );
		SDL_FreeSurface( surface );

		// draw some help
		dstrect.x = 20; dstrect.y = 80; dstrect.w = 55; dstrect.h = 25;
		SDL_FillRect( screen, &dstrect, u32black );
		dstrect2.x = dstrect.x + 1; dstrect2.y = dstrect.y + 1;
		dstrect2.w = dstrect.w - 2; dstrect2.h = dstrect.h - 2;
		SDL_FillRect( screen, &dstrect2, u32gray );
		write_text( screen, dstrect.x + 10, dstrect.y + 2, black, gray, "Help" );

		if( x >= dstrect.x && y >= dstrect.y &&
			x <= dstrect.x + dstrect.w &&
			y <= dstrect.y + dstrect.h )
		{
			dstrect.x = x + 5; dstrect.y = y + 5;
			dstrect.w = 310; dstrect.h = 190;
			SDL_FillRect( screen, &dstrect, u32black );
			dstrect.x++; dstrect.y++; dstrect.w -= 2; dstrect.h -= 2;
			SDL_FillRect( screen, &dstrect, u32gray );
			write_text( screen, dstrect.x + 10, dstrect.y +   0, black, gray, "Move the mouse cursor over a button" );
			write_text( screen, dstrect.x + 10, dstrect.y +  20, black, gray, "name to see the mappings. Click it" );
			write_text( screen, dstrect.x + 10, dstrect.y +  40, black, gray, "to assign a new mapping. Press DEL" );
			write_text( screen, dstrect.x + 10, dstrect.y +  60, black, gray, "to clear the mappings and ESC to" );
			write_text( screen, dstrect.x + 10, dstrect.y +  80, black, gray, "cancel an assignment." );
			write_text( screen, dstrect.x + 10, dstrect.y + 100, black, gray, "The plugged and mempak checkbuttons" );
			write_text( screen, dstrect.x + 10, dstrect.y + 120, black, gray, "can be toggled by clicking them." );
			write_text( screen, dstrect.x + 10, dstrect.y + 140, black, gray, "You can cycle through the devices by" );
			write_text( screen, dstrect.x + 10, dstrect.y + 160, black, gray, "clicking the device entry." );
		}

		// draw ok button
		dstrect.x = 150; dstrect.y = 445; dstrect.w = 90; dstrect.h = 25;
		SDL_FillRect( screen, &dstrect, u32black );
		dstrect.x++; dstrect.y++; dstrect.w -= 2; dstrect.h -= 2;
		SDL_FillRect( screen, &dstrect, u32gray );
		write_text( screen, dstrect.x + 35, dstrect.y, black, gray, "Ok" );
		
		// draw cancel button
		dstrect.x = 380; dstrect.y = 445; dstrect.w = 90; dstrect.h = 25;
		SDL_FillRect( screen, &dstrect, u32black );
		dstrect.x++; dstrect.y++; dstrect.w -= 2; dstrect.h -= 2;
		SDL_FillRect( screen, &dstrect, u32gray );
		write_text( screen, dstrect.x + 20, dstrect.y, black, gray, "Cancel" );

		// draw mapping tool-tips
		// search button
		button = -1;
		for( i = 0; callback[i].callback; i++ )
		{
			if( callback[i].callback == pad_button_clicked )
			{
				if( x >= callback[i].x && y >= callback[i].y &&
					x <= callback[i].x + callback[i].w &&
					y <= callback[i].y + callback[i].h )
				{
					display_button_map( callback[i].arg, x + 5, y + 5 );
					button = callback[i].arg;
				}
			}
		}


		// update screen
		SDL_Flip( screen );

		// process input queue
		while( SDL_PollEvent( &event ) )
		{
			switch( event.type )
			{
			case SDL_MOUSEBUTTONDOWN:
				// search callback
				for( i = 0; callback[i].callback; i++ )
				{
					if( event.button.x >= callback[i].x && event.button.y >= callback[i].y &&
						event.button.x <= callback[i].x + callback[i].w &&
						event.button.y <= callback[i].y + callback[i].h )
					{
						callback[i].callback( callback[i].arg );
						break;
					}
				}
				break;
			case SDL_KEYDOWN:
				if( event.key.keysym.sym == SDLK_DELETE )
				{
					if( button >= 0 )
					{
						// clear mappings
						if( button == Y_AXIS || button == X_AXIS )
						{
							int axis = button - Y_AXIS;

							config[cont].axis[axis].axis = -1;
							config[cont].axis[axis].button_a = -1;
							config[cont].axis[axis].button_b = -1;
							config[cont].axis[axis].key_a = SDLK_UNKNOWN;
							config[cont].axis[axis].key_b = SDLK_UNKNOWN;
							config[cont].axis[axis].hat = -1;
							config[cont].axis[axis].hat_pos_a = -1;
							config[cont].axis[axis].hat_pos_b = -1;
						}
						else
						{
							config[cont].button[button].axis = -1;
							config[cont].button[button].axis_dir = 0;
							config[cont].button[button].button = -1;
							config[cont].button[button].key = SDLK_UNKNOWN;
							config[cont].button[button].hat = -1;
							config[cont].button[button].hat_pos = -1;
							config[cont].button[button].mouse = -1;
						}
					}
				}
				break;
			case SDL_QUIT:
				keep_looping = 0;
				break;
			}
		}
	}

	SDL_FreeSurface( pad );

	// close sdl
	TTF_Quit();
	SDL_JoystickEventState( SDL_DISABLE );
	SDL_QuitSubSystem( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK );

	return 0;
}

// config function
void
configure_sdl( SController *controller )
{
	SDL_RWops *rw;
	int i;

	// init sdl
	if( !SDL_WasInit( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) )
		if( SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 )
		{
			fprintf( stderr, "["PLUGIN_NAME"]: Couldn't init SDL video and joystick subsystem: %s\n", SDL_GetError() );
			return;
		}
	SDL_JoystickEventState( SDL_ENABLE );

	// load the pad image
	pad = SDL_CreateRGBSurfaceFrom( (char *)pad_image.pixel_data, pad_image.width, pad_image.height, pad_image.bytes_per_pixel * 8,
					pad_image.width * pad_image.bytes_per_pixel, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 );
	if( !pad )
	{
		fprintf( stderr, "["PLUGIN_NAME"]: Couldn't load the pad image from memory: %s\n", SDL_GetError() );
		SDL_JoystickEventState( SDL_DISABLE );
		SDL_QuitSubSystem( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK );
		return;
	}

	// init sdl_ttf2
	if( !TTF_WasInit() )
		if( TTF_Init() < 0 )
		{
			fprintf( stderr, "["PLUGIN_NAME"]: Couldn't init TTF library: %s\n", SDL_GetError() );
			SDL_JoystickEventState( SDL_DISABLE );
			SDL_QuitSubSystem( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK );
			return;
		}

	// open font
	rw = SDL_RWFromMem( (char *)arial.data, arial.size );
	font = TTF_OpenFontRW( rw, 0, FONT_SIZEPT );
	if( font == NULL )
	{
		fprintf( stderr, "["PLUGIN_NAME"]: Couldn't load %d pt font: %s\n", FONT_SIZEPT, SDL_GetError() );
		TTF_Quit();
		SDL_JoystickEventState( SDL_DISABLE );
		SDL_QuitSubSystem( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK );
		return;
	}
	TTF_SetFontStyle( font, TTF_STYLE_NORMAL );

	// display dialog (set video mode)
	screen = SDL_SetVideoMode( 640, 480, 0, SDL_SWSURFACE );
	if( !screen )
	{
		fprintf( stderr, "["PLUGIN_NAME"]: Couldn't set video mode 640x480: %s\n", SDL_GetError() );
		TTF_Quit();
		SDL_JoystickEventState( SDL_DISABLE );
		SDL_QuitSubSystem( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK );
		return;
	}
	SDL_WM_SetCaption( PLUGIN_NAME" "PLUGIN_VERSION, NULL );

	// create colors
	u32black      = SDL_MapRGBA( screen->format, black.r, black.g, black.b, 0 );
	u32gray       = SDL_MapRGBA( screen->format, gray.r, gray.g, gray.b, 0 );
	u32dark_gray  = SDL_MapRGBA( screen->format, dark_gray.r, dark_gray.g, dark_gray.b, 0 );

	// load configuration
	orig_cont = controller;
	for( i = 0; i < 4; i++ )
	{
		config[i].device = orig_cont[i].device;
		config[i].mouse = orig_cont[i].mouse;
		memcpy( config[i].axis, orig_cont[i].axis, sizeof( SAxisMap ) * 2 );
		memcpy( config[i].button, orig_cont[i].button, sizeof( SButtonMap ) * 14 );
		memcpy( &config[i].control, &orig_cont[i].control, sizeof( CONTROL ) );
	}

	// run thread
	thread = SDL_CreateThread( configure_thread, NULL );
	if( !thread )
	{
		fprintf( stderr, "["PLUGIN_NAME"]: Couldn't create thread: %s\n", SDL_GetError() );
		TTF_Quit();
		SDL_JoystickEventState( SDL_DISABLE );
		SDL_QuitSubSystem( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK );
		return;
	}

	// everything ok
}

#endif // GUI_SDL
