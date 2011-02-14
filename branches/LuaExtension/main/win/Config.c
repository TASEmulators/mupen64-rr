/***************************************************************************
                          config.c  -  description
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

/*
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
*/
#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include "config.h"
#include "main_win.h"
#include "rombrowser.h"
#include "commandline.h"
#include "../../winproject/resource.h"

#define CfgFileName "mupen64.cfg"

extern int no_audio_delay;
extern int no_compiled_jump;

////////////////////// Service functions and structures ////////////////////////

CONFIG Config;

// is this the best way to handle this?
int *autoinc_save_slot = &Config.AutoIncSaveSlot;

char *CfgFilePath()
{
   static char *cfgpath = NULL;
   if (cfgpath == NULL)
     {
       	cfgpath = (char*)malloc(strlen(AppPath)+1+strlen(CfgFileName));
	    strcpy(cfgpath, AppPath);
    	strcat(cfgpath, CfgFileName);
     }
   return cfgpath;
}


void WriteCfgString   (char *Section,char *Key,char *Value) 
{
    WritePrivateProfileString( Section, Key, Value, CfgFilePath());
}


void WriteCfgInt      (char *Section,char *Key,int Value) 
{
    static char TempStr[20];
    sprintf( TempStr, "%d", Value);
    WriteCfgString( Section, Key, TempStr );
}


void ReadCfgString    (char *Section,char *Key,char *DefaultValue,char *retValue) 
{
    GetPrivateProfileString( Section, Key, DefaultValue, retValue, MAX_PATH, CfgFilePath());
}


int ReadCfgInt        (char *Section,char *Key,int DefaultValue) 
{
    return GetPrivateProfileInt( Section, Key, DefaultValue, CfgFilePath());
}

//////////////////////////// Load and Save Config //////////////////////////////

void LoadRecentRoms()
{
    int i;
    char tempStr[50];
    
    Config.RecentRomsFreeze = ReadCfgInt( "Recent Roms", "Freeze", 0 ) ;
    for ( i=0; i < MAX_RECENT_ROMS; i++)
    {
        
        sprintf( tempStr, "RecentRom%d", i) ; 
        ReadCfgString( "Recent Roms", tempStr, "", Config.RecentRoms[i]) ;
//        FILE* f = fopen(tempStr, "wb");
//        fputs(Config.RecentRoms[i], f);
//        fclose(f);
    }
  
}



