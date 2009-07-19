/***************************************************************************
                          RomSettings.c  -  description
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

#include <windows.h>
#include <ctype.h>
#include "main_win.h"
#include "RomSettings.h"

#define ROM_SETIINGS_FILE "romsettings.ini"

char *trim (char *str)
{
      char *ibuf, *obuf;

      if (str)
      {
            for (ibuf = obuf = str; *ibuf; )
            {
                  while (*ibuf && (isspace (*ibuf)))
                        ibuf++;
                  if (*ibuf && (obuf != str))
                        *(obuf++) = ' ';
                  while (*ibuf && (!isspace (*ibuf)))
                        *(obuf++) = *(ibuf++);
            }
            *obuf = 0;
      }
      return (str);
}


char *RomSettingsFilePath()
{
   static char *cfgpath = NULL;
   if (cfgpath == NULL)
     {
       	cfgpath = malloc(strlen(AppPath)+1+strlen(ROM_SETIINGS_FILE));
	    strcpy(cfgpath, AppPath);
    	strcat(cfgpath, ROM_SETIINGS_FILE);
     }
   return cfgpath;
}

DEFAULT_ROM_SETTINGS GetDefaultRomSettings(char *Section) 
{
    static DEFAULT_ROM_SETTINGS DefaultRomSettings;
    
    trim(Section);
    GetPrivateProfileString( Section, "Graphics", "", DefaultRomSettings.GfxPluginName,    100, RomSettingsFilePath());
    GetPrivateProfileString( Section, "Input",    "", DefaultRomSettings.InputPluginName , 100, RomSettingsFilePath());
    GetPrivateProfileString( Section, "Sound",    "", DefaultRomSettings.SoundPluginName,  100, RomSettingsFilePath());
    GetPrivateProfileString( Section, "RSP",      "", DefaultRomSettings.RspPluginName,    100, RomSettingsFilePath());
    
    return DefaultRomSettings;
}

void saveDefaultRomSettings(char *Section, DEFAULT_ROM_SETTINGS DefaultRomSettings)
{
    trim(Section);
    WritePrivateProfileString( Section, "Graphics",  DefaultRomSettings.GfxPluginName,    RomSettingsFilePath());
    WritePrivateProfileString( Section, "Input",     DefaultRomSettings.InputPluginName,  RomSettingsFilePath());
    WritePrivateProfileString( Section, "Sound",     DefaultRomSettings.SoundPluginName,  RomSettingsFilePath());
    WritePrivateProfileString( Section, "RSP",       DefaultRomSettings.RspPluginName,    RomSettingsFilePath());
}
