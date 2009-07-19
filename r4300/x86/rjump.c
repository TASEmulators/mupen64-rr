/**
 * Mupen64 - rjump.c
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#include <stdlib.h>

#include "../recomp.h"
#include "../r4300.h"
#include "../macros.h"
#include "../ops.h"
#include "../recomph.h"

void dyna_jump()
{
   if (PC->reg_cache_infos.need_map)
     *return_address = (unsigned long)(PC->reg_cache_infos.jump_wrapper);
   else
     *return_address = (unsigned long)(actual->code + PC->local_addr);
   /*asm("mov return_address, %%esp \n"
       "ret                       \n"
       :
       :
       : "memory");*/
}

static int save_ebp;

void dyna_start(void (*code)())
{
#ifndef _WIN32
   save_ebp=0;
   asm("mov %%ebp, save_ebp \n" : : : "memory");
   code();
   asm("mov save_ebp, %%ebp \n" : : : "memory");
#else // _WIN32
   save_ebp=0;
   asm("mov %%ebp, _save_ebp \n" : : : "memory");
   code();
   asm("mov _save_ebp, %%ebp \n" : : : "memory");
#endif // _WIN32
}

static void dyna_stop2() {}

void dyna_stop()
{
   *return_address = (unsigned long)dyna_stop2;
#ifndef _WIN32
   asm("mov return_address, %%esp \n"
       "ret                       \n"
       :
       :
       : "memory");
#else // _WIN32
   asm("mov _return_address, %%esp \n"
       "ret                        \n"
       :
       :
       : "memory");
#endif // _WIN32
}
