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

#include <csetjmp>

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

jmp_buf g_jmp_state;
void dyna_start(void (*code)())
{
	// code() のどこかで stop が true になった時、dyna_stop() が呼ばれ、longjmp() で setjmp() したところに戻る
	// 戻ってきた setjmp() は 1 を返すので、dyna_start() 終了
	// レジスタ ebx, esi, edi, ebp の保存と復元が必要だが、setjmp() がやってくれる
	if(setjmp(g_jmp_state) == 0)
	{
		code();
	}
}

void dyna_stop()
{
   longjmp(g_jmp_state, 1); // goto dyna_start()
}