void LoadConfig()
{
   LoadRecentRoms();
   
   // Language
    ReadCfgString( "Language", "Default", "English" , Config.DefaultLanguage);
       
   //Window position and size 
    Config.WindowWidth = ReadCfgInt("Window","Width",640); 
    Config.WindowHeight = ReadCfgInt("Window","Height",480); 
    Config.WindowPosX = ReadCfgInt("Window","X",(GetSystemMetrics( SM_CXSCREEN ) - Config.WindowWidth) / 2);
    Config.WindowPosY = ReadCfgInt("Window","Y",(GetSystemMetrics( SM_CYSCREEN ) - Config.WindowHeight) / 2);
    
    //General Vars
    Config.showFPS = ReadCfgInt("General","Show FPS",1);
    Config.showVIS = ReadCfgInt("General","Show VIS",1);
    Config.alertBAD = ReadCfgInt("General","Alert Bad roms",1);
    Config.alertHACK = ReadCfgInt("General","Alert Hacked roms",1);
    Config.savesERRORS = ReadCfgInt("General","Alert Saves errors",1);
    Config.limitFps = ReadCfgInt("General","Alert Saves errors",1);
    Config.compressedIni = ReadCfgInt("General","Compressed Ini",1);
    Config.UseFPSmodifier = ReadCfgInt("General","Use Fps Modifier",1);
    Config.FPSmodifier = ReadCfgInt("General","Fps Modifier",100);
    
    
    Config.guiDynacore = ReadCfgInt("CPU","Core",1);
    
    
    
    //Advanced vars
    Config.StartFullScreen = ReadCfgInt("Advanced","Start Full Screen",0);
    Config.PauseWhenNotActive = ReadCfgInt("Advanced","Pause when not active",1);
    Config.OverwritePluginSettings = ReadCfgInt("Advanced","Overwrite Plugins Settings ",0);
    Config.GuiToolbar = ReadCfgInt( "Advanced", "Use Toolbar", 0);
    Config.GuiStatusbar = ReadCfgInt( "Advanced", "Use Statusbar", 1);
    Config.AutoIncSaveSlot = ReadCfgInt( "Advanced", "Auto Increment Save Slot", 0);
    
    //Compatibility Settings
    no_audio_delay = ReadCfgInt("Compatibility","No Audio Delay", 0);
    no_compiled_jump = ReadCfgInt("Compatibility","No Compiled Jump", 0);
    
    //RomBrowser Columns
    Config.Column_GoodName = ReadCfgInt("Rom Browser Columns","Good Name", 1);
    Config.Column_InternalName = ReadCfgInt("Rom Browser Columns","Internal Name", 0);
    Config.Column_Country = ReadCfgInt("Rom Browser Columns","Country", 1);
    Config.Column_Size = ReadCfgInt("Rom Browser Columns","Size", 1);
    Config.Column_Comments = ReadCfgInt("Rom Browser Columns","Comments", 1);
    Config.Column_FileName = ReadCfgInt("Rom Browser Columns","File Name", 0);
    Config.Column_MD5 = ReadCfgInt("Rom Browser Columns","MD5", 0);
    
      // Directories
    Config.DefaultPluginsDir = ReadCfgInt("Directories","Default Plugins Directory",1);
    Config.DefaultSavesDir = ReadCfgInt("Directories","Default Saves Directory",1);
    Config.DefaultScreenshotsDir = ReadCfgInt("Directories","Default Screenshots Directory",1);
    
    sprintf(Config.PluginsDir,"%sPlugin\\",AppPath);
    ReadCfgString("Directories","Plugins Directory",Config.PluginsDir, Config.PluginsDir);
    
    sprintf(Config.SavesDir,"%sSave\\",AppPath);
    ReadCfgString("Directories","Saves Directory",Config.SavesDir, Config.SavesDir);
    
    sprintf(Config.ScreenshotsDir,"%sScreenShots\\",AppPath);
    ReadCfgString("Directories","Screenshots Directory",Config.ScreenshotsDir, Config.ScreenshotsDir);    
    
    
    
    // Rom Browser
    Config.RomBrowserSortColumn = ReadCfgInt( "Rom Browser", "Sort Column" , 0);
    Config.RomBrowserRecursion = ReadCfgInt("Rom Browser","Recursion",0);
    ReadCfgString("Rom Browser","Sort Method","ASC",Config.RomBrowserSortMethod);

	// Load A Whole Whackton Of Hotkeys:

    Config.hotkey[0].key = ReadCfgInt("Hotkeys","Fast Forward Key",VK_TAB);
    Config.hotkey[0].shift = ReadCfgInt("Hotkeys","Fast Forward Shift",0);
    Config.hotkey[0].ctrl = ReadCfgInt("Hotkeys","Fast Forward Ctrl",0);
    Config.hotkey[0].alt = ReadCfgInt("Hotkeys","Fast Forward Alt",0);
    Config.hotkey[0].command = 0; // handled specially

    Config.hotkey[1].key = ReadCfgInt("Hotkeys","Speed Up Key",/*VK_OEM_PLUS*/0xBB);
    Config.hotkey[1].shift = ReadCfgInt("Hotkeys","Speed Up Shift",0);
    Config.hotkey[1].ctrl = ReadCfgInt("Hotkeys","Speed Up Ctrl",0);
    Config.hotkey[1].alt = ReadCfgInt("Hotkeys","Speed Up Alt",0);
	Config.hotkey[1].command = IDC_INCREASE_MODIFIER;

    Config.hotkey[2].key = ReadCfgInt("Hotkeys","Slow Down Key",/*VK_OEM_MINUS*/0xBD);
    Config.hotkey[2].shift = ReadCfgInt("Hotkeys","Slow Down Shift",0);
    Config.hotkey[2].ctrl = ReadCfgInt("Hotkeys","Slow Down Ctrl",0);
    Config.hotkey[2].alt = ReadCfgInt("Hotkeys","Slow Down Alt",0);
	Config.hotkey[2].command = IDC_DECREASE_MODIFIER;

    Config.hotkey[3].key = ReadCfgInt("Hotkeys","Frame Advance Key",VK_OEM_5);
    Config.hotkey[3].shift = ReadCfgInt("Hotkeys","Frame Advance Shift",0);
    Config.hotkey[3].ctrl = ReadCfgInt("Hotkeys","Frame Advance Ctrl",0);
    Config.hotkey[3].alt = ReadCfgInt("Hotkeys","Frame Advance Alt",0);
	Config.hotkey[3].command = EMU_FRAMEADVANCE;

    Config.hotkey[4].key = ReadCfgInt("Hotkeys","Pause Resume Key",VK_PAUSE);
    Config.hotkey[4].shift = ReadCfgInt("Hotkeys","Pause Resume Shift",0);
    Config.hotkey[4].ctrl = ReadCfgInt("Hotkeys","Pause Resume Ctrl",0);
    Config.hotkey[4].alt = ReadCfgInt("Hotkeys","Pause Resume Alt",0);
	Config.hotkey[4].command = EMU_PAUSE;

    Config.hotkey[5].key = ReadCfgInt("Hotkeys","ReadOnly Key",'8');
    Config.hotkey[5].shift = ReadCfgInt("Hotkeys","ReadOnly Shift",1);
    Config.hotkey[5].ctrl = ReadCfgInt("Hotkeys","ReadOnly Ctrl",0);
    Config.hotkey[5].alt = ReadCfgInt("Hotkeys","ReadOnly Alt",0);
	Config.hotkey[5].command = EMU_VCRTOGGLEREADONLY;

    Config.hotkey[6].key = ReadCfgInt("Hotkeys","Play Key",'P');
    Config.hotkey[6].shift = ReadCfgInt("Hotkeys","Play Shift",1);
    Config.hotkey[6].ctrl = ReadCfgInt("Hotkeys","Play Ctrl",1);
    Config.hotkey[6].alt = ReadCfgInt("Hotkeys","Play Alt",0);
	Config.hotkey[6].command = ID_START_PLAYBACK;

    Config.hotkey[7].key = ReadCfgInt("Hotkeys","PlayStop Key",'S');
    Config.hotkey[7].shift = ReadCfgInt("Hotkeys","PlayStop Shift",1);
    Config.hotkey[7].ctrl = ReadCfgInt("Hotkeys","PlayStop Ctrl",1);
    Config.hotkey[7].alt = ReadCfgInt("Hotkeys","PlayStop Alt",0);
	Config.hotkey[7].command = ID_STOP_PLAYBACK;

    Config.hotkey[8].key = ReadCfgInt("Hotkeys","Record Key",'R');
    Config.hotkey[8].shift = ReadCfgInt("Hotkeys","Record Shift",1);
    Config.hotkey[8].ctrl = ReadCfgInt("Hotkeys","Record Ctrl",1);
    Config.hotkey[8].alt = ReadCfgInt("Hotkeys","Record Alt",0);
	Config.hotkey[8].command = ID_START_RECORD;

    Config.hotkey[9].key = ReadCfgInt("Hotkeys","RecordStop Key",'S');
    Config.hotkey[9].shift = ReadCfgInt("Hotkeys","RecordStop Shift",1);
    Config.hotkey[9].ctrl = ReadCfgInt("Hotkeys","RecordStop Ctrl",1);
    Config.hotkey[9].alt = ReadCfgInt("Hotkeys","RecordStop Alt",0);
	Config.hotkey[9].command = ID_STOP_RECORD;

    Config.hotkey[10].key = ReadCfgInt("Hotkeys","Screenshot Key",VK_F12);
    Config.hotkey[10].shift = ReadCfgInt("Hotkeys","Screenshot Shift",0);
    Config.hotkey[10].ctrl = ReadCfgInt("Hotkeys","Screenshot Ctrl",0);
    Config.hotkey[10].alt = ReadCfgInt("Hotkeys","Screenshot Alt",0);
	Config.hotkey[10].command = GENERATE_BITMAP;

    Config.hotkey[11].key = ReadCfgInt("Hotkeys","Save Current Key",VK_F5);
    Config.hotkey[11].shift = ReadCfgInt("Hotkeys","Save Current Shift",0);
    Config.hotkey[11].ctrl = ReadCfgInt("Hotkeys","Save Current Ctrl",1);
    Config.hotkey[11].alt = ReadCfgInt("Hotkeys","Save Current Alt",0);
	Config.hotkey[11].command = STATE_SAVE;

    Config.hotkey[12].key = ReadCfgInt("Hotkeys","Load Current Key",VK_F7);
    Config.hotkey[12].shift = ReadCfgInt("Hotkeys","Load Current Shift",0);
    Config.hotkey[12].ctrl = ReadCfgInt("Hotkeys","Load Current Ctrl",1);
    Config.hotkey[12].alt = ReadCfgInt("Hotkeys","Load Current Alt",0);
	Config.hotkey[12].command = STATE_RESTORE;

    // Save/Load Hotkeys
    {
		int i;
	    char str [128];
	    
	    for(i = 1 ; i <= 9 ; i++)
	    {
			sprintf(str, "Save %d Key", i);
		    Config.hotkey[12+i].key = ReadCfgInt("Hotkeys",str,(VK_F1-1)+i);
			sprintf(str, "Save %d Shift", i);
		    Config.hotkey[12+i].shift = ReadCfgInt("Hotkeys",str,1);
			sprintf(str, "Save %d Ctrl", i);
		    Config.hotkey[12+i].ctrl = ReadCfgInt("Hotkeys",str,0);
			sprintf(str, "Save %d Alt", i);
		    Config.hotkey[12+i].alt = ReadCfgInt("Hotkeys",str,0);
		    Config.hotkey[12+i].command = (ID_SAVE_1-1) + i;
		}
	    for(i = 1 ; i <= 9 ; i++)
	    {
			sprintf(str, "Load %d Key", i);
		    Config.hotkey[12+9+i].key = ReadCfgInt("Hotkeys",str,(VK_F1-1)+i);
			sprintf(str, "Load %d Shift", i);
		    Config.hotkey[12+9+i].shift = ReadCfgInt("Hotkeys",str,0);
			sprintf(str, "Load %d Ctrl", i);
		    Config.hotkey[12+9+i].ctrl = ReadCfgInt("Hotkeys",str,0);
			sprintf(str, "Load %d Alt", i);
		    Config.hotkey[12+9+i].alt = ReadCfgInt("Hotkeys",str,0);
		    Config.hotkey[12+9+i].command = (ID_LOAD_1-1) + i;
		}
	    for(i = 1 ; i <= 9 ; i++)
	    {
			sprintf(str, "Select %d Key", i);
		    Config.hotkey[12+9+9+i].key = ReadCfgInt("Hotkeys",str,'0'+i);
			sprintf(str, "Select %d Shift", i);
		    Config.hotkey[12+9+9+i].shift = ReadCfgInt("Hotkeys",str,0);
			sprintf(str, "Select %d Ctrl", i);
		    Config.hotkey[12+9+9+i].ctrl = ReadCfgInt("Hotkeys",str,0);
			sprintf(str, "Select %d Alt", i);
		    Config.hotkey[12+9+9+i].alt = ReadCfgInt("Hotkeys",str,0);
		    Config.hotkey[12+9+9+i].command = (ID_CURRENTSAVE_1-1) + i;
		}
	}
  
}

