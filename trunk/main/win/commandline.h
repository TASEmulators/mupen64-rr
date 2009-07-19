/***************************************************************************
                          commandline.h  -  description
                             -------------------
    copyright            : (C) 2003 by ShadowPrince
    email                : shadow@emulation64.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 
#ifndef COMMANDLINE_H
#define COMMANDLINE_H
#include <windows.h>

typedef enum {
	CMDLINE_AUDIO_PLUGIN,
	CMDLINE_VIDEO_PLUGIN,
	CMDLINE_CONTROLLER_PLUGIN,
	CMDLINE_RSP_PLUGIN,
	CMDLINE_ROM_DIR,
	CMDLINE_GAME_FILENAME,
	CMDLINE_FULL_SCREEN_FLAG,
	CMDLINE_NO_GUI,
	CMDLINE_SAVE_OPTIONS,
	CMDLINE_MAX_NUMBER
}CmdLineParameterType;

void SaveCmdLineParameter(char *cmdline);
void GetCmdLineParameter(CmdLineParameterType arg, char *buf);
BOOL StartGameByCommandLine();
BOOL GuiDisabled();
BOOL CmdLineParameterExist( int param);

extern BOOL cmdlineMode; 
extern BOOL cmdlineSave;
extern BOOL cmdlineNoGui;
#endif
