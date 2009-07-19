/***************************************************************************
                          kaillera.c  -  description
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
#include "kaillera.h"
#include "main_win.h"
#include "Gui_logWindow.h"
#include "rombrowser.h"
#include "../../winproject/resource.h"

static HMODULE KailleraHandle = NULL;
static char	szKailleraNamedRoms[50 * 2000];

DWORD WINAPI KailleraThread(LPVOID lpParam) 
{
    LoadKaillera();
    ShowInfo("Kaillera thread : Kaillera Loaded") ;
    while (KailleraHandle) {
        
    }
    return 0;
}

int isKailleraExist()
{
    static int KailleraExist = -1 ;
    char TempStr[MAX_PATH];
    if (KailleraExist<0) {    
        sprintf(TempStr,"%s%s",AppPath,"kailleraclient.dll") ;
        KailleraHandle = LoadLibrary(TempStr);
        if (KailleraHandle) {
          KailleraExist = 1 ; 
          FreeLibrary(KailleraHandle) ;
          KailleraHandle = NULL ;
        }
        else {
          KailleraExist = 0 ; 
        }
    }    
    return KailleraExist;
}

int LoadKaillera()
{
    
    char TempStr[MAX_PATH];
        
    sprintf(TempStr,"%s%s",AppPath,"kailleraclient.dll") ; 
      
    KailleraHandle = LoadLibrary(TempStr);
    
    if (KailleraHandle) {
      ShowInfo("Kaillera Library found") ;
      kailleraGetVersion = (void (__stdcall* ) ()) GetProcAddress( KailleraHandle, "_kailleraGetVersion@4");
      if (kailleraGetVersion==NULL) {
         ShowError("kailleraGetVersion not found") ;
         return 0 ;
      }   
      
      kailleraInit = (void (__stdcall *)(void)) GetProcAddress( KailleraHandle, "_kailleraInit@0");
      if (kailleraInit==NULL) {
         ShowError("kailleraInit not found") ;
         return 0  ;
      } 
      
      kailleraShutdown = (void (__stdcall *) (void)) GetProcAddress(KailleraHandle, "_kailleraShutdown@0");
	  if(kailleraShutdown == NULL) {
	     ShowError("kailleraShutdown not found") ;
         return 0  ;
      }
      
      kailleraSetInfos = (void(__stdcall *) (kailleraInfos *)) GetProcAddress(KailleraHandle, "_kailleraSetInfos@4");
      if(kailleraSetInfos == NULL) {
	     ShowError("kailleraSetInfos not found") ;
         return 0  ;
      } 
      
      kailleraSelectServerDialog = (void (__stdcall* ) (HWND parent)) GetProcAddress(KailleraHandle, "_kailleraSelectServerDialog@4");
      if (kailleraSelectServerDialog == NULL) {
        ShowError("kailleraSelectServerDialog not found"); 
        return 0  ;   
      }
      
      kailleraModifyPlayValues = (void (__stdcall *) (void *values, int size)) GetProcAddress (	KailleraHandle,"_kailleraModifyPlayValues@8");
	  if(kailleraModifyPlayValues == NULL) {
	    ShowError("kailleraModifyPlayValues not found");
        return 0  ;
      }
      
      kailleraChatSend = (void(__stdcall *) (char *)) GetProcAddress( KailleraHandle, "_kailleraChatSend@4");
	  if(kailleraChatSend == NULL) {
	    ShowError("kailleraChatSend not found");
        return 0  ;
      }

      kailleraEndGame = (void (__stdcall *) (void)) GetProcAddress( KailleraHandle, "_kailleraEndGame@0");
	  if(kailleraEndGame == NULL) {
	    ShowError("kailleraEndGame not found");
	    return 0 ;
      }
      kailleraGetVersion(TempStr);
      ShowInfo( "Kaillera version %s", TempStr);
      ShowInfo( "Starting Kaillera...") ;
      KailleraPlay() ;
      return 1;        
   }
   else {
      ShowError("Kaillera Library file 'kailleraclient.dll' not found ") ;
      return 0;
    }
}

void CloseKaillera()
{
    ShowInfo("Kaillera : kailleraShutdown()");
    if (kailleraShutdown) kailleraShutdown();
    if (KailleraHandle) FreeLibrary(KailleraHandle) ;
    KailleraHandle = NULL ;
}

void EndGameKaillera()
{
    if (KailleraHandle) { 
       ShowInfo("Kaillera : kailleraEndGame()");
       kailleraEndGame();
    }   
}

/// Used Parts of 1964 Kaillera code as referece,not finished

int WINAPI kailleraGameCallback(char *game, int player, int numplayers)
{

    int i;
    ROM_INFO *pRomInfo;
    
    ShowInfo("Kaillera : kailleraGameCallback()");
    for (i=0;i<ItemList.ListCount;i++)
    {
        pRomInfo = &ItemList.List[i];
        if (strcmp(pRomInfo->GoodName, game) == 0) {
          ShowInfo("Kaillera : Starting rom %s",pRomInfo->GoodName);
          StartRom(pRomInfo->szFullFileName);
          SetForegroundWindow( mainHWND) ;
          SetFocus( mainHWND) ;
          ShowWindow( mainHWND, SW_SHOW) ;
          return 0;
        }
    }
    
/*	//~~//
	int i;
	//~~//

	Kaillera_Is_Running = TRUE;
	Kaillera_Players = numplayers;
	Kaillera_Counter = 0;

	for(i = 0; i < romlist_count; i++)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
		char			szRom[50];
		ROMLIST_ENTRY	*entry = romlist[i];
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~////

		sprintf
		(
			szRom,
			"%s (%X-%X:%c)",
			entry->pinientry->Game_Name,
			entry->pinientry->crc1,
			entry->pinientry->crc2,
			entry->pinientry->countrycode
		);

		if(strcmp(szRom, game) == 0)
		{
			RomListOpenRom(i, 1);
		}
	}

	return 0;
	*/
	return 0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void WINAPI kailleraChatReceivedCallback(char *nick, char *text)
{
	/* Do what you want with this :) */
	ShowInfo("Kaillera : <%s> : %s",nick,text) ;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void WINAPI kailleraClientDroppedCallback(char *nick, int playernb)
{
	/* Do what you want with this :) */
	ShowInfo("Kaillera : <%s> dropped (%d)",nick,playernb) ;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void WINAPI kailleraMoreInfosCallback(char *gamename)
{
	/* Do what you want with this :) */
	ShowInfo("Kaillera : MoreInfosCallback %s ", gamename) ;
}

void buildRomList() {
    
    int i;
    int sort_column;
    char sort_method[10];
    ROM_INFO *pRomInfo;
    int itemsNum;
    LV_ITEM lvItem;
    memset(&lvItem, 0, sizeof(LV_ITEM));
    char *pszKailleraNamedRoms = szKailleraNamedRoms;
    *pszKailleraNamedRoms = '\0';
    
    sprintf( sort_method, Config.RomBrowserSortMethod) ;
    sort_column = Config.RomBrowserSortColumn ;
    
    sprintf(Config.RomBrowserSortMethod,"ASC");
    Config.RomBrowserSortColumn = 0 ;
    ListViewSort() ;
    
     
    itemsNum = ListView_GetItemCount(hRomList);
    for(i=0;i<itemsNum;i++)
    {
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = i;
        ListView_GetItem(hRomList,&lvItem);
        pRomInfo = &ItemList.List[lvItem.lParam];
        strncpy(pszKailleraNamedRoms, pRomInfo->GoodName, strlen(pRomInfo->GoodName) + 1);
        pszKailleraNamedRoms += strlen(pRomInfo->GoodName) + 1;
    } 
        
    *(++pszKailleraNamedRoms) = '\0';
    
    sprintf( Config.RomBrowserSortMethod, sort_method ) ;
    Config.RomBrowserSortColumn = sort_column ;
    ListViewSort() ;
}

void KailleraPlay(void)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	kailleraInfos	kInfos;
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* build roms list :) */
	
	buildRomList();

	kInfos.appName = MUPEN_VERSION;
	kInfos.gameList = szKailleraNamedRoms;
	kInfos.gameCallback = kailleraGameCallback;
	kInfos.chatReceivedCallback = kailleraChatReceivedCallback;
	kInfos.clientDroppedCallback = kailleraClientDroppedCallback;
	kInfos.moreInfosCallback = kailleraMoreInfosCallback;

	// Lock some menu items 
	

	kailleraInit();
	kailleraSetInfos(&kInfos);
	kailleraSelectServerDialog(mainHWND);

	// Stop emulator if running


	// Unlock menu items


}



