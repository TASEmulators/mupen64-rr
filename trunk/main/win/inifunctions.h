/***************************************************************************
                          inifunctions.h  -  description
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
#include "../../memory/memory.h"
#include "../rom.h"
#include "rombrowser.h"

void intCrcCode(unsigned long CRC1,unsigned long CRC2,BYTE Country_code,char* result);
void getIniGoodName(rom_header *HEADER,char *result);
void getIniComments(ROM_INFO *pRomInfo,char *result);
void setIniComments(ROM_INFO *pRomInfo,char *result);
BOOL getIniGoodNameByMD5(char *md5str,char *result);