/////////////////////////////////////////////////////////////////////////////////

void saveWindowSettings()
{
    RECT rcMain;
    GetWindowRect(mainHWND, &rcMain);
	Config.WindowPosX=rcMain.left;
	Config.WindowPosY=rcMain.top;
	Config.WindowWidth = rcMain.right - rcMain.left;
	Config.WindowHeight = rcMain.bottom - rcMain.top;
	WriteCfgInt("Window","Width",Config.WindowWidth); 
    WriteCfgInt("Window","Height",Config.WindowHeight); 
    WriteCfgInt("Window","X",Config.WindowPosX);
    WriteCfgInt("Window","Y",Config.WindowPosY);
}

void saveBrowserSettings()
{
    int Column,ColWidth,index;
    index=0;
    for (Column = 0; Column < ROM_COLUMN_FIELDS; Column ++) {
        if ( isFieldInBrowser( Column) ) {
        ColWidth = ListView_GetColumnWidth(hRomList,index);
        WriteCfgInt("Rom Browser",getFieldName(Column),ColWidth);
        index++;
        }
    }
    
    WriteCfgInt( "Rom Browser", "Sort Column", Config.RomBrowserSortColumn);
    WriteCfgInt( "Rom Browser", "Recursion", Config.RomBrowserRecursion);
    WriteCfgString("Rom Browser","Sort Method", Config.RomBrowserSortMethod);
      
}

