/***************************************************************************
                          guifuncs.c  -  description
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

#define _WIN32_IE 0x0500
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "../guifuncs.h"
#include "main_win.h"
#include "translation.h"
#include "rombrowser.h"
#include "../../winproject/resource.h"

char *get_currentpath()
{
    return AppPath;
}

char *get_savespath()
{
    static char defDir[MAX_PATH];
    if (Config.DefaultSavesDir) {
        sprintf(defDir,"%sSave\\",AppPath);
        return defDir;
    }
    else return Config.SavesDir;
}

void display_loading_progress(int p)
{
   sprintf(TempMessage,"%d%%",p);
   //SendMessage( hStatus, SB_SETTEXT, 1, (LPARAM)TempMessage );  
   SendMessage( hStatusProgress, PBM_SETPOS, p+1, 0 );
}

void warn_savestate_not_exist()
{
   if (!Config.savesERRORS) return;
   TranslateDefault("Savestates Wrong Slot","You have selected wrong save slot or save doesn't exist",TempMessage);
   SendMessage( hStatus, SB_SETTEXT, 0, (LPARAM)TempMessage ); 
  
}

void warn_savestate_from_another_rom()
{
   if (!Config.savesERRORS) return;
   TranslateDefault("Savestates Wrong Region","This savestate is from another ROM or version",TempMessage);
   SendMessage( hStatus, SB_SETTEXT, 0, (LPARAM)TempMessage ); 
  
}

void display_MD5calculating_progress(int p )
{
   if (romInfoHWND!=NULL) {
    if (p>0) {
        sprintf(TempMessage,"%d%%",p);                                 
        SetDlgItemText( romInfoHWND, IDC_MD5_PROGRESS, TempMessage );
        SendMessage( GetDlgItem(romInfoHWND, IDC_CURRENT_ROM_PROGRESS), PBM_SETPOS, p+1, 0 );
        SendMessage( GetDlgItem(romInfoHWND, IDC_MD5_PROGRESS_BAR), PBM_SETPOS, p+1, 0 );
        }
     else
       {
        SetDlgItemText( romInfoHWND, IDC_MD5_PROGRESS, "" );           
        SendMessage( GetDlgItem(romInfoHWND, IDC_CURRENT_ROM_PROGRESS), PBM_SETPOS, 0, 0 );
        SendMessage( GetDlgItem(romInfoHWND, IDC_MD5_PROGRESS_BAR), PBM_SETPOS, 0, 0 );
       }   
   }    
}

int ask_bad()
{
    if (!Config.alertBAD) return 1;
    
    if (MessageBox(NULL,"This rom is a bad dump, do you want to continue?","Warning",MB_YESNO | MB_ICONWARNING) != IDNO)
            return 1;
    else 
            return 0;            
}

int ask_hack()
{
    if (!Config.alertHACK) return 1;
    
    if (MessageBox(NULL,"This rom is a hacked dump, do you want to continue?","Question",MB_YESNO | MB_ICONQUESTION) != IDNO)
            return 1;
    else
            return 0;
}

void display_status(const char* status)
{
   SendMessage( hStatus, SB_SETTEXT, 0, (LPARAM)status ); 
}
