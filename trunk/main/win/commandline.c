
/***************************************************************************
                          commandline.c  -  description
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

// Based on code from 1964 by Schibo and Rice
// Slightly improved command line params parsing function to work with spaced arguments

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "commandline.h"
#include "main_win.h"
#include "../plugin.h"
#include "GUI_LogWindow.h"


BOOL cmdlineMode = 0;
BOOL cmdlineSave = 0;
BOOL cmdlineNoGui = 0;
char cmdLineParameterBuf[250] = {0};
void SaveCmdLineParameter(char *cmdline)
{
	strcpy(cmdLineParameterBuf, cmdline);
	if( strlen(cmdLineParameterBuf) > 0 )
	{
		int i;
		int len = strlen(cmdLineParameterBuf);
		for( i=0; i<len; i++ )
		{
			if( isupper(cmdLineParameterBuf[i]) )
			{
				cmdLineParameterBuf[i] = tolower(cmdLineParameterBuf[i]);
			}
		}
	}
}

//To get a command line parameter if available, please pass a flag
// Flags:
//	"-v"	-> return video plugin name
//	"-a"	-> return audio plugin name
//  "-c"	-> return controller plugin name
//  "-g"	-> return game name to run
//	"-f"	-> return play-in-full-screen flag
//	"-r"	-> return rom path
//  "-nogui"-> nogui mode
//  "-save" -> save options on exit 
char *CmdLineArgFlags[] =
{
	"-a",
	"-v",
	"-c",
	"-rsp",
	"-r",
	"-g",
	"-f",
	"-nogui",
	"-save"
};

void GetCmdLineParameter(CmdLineParameterType arg, char *buf)
{
	char *ptr1;
	char *ptr2 = buf;

	if( (arg >= CMDLINE_MAX_NUMBER) || (ptr1=strstr(cmdLineParameterBuf,CmdLineArgFlags[arg]))==NULL )
	{
		buf[0] = 0;
		return;
	}
	
	if( arg == CMDLINE_FULL_SCREEN_FLAG )
	{
		strcpy(buf, "1");
		return;
	}
	
	if ( arg == CMDLINE_NO_GUI )
	{
	    strcpy(buf, "1");
		return;
    }
    
    if ( arg == CMDLINE_SAVE_OPTIONS )
	{
	    strcpy(buf, "1");
		return;
    }
	
	ptr1 = strstr(cmdLineParameterBuf,CmdLineArgFlags[arg]);
	ptr1 += strlen(CmdLineArgFlags[arg]);	//Skip the flag
	
    while( *ptr1 != 0 && isspace(*ptr1) )
	{
		ptr1++;	//skip all spaces
	}
	
	if (strncmp (ptr1, "\"",1)==0) {
	    ptr1++;   //skipping first "
	    while( !(strncmp (ptr1, "\"",1)==0) && (*ptr1 != 0)) {
	        *ptr2++ = *ptr1++;  
        }
    }
    else {
	    while( !isspace(*ptr1) && *ptr1 != 0)
	    {
		   *ptr2++ = *ptr1++;
	    };
	}
	*ptr2 = 0;
}

void setPluginFromCmndLine( CmdLineParameterType plugintype, char *PluginVar, int spec_type)
{
    char tempstr[100];
    char *tempPluginStr;
    GetCmdLineParameter( plugintype, tempstr);
    if( strlen(tempstr) > 0 )
	  {
        ShowInfo("Command Line: Checking plugin name: %s",tempstr);
        tempPluginStr = getPluginName( tempstr, spec_type);
        if (tempPluginStr) {
          sprintf(PluginVar,tempPluginStr);
          ShowInfo("Command Line: Loaded Plugin : %s",PluginVar);
         }    
   	  }
  
}

BOOL StartGameByCommandLine()
{
	char szFileName[MAX_PATH];
	
    if( strlen(cmdLineParameterBuf) == 0 ) {
        ShowInfo("No command line params specified");
        return FALSE;
    }
    
    cmdlineMode = 1;
		
	/// Reading command line params
	
	if( CmdLineParameterExist(CMDLINE_FULL_SCREEN_FLAG))
	{
		 ShowInfo("Command Line: Fullscreen mode on");
         Config.StartFullScreen = 1;
	}
	
	if( CmdLineParameterExist(CMDLINE_SAVE_OPTIONS) )
	{
		 ShowInfo("Command Line: Save mode on");
         cmdlineSave = 1;
	}
	//Plugins
    setPluginFromCmndLine( CMDLINE_AUDIO_PLUGIN, sound_name, PLUGIN_TYPE_AUDIO);
    setPluginFromCmndLine( CMDLINE_VIDEO_PLUGIN, gfx_name, PLUGIN_TYPE_GFX);
    setPluginFromCmndLine( CMDLINE_CONTROLLER_PLUGIN, input_name, PLUGIN_TYPE_CONTROLLER);
    setPluginFromCmndLine( CMDLINE_RSP_PLUGIN, rsp_name, PLUGIN_TYPE_RSP);
    
    if( !CmdLineParameterExist(CMDLINE_GAME_FILENAME) )
	{
		ShowInfo("Command Line: Rom name not specified");
		return FALSE;
    }
	else 
	{
	    GetCmdLineParameter(CMDLINE_GAME_FILENAME, szFileName);
        ShowInfo("Command Line: Rom Name :%s",szFileName);
    }
    
   	if(!StartRom(szFileName))
	{
	     return TRUE;
	}
	else
	{
		 ShowInfo("Command Line: Rom not found");
         return FALSE;
	}

}

BOOL GuiDisabled() 
{
   cmdlineNoGui = CmdLineParameterExist(CMDLINE_NO_GUI);
   return cmdlineNoGui;
}

BOOL CmdLineParameterExist(int param)
{
   char tempStr[MAX_PATH];
   GetCmdLineParameter(param, tempStr); 
   if( strlen(tempStr) == 0 )
   {
      return FALSE;
   }
   return TRUE;
}