void SaveRecentRoms()
{
    int i;
    char tempStr[50];
    
    WriteCfgInt( "Recent Roms", "Freeze", Config.RecentRomsFreeze) ;
    for ( i=0; i < MAX_RECENT_ROMS; i++)
    {
        sprintf( tempStr, "RecentRom%d", i) ; 
        WriteCfgString( "Recent Roms", tempStr, Config.RecentRoms[i]) ;
    }
}

void SaveConfig()
{
    saveWindowSettings();
    
    if (!cmdlineNoGui) {
        saveBrowserSettings();
        SaveRecentRoms();
    }    
    
    //Language
    WriteCfgString( "Language", "Default", Config.DefaultLanguage);
    
    //General Vars
    WriteCfgInt( "General", "Show FPS", Config.showFPS);
    WriteCfgInt( "General", "Show VIS", Config.showVIS);
    WriteCfgInt( "General", "Alert Bad roms", Config.alertBAD);
    WriteCfgInt( "General", "Alert Hacked roms", Config.alertHACK);
    WriteCfgInt( "General", "Alert Saves errors", Config.savesERRORS);
    WriteCfgInt( "General", "Alert Saves errors", Config.limitFps);
    WriteCfgInt( "General", "Compressed Ini", Config.compressedIni);
    WriteCfgInt( "General", "Fps Modifier", Config.FPSmodifier);
    WriteCfgInt( "General", "Use Fps Modifier", Config.UseFPSmodifier);
    
    //Advanced Vars
    WriteCfgInt( "Advanced", "Start Full Screen", Config.StartFullScreen);
    WriteCfgInt( "Advanced", "Pause when not active", Config.PauseWhenNotActive);
    WriteCfgInt( "Advanced", "Overwrite Plugins Settings", Config.OverwritePluginSettings);
    WriteCfgInt( "Advanced", "Use Toolbar", Config.GuiToolbar);
    WriteCfgInt( "Advanced", "Use Statusbar", Config.GuiStatusbar);
    WriteCfgInt( "Advanced", "Auto Increment Save Slot", Config.AutoIncSaveSlot);
    
    WriteCfgInt( "CPU", "Core", Config.guiDynacore);
    
    //Compatibility Settings
    WriteCfgInt( "Compatibility", "No Audio Delay", no_audio_delay);
    WriteCfgInt( "Compatibility", "No Compiled Jump", no_compiled_jump);
    
    //Rom Browser columns
    WriteCfgInt("Rom Browser Columns","Good Name", Config.Column_GoodName);
    WriteCfgInt("Rom Browser Columns","Internal Name", Config.Column_InternalName);
    WriteCfgInt("Rom Browser Columns","Country", Config.Column_Country);
    WriteCfgInt("Rom Browser Columns","Size", Config.Column_Size);
    WriteCfgInt("Rom Browser Columns","Comments", Config.Column_Comments);
    WriteCfgInt("Rom Browser Columns","File Name", Config.Column_FileName);
    WriteCfgInt("Rom Browser Columns","MD5", Config.Column_MD5);
    
    // Directories
    WriteCfgInt( "Directories", "Default Plugins Directory", Config.DefaultPluginsDir);
    WriteCfgString("Directories","Plugins Directory",Config.PluginsDir);
    WriteCfgInt( "Directories","Default Saves Directory", Config.DefaultSavesDir);
    WriteCfgString("Directories","Saves Directory",Config.SavesDir);
    WriteCfgInt( "Directories", "Default Screenshots Directory", Config.DefaultScreenshotsDir);
    WriteCfgString("Directories","Screenshots Directory",Config.ScreenshotsDir);    


	// Load A Whole Whackton Of Hotkeys:

    WriteCfgInt("Hotkeys","Fast Forward Key",Config.hotkey[0].key);
    WriteCfgInt("Hotkeys","Fast Forward Shift",Config.hotkey[0].shift);
    WriteCfgInt("Hotkeys","Fast Forward Ctrl",Config.hotkey[0].ctrl);
    WriteCfgInt("Hotkeys","Fast Forward Alt",Config.hotkey[0].alt);

    WriteCfgInt("Hotkeys","Speed Up Key",Config.hotkey[1].key);
    WriteCfgInt("Hotkeys","Speed Up Shift",Config.hotkey[1].shift);
    WriteCfgInt("Hotkeys","Speed Up Ctrl",Config.hotkey[1].ctrl);
    WriteCfgInt("Hotkeys","Speed Up Alt",Config.hotkey[1].alt);
  
    WriteCfgInt("Hotkeys","Slow Down Key",Config.hotkey[2].key);
    WriteCfgInt("Hotkeys","Slow Down Shift",Config.hotkey[2].shift);
    WriteCfgInt("Hotkeys","Slow Down Ctrl",Config.hotkey[2].ctrl);
    WriteCfgInt("Hotkeys","Slow Down Alt",Config.hotkey[2].alt);

    WriteCfgInt("Hotkeys","Frame Advance Key",Config.hotkey[3].key);
    WriteCfgInt("Hotkeys","Frame Advance Shift",Config.hotkey[3].shift);
    WriteCfgInt("Hotkeys","Frame Advance Ctrl",Config.hotkey[3].ctrl);
    WriteCfgInt("Hotkeys","Frame Advance Alt",Config.hotkey[3].alt);

    WriteCfgInt("Hotkeys","Pause Resume Key",Config.hotkey[4].key);
    WriteCfgInt("Hotkeys","Pause Resume Shift",Config.hotkey[4].shift);
    WriteCfgInt("Hotkeys","Pause Resume Ctrl",Config.hotkey[4].ctrl);
    WriteCfgInt("Hotkeys","Pause Resume Alt",Config.hotkey[4].alt);

    WriteCfgInt("Hotkeys","ReadOnly Key",Config.hotkey[5].key);
    WriteCfgInt("Hotkeys","ReadOnly Shift",Config.hotkey[5].shift);
    WriteCfgInt("Hotkeys","ReadOnly Ctrl",Config.hotkey[5].ctrl);
    WriteCfgInt("Hotkeys","ReadOnly Alt",Config.hotkey[5].alt);

    WriteCfgInt("Hotkeys","Play Key",Config.hotkey[6].key);
    WriteCfgInt("Hotkeys","Play Shift",Config.hotkey[6].shift);
    WriteCfgInt("Hotkeys","Play Ctrl",Config.hotkey[6].ctrl);
    WriteCfgInt("Hotkeys","Play Alt",Config.hotkey[6].alt);

    WriteCfgInt("Hotkeys","PlayStop Key",Config.hotkey[7].key);
    WriteCfgInt("Hotkeys","PlayStop Shift",Config.hotkey[7].shift);
    WriteCfgInt("Hotkeys","PlayStop Ctrl",Config.hotkey[7].ctrl);
    WriteCfgInt("Hotkeys","PlayStop Alt",Config.hotkey[7].alt);

    WriteCfgInt("Hotkeys","Record Key",Config.hotkey[8].key);
    WriteCfgInt("Hotkeys","Record Shift",Config.hotkey[8].shift);
    WriteCfgInt("Hotkeys","Record Ctrl",Config.hotkey[8].ctrl);
    WriteCfgInt("Hotkeys","Record Alt",Config.hotkey[8].alt);

    WriteCfgInt("Hotkeys","RecordStop Key",Config.hotkey[9].key);
    WriteCfgInt("Hotkeys","RecordStop Shift",Config.hotkey[9].shift);
    WriteCfgInt("Hotkeys","RecordStop Ctrl",Config.hotkey[9].ctrl);
    WriteCfgInt("Hotkeys","RecordStop Alt",Config.hotkey[9].alt);

    WriteCfgInt("Hotkeys","Screenshot Key",Config.hotkey[10].key);
    WriteCfgInt("Hotkeys","Screenshot Shift",Config.hotkey[10].shift);
    WriteCfgInt("Hotkeys","Screenshot Ctrl",Config.hotkey[10].ctrl);
    WriteCfgInt("Hotkeys","Screenshot Alt",Config.hotkey[10].alt);

    WriteCfgInt("Hotkeys","Save Current Key",Config.hotkey[11].key);
    WriteCfgInt("Hotkeys","Save Current Shift",Config.hotkey[11].shift);
    WriteCfgInt("Hotkeys","Save Current Ctrl",Config.hotkey[11].ctrl);
    WriteCfgInt("Hotkeys","Save Current Alt",Config.hotkey[11].alt);
  
    WriteCfgInt("Hotkeys","Load Current Key",Config.hotkey[12].key);
    WriteCfgInt("Hotkeys","Load Current Shift",Config.hotkey[12].shift);
    WriteCfgInt("Hotkeys","Load Current Ctrl",Config.hotkey[12].ctrl);
    WriteCfgInt("Hotkeys","Load Current Alt",Config.hotkey[12].alt);
      
    // Save/Load Hotkeys
    {
		int i;
	    char str [128];
	    
	    for(i = 1 ; i <= 9 ; i++)
	    {
			sprintf(str, "Save %d Key", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+i].key);
			sprintf(str, "Save %d Shift", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+i].shift);
			sprintf(str, "Save %d Ctrl", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+i].ctrl);
			sprintf(str, "Save %d Alt", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+i].alt);
		}
	    for(i = 1 ; i <= 9 ; i++)
	    {
			sprintf(str, "Load %d Key", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+9+i].key);
			sprintf(str, "Load %d Shift", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+9+i].shift);
			sprintf(str, "Load %d Ctrl", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+9+i].ctrl);
			sprintf(str, "Load %d Alt", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+9+i].alt);
		}
	    for(i = 1 ; i <= 9 ; i++)
	    {
			sprintf(str, "Select %d Key", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+9+9+i].key);
			sprintf(str, "Select %d Shift", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+9+9+i].shift);
			sprintf(str, "Select %d Ctrl", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+9+9+i].ctrl);
			sprintf(str, "Select %d Alt", i);
		    WriteCfgInt("Hotkeys",str,Config.hotkey[12+9+9+i].alt);
		}
	}
}


