/***************************************************************************
                          configdialog.h  -  description
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


#include <Windows.h>

void ChangeSettings(HWND hwndOwner);
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DirectoriesCfg(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK PluginsCfg(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AuditDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK LangInfoProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK GeneralCfg(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AdvancedSettingsProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK HotkeysProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


void WriteCheckBoxValue( HWND hwnd, int resourceID , int value);
int ReadCheckBoxValue( HWND hwnd, int resourceID);
void WriteComboBoxValue(HWND hwnd,int ResourceID,char *PrimaryVal,char *DefaultVal);
void ReadComboBoxValue(HWND hwnd,int ResourceID,char *ret);
