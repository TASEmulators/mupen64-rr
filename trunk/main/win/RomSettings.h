/***************************************************************************
                          RomSettings.h  -  description
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

typedef struct _DEFAULT_ROM_SETTINGS {
     unsigned char Version ;
     char GfxPluginName[100];
     char InputPluginName[100];
     char SoundPluginName[100];
     char RspPluginName[100];
    
} DEFAULT_ROM_SETTINGS;

DEFAULT_ROM_SETTINGS GetDefaultRomSettings(char *Section);
void saveDefaultRomSettings(char *Section, DEFAULT_ROM_SETTINGS DefaultRomSettings);
