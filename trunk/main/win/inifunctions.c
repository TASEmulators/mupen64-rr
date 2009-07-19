/***************************************************************************
                          inifunctions.c  -  description
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

#include "inifunctions.h"
#include "main_win.h"
#include "translation.h"
#include "../mupenIniApi.h"

void intCrcCode(unsigned long CRC1,unsigned long CRC2,BYTE Country_code,char* result)
{
    sprintf(result,"%08X-%08X-C%02X",(int)sl(CRC1),(int)sl(CRC2),Country_code);
} 
BOOL getIniGoodNameByMD5(char *md5str,char *result)
{
      mupenEntry *entry;
      entry = ini_search_by_md5(md5str);
      if (entry) {
          strcpy(result,entry->goodname);
          return TRUE;
      }
      else {   //unknown dump
          strcpy(result,"Unknown Dump");  
          return FALSE;
      }
}

void getIniGoodName(rom_header *HEADER,char *result)
{
      char crccode[200];
      mupenEntry *entry;
      strcpy(result,""); 
      intCrcCode(HEADER->CRC1,HEADER->CRC2,HEADER->Country_code,crccode);
      entry = ini_search_by_CRC(crccode);
      if (entry) {
         strcpy(result,entry->goodname);                
      }
}

void getIniComments(ROM_INFO *pRomInfo,char *result)
{
    char tmp[300];
    mupenEntry *entry;
    result[0] = '\0';
    sprintf(tmp,pRomInfo->MD5);
    if (strcmp(tmp,"")==0) {
       sprintf(tmp,"%08X-%08X-C%02X",(int)pRomInfo->CRC1,(int)pRomInfo->CRC2,pRomInfo->Country);
       entry = ini_search_by_CRC(tmp); 
    }
    else {
       entry = ini_search_by_md5(tmp);  
    }
    if (entry) {
       strcpy(result, entry->comments);
    }   
}

void setIniComments(ROM_INFO *pRomInfo,char *result)
{
    
    char tmp[300];
    mupenEntry *entry;
    if (strcmp(result,"")==0) { 
          return; 
    }
    sprintf(tmp,pRomInfo->MD5);
    if (strcmp(tmp,"")==0) {
       sprintf(tmp,"%08X-%08X-C%02X",(int)pRomInfo->CRC1,(int)pRomInfo->CRC2,pRomInfo->Country);
       entry = ini_search_by_CRC(tmp); 
    }
    else {
       entry = ini_search_by_md5(tmp);  
    }
    if (entry) {
       strcpy( entry->comments,result );
    }  
}
