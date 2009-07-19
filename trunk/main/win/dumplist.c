/***************************************************************************
                          dumplist.c  -  description
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


#include <stdio.h>
#include <stdarg.h>
#include "rombrowser.h"
#include "dumplist.h"
#include "../../winproject/resource.h"
#include "main_win.h"
//#include "process.h"

FILE  *dumpfile;

void OpenRomListFile() {
    dumpfile = fopen(RomListFileName, "w");

}


void PrintToRomListFile ( char * ftext  , ...   ) {
    va_list ap;
    va_start (ap, ftext);
    vfprintf (dumpfile, ftext, ap);
    va_end (ap);
 
}


void CloseRomListFile() {
    fclose( dumpfile ) ;

}


void generateRomInfo()
{
    int i;   
    ROM_INFO *pRomInfo;
    int itemsNum;
    LV_ITEM lvItem;
    memset(&lvItem, 0, sizeof(LV_ITEM));
    OpenRomListFile();
       PrintToRomListFile("%s\n----------------------------------\n\n",MUPEN_VERSION);
 
    itemsNum = ListView_GetItemCount(hRomList);
    for(i=0;i<itemsNum;i++)
    {
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = i;
        ListView_GetItem(hRomList,&lvItem);
        pRomInfo = &ItemList.List[lvItem.lParam];
        PrintToRomListFile("%-60s %s\n",pRomInfo->GoodName,pRomInfo->UserNotes);
    } 
        
    CloseRomListFile();
    //ShowMessage("File romlist.txt created");
    if (MessageBox(NULL,"File romlist.txt created. Do you want to open it?","ROM Info",MB_YESNO | MB_ICONQUESTION) != IDNO)
    {    
    //    _spawnlp( _P_NOWAIT, "notepad.exe", "notepad.exe", RomListFileName, NULL );
        sprintf(TempMessage,"%s%s",AppPath,RomListFileName);
        ShellExecute(NULL, "open", TempMessage, NULL, NULL, SW_SHOWNORMAL);           
    }

}
