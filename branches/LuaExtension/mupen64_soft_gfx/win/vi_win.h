 /**
 * Mupen64 - vi_SDL.h
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

#include <ddraw.h> 
#include "../vi.h"
void ShowDebugMessage(char *Message, ... );

class VI_WIN : public VI
{
   int width;
   int height;
   
   void showFPS();
   virtual void setVideoMode(int w, int h);
   virtual void* getScreenPointer();
   virtual void blit();
   bool InitFullScreen(HWND hwnd);
   bool InitWindowed(HWND hwnd);  
   void ShowDebugMessage(char *Message, ... );
   void VI_WIN::ReleaseAll();
   void ConvertFormat(short * szSurface);
   int screenBPP;
   WINDOWPLACEMENT wndpl;
   HMENU OldMenu;
   LONG OldStlye;
   HANDLE hMutex;
   bool locked;
   void ChangeWinSize ( long width, long height );
   // globals (ugh)
   LPDIRECTDRAW lpDD;                       // DirectDraw object defined in DDRAW.H
   LPDIRECTDRAWSURFACE lpDDSPrimary ; // DirectDraw primary surface
   LPDIRECTDRAWSURFACE lpDDSBack ;    // DirectDraw back surface
   LPDIRECTDRAWCLIPPER lpClipper;
   
   bool bIsWindowed ;
   RECT rcRectDest,rcRectSrc;
   short * szSurface;                       // Store pointer to surface
   GFX_INFO g_info;
     
 public:
   VI_WIN(GFX_INFO);
   virtual ~VI_WIN();
   
   virtual void switchFullScreenMode();
   virtual void switchWindowMode();
   virtual void setGamma(float gamma);
   virtual void moveScreen(int xpos, int ypos);
   void statusChanged();
   void widthChanged();
   void updateScreen();
   void debug_plot(int x, int y, int c);
   void flush();

};

