/**
 * Mupen64 - vi_SDL.cpp
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

#include <windows.h>
#include <ddraw.h> 
#include <stdio.h>

#include "vi_win.h"

/*
 * Function to initialize DirectDraw
 * Demonstrates:
 *   1) Creating the Direct Draw Object
 *   2) Setting the Cooperative level
 *   3) Setting the Display mode
 *
 */

bool VI_WIN::InitFullScreen(HWND hwnd)
{
    HRESULT ddrval;
    DDPIXELFORMAT  ddpf;
   //
   //* Create the main DirectDraw object.
   // *
   // * This function takes care of initializing COM and Constructing
   // * the DirectDraw object.
   // *
    ddrval = DirectDrawCreate( NULL, &lpDD, NULL );
    if( ddrval != DD_OK )
    {
        return(false);
    }
   //*
   // * The cooperative level determines how much control we have over the
   // * screen. This must at least be either DDSCL_EXCLUSIVE or DDSCL_NORMAL
   // *
   // * DDSCL_EXCLUSIVE allows us to change video modes, and requires
   // * the DDSCL_FULLSCREEN flag, which will cause the window to take over
   // * the fullscreen. This is the preferred DirectDraw mode because it allows
   // * us to have control of the whole screen without regard for GDI.
   // *
   // * DDSCL_NORMAL is used to allow the DirectDraw app to run windowed.
   // *
    ddrval = lpDD->SetCooperativeLevel( hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
    
    if( ddrval != DD_OK )
    {
        lpDD->Release();
        return(false);
    }
    
    lpDD->SetDisplayMode(640, 480, 16);
    
    /*
     * Create a Primary surface that is flippable, with an
     * attached backbuffer
    */
    DDSURFACEDESC ddsd;
    DDSCAPS ddscaps;
    
    // Create the primary surface with 1 back buffer
    memset( &ddsd, 0, sizeof(ddsd) );
    ddsd.dwSize = sizeof( ddsd );
    
    //ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT | DDSD_PIXELFORMAT;
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT ;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;

    ddrval = lpDD->CreateSurface( &ddsd, &lpDDSPrimary, NULL );
    if( ddrval != DD_OK )
    {
        lpDD->Release();
        return(false);
    }
    // Get the pointer to the back buffer
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    ddrval = lpDDSPrimary->GetAttachedSurface(&ddscaps, &lpDDSBack);
    if( ddrval != DD_OK )
    {
        lpDDSPrimary->Release();
        lpDD->Release();
        return(false);
    }
    bIsWindowed = false;
    return true;
}


bool VI_WIN::InitWindowed(HWND hwnd)
{
    DDSURFACEDESC ddsd;
    DDSCAPS ddscaps;
    DDPIXELFORMAT  ddpf;
    HRESULT ddrval;
   
    // filling ddpf
    memset(&ddpf, 0, sizeof(ddpf));
    ddpf.dwSize = sizeof(ddpf);
    ddpf.dwFlags = DDPF_RGB;
    ddpf.dwRGBBitCount = 16;
    ddpf.dwRBitMask = 0xF800;
    ddpf.dwGBitMask = 0x07C0;
    ddpf.dwBBitMask = 0x003E;
    ddpf.dwRGBAlphaBitMask = 1;
   
    //* create the main DirectDraw object
    
    ddrval = DirectDrawCreate( NULL, &lpDD, NULL );
    if( ddrval != DD_OK )
    {
        ShowDebugMessage("Unable to create direct draw object");
        return(false);
    }

    // using DDSCL_NORMAL means we will coexist with GDI
    ddrval = lpDD->SetCooperativeLevel( hwnd, DDSCL_NORMAL );
    if( ddrval != DD_OK )
    {
        lpDD->Release();
        return(false);
    }

    memset( &ddsd, 0, sizeof(ddsd) );
    ddsd.dwSize = sizeof( ddsd );
    //ddsd.dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.dwFlags = DDSD_CAPS ;
    ddsd.ddpfPixelFormat = ddpf;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    // The primary surface is not a page flipping surface this time
    ddrval = lpDD->CreateSurface( &ddsd, &lpDDSPrimary, NULL );
    
    lpDDSPrimary->GetPixelFormat(&ddpf);
    screenBPP = ddpf.dwRGBBitCount;
    
    ShowDebugMessage("%4X\n",ddsd.ddpfPixelFormat.dwRBitMask);
    ShowDebugMessage("%4X\n",ddsd.ddpfPixelFormat.dwGBitMask);
    ShowDebugMessage("%4X\n",ddsd.ddpfPixelFormat.dwBBitMask);
    if( ddrval != DD_OK )
    {
        ShowDebugMessage("Unable to create primary Ddraw surface");
        ShowDebugMessage("%d",GetLastError()) ;
        lpDD->Release();
        return(false);
    }

    // Create a clipper to ensure that our drawing stays inside our window
    ddrval = lpDD->CreateClipper( 0, &lpClipper, NULL );
    if( ddrval != DD_OK )
    {
        lpDDSPrimary->Release();
        lpDD->Release();
        return(false);
    }

    // setting it to our hwnd gives the clipper the coordinates from our window
    ddrval = lpClipper->SetHWnd( 0, hwnd );
    if( ddrval != DD_OK )
    {
        lpClipper-> Release();
        lpDDSPrimary->Release();
        lpDD->Release();
        return(false);
    }

    // attach the clipper to the primary surface
    ddrval = lpDDSPrimary->SetClipper( lpClipper );
    if( ddrval != DD_OK )
    {
        lpClipper-> Release();
        lpDDSPrimary->Release();
        lpDD->Release();
        return(false);
    }

    memset( &ddsd, 0, sizeof(ddsd) );
    ddsd.dwSize = sizeof( ddsd );
//    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH ;
    ddsd.ddpfPixelFormat = ddpf;
    ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
    ddsd.dwWidth = 640;
    ddsd.dwHeight = 480;

    // create the backbuffer separately
    ddrval = lpDD->CreateSurface( &ddsd, &lpDDSBack, NULL );
    if( ddrval != DD_OK )
    {
        lpClipper-> Release();
        lpDDSPrimary->Release();
        lpDD->Release();
        return(false);
    }
    bIsWindowed = true;
    moveScreen(0,0);

    return(true);
}

void VI_WIN::ChangeWinSize ( long width, long height ) {
	WINDOWPLACEMENT wndpl;
    RECT rc1, swrect;

	if (!bIsWindowed) { return; }
	height+=30;
	wndpl.length = sizeof(wndpl);
	GetWindowPlacement( g_info.hWnd, &wndpl);

	if ( g_info.hStatusBar != NULL ) {
		GetClientRect( g_info.hStatusBar, &swrect );
	    SetRect( &rc1, 0, 0, width, height + swrect.bottom );
	} else {
	    SetRect( &rc1, 0, 0, width, height );
	}

    AdjustWindowRectEx( &rc1,GetWindowLong( g_info.hWnd, GWL_STYLE ),
		GetMenu( g_info.hWnd ) != NULL, GetWindowLong( g_info.hWnd, GWL_EXSTYLE ) ); 

    MoveWindow( g_info.hWnd, wndpl.rcNormalPosition.left, wndpl.rcNormalPosition.top, 
		rc1.right - rc1.left, rc1.bottom - rc1.top, TRUE );
    
	MoveScreen(wndpl.rcNormalPosition.left, wndpl.rcNormalPosition.top);
}


void VI_WIN::ReleaseAll()
{
    if (lpDDSPrimary) {lpDDSPrimary->Release();lpDDSPrimary=NULL;};
    if (lpClipper) {lpClipper->Release();lpClipper=NULL;};
    if (lpDDSBack) {lpDDSBack->Release();lpDDSBack=NULL;};
    if (lpDD) {lpDD->Release();lpDD=NULL;};
}

VI_WIN::VI_WIN(GFX_INFO info) : VI(info), lpDDSPrimary(NULL), lpDDSBack(NULL), bIsWindowed(false)
{
    OutputDebugString("Constructor\n");
    memcpy(&g_info,&info,sizeof(info));
    if (!InitWindowed( g_info.hWnd)) {
       OutputDebugString("Failed to init windowed mode\n");
       ShowDebugMessage("%d",GetLastError()) ;
    };
    hMutex = CreateMutex(NULL, FALSE, NULL);
    locked = false;
}

VI_WIN::~VI_WIN()
{
    OutputDebugString("Destructor\n");
    ReleaseAll();
}

void VI_WIN::setVideoMode(int w, int h)
{
  OutputDebugString("Set_Video_Mode\n");
  ChangeWinSize(w,h);
}

void VI_WIN::switchFullScreenMode()
{
  WaitForSingleObject(hMutex, INFINITE);
  OutputDebugString("switchFullScreenMode\n");
  
  bIsWindowed = false;
  
  wndpl.length = sizeof(wndpl);
  GetWindowPlacement( g_info.hWnd, &wndpl);
  if (g_info.hStatusBar) { ShowWindow(g_info.hStatusBar,SW_HIDE); }
  OldMenu = GetMenu(g_info.hWnd);
  if (OldMenu) { SetMenu(g_info.hWnd,NULL); }
  OldStlye = GetWindowLong(g_info.hWnd,GWL_STYLE);
  SetWindowLong(g_info.hWnd, GWL_STYLE, WS_VISIBLE);
  SetWindowPos(g_info.hWnd, HWND_TOPMOST, 0, 0, 640, 480, SWP_SHOWWINDOW);
  ShowCursor(FALSE);
  if (lpDDSPrimary) { 
    lpDDSPrimary->Release();
    lpDDSPrimary = NULL;
  }
  if (lpClipper) { 
    lpClipper->Release();
    lpClipper = NULL;
  }
  if (lpDDSBack) {
    lpDDSBack->Release();
    lpDDSBack = NULL;
  }
  lpDD->SetCooperativeLevel(g_info.hWnd,DDSCL_ALLOWREBOOT|DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
  lpDD->SetDisplayMode( 640, 480, 16);
  
  DDSURFACEDESC ddsd;
  ddsd.dwSize = sizeof( ddsd );
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  if ( FAILED(lpDD->CreateSurface( &ddsd, &lpDDSPrimary, NULL ))) {
    OutputDebugString("Primary surface creation failed");
    ChangeWindow();
    ReleaseMutex(hMutex);
    return;
  }
  
  memset( &ddsd, 0, sizeof(ddsd) );
  ddsd.dwSize = sizeof( ddsd );
//ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH ;
  ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
  ddsd.dwWidth = 640;
  ddsd.dwHeight = 480;
  
  // create the backbuffer separately
  if( FAILED(lpDD->CreateSurface( &ddsd, &lpDDSBack, NULL )) )
  {
    OutputDebugString("Back surface creation failed");
    lpClipper-> Release();
    lpDDSPrimary->Release();
    lpDD->Release();
    ReleaseMutex(hMutex);
    return;
  }

  screenBPP = 16;
  SetRect(&rcRectDest, 0, 0, 640, 480);
  ReleaseMutex(hMutex);
}

void VI_WIN::switchWindowMode()
{
  WaitForSingleObject(hMutex,INFINITE);
  OutputDebugString("switchWindowMode\n");
  
  bIsWindowed = true;
  if (lpDDSPrimary) { 
    lpDDSPrimary->Release();
    lpDDSPrimary = NULL;
  }
  if (lpDDSBack) {
    lpDDSBack->Release();
    lpDDSBack = NULL;
  }
  lpDD->SetCooperativeLevel(g_info.hWnd,DDSCL_NORMAL);
  lpDD->RestoreDisplayMode();

  // filling ddpf
  DDPIXELFORMAT  ddpf;
  memset(&ddpf, 0, sizeof(ddpf));
  ddpf.dwSize = sizeof(ddpf);
  ddpf.dwFlags = DDPF_RGB;
  ddpf.dwRGBBitCount = 16;
  ddpf.dwRBitMask = 0xF800;
  ddpf.dwGBitMask = 0x07C0;
  ddpf.dwBBitMask = 0x003E;
  ddpf.dwRGBAlphaBitMask = 1;

  DDSURFACEDESC ddsd;
  ddsd.dwSize = sizeof( ddsd );
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddpfPixelFormat = ddpf;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  if ( FAILED(lpDD->CreateSurface( &ddsd, &lpDDSPrimary, NULL ))) {
    ReleaseMutex(hMutex);
  	CloseDLL();
  }
  
  
  if ( FAILED( lpDD->CreateClipper( 0, &lpClipper, NULL ) ) ) {
    ReleaseMutex(hMutex);
    CloseDLL();
  }

  if ( FAILED( lpClipper->SetHWnd( 0, g_info.hWnd ) ) ) {
    ReleaseMutex(hMutex);
    CloseDLL();
  }

  if ( FAILED( lpDDSPrimary->SetClipper( lpClipper ) ) ) {
    ReleaseMutex(hMutex);
    CloseDLL();
  }
  
  lpDDSPrimary->GetPixelFormat(&ddpf);
  screenBPP = ddpf.dwRGBBitCount;
  
  memset( &ddsd, 0, sizeof(ddsd) );
  ddsd.dwSize = sizeof( ddsd );
//ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH ;
  ddsd.ddpfPixelFormat = ddpf;
  ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
  ddsd.dwWidth = 640;
  ddsd.dwHeight = 480;
  
  // create the backbuffer separately
  if( FAILED(lpDD->CreateSurface( &ddsd, &lpDDSBack, NULL )) )
  {
    OutputDebugString("Back surface creation failed");
    lpClipper-> Release();
    lpDDSPrimary->Release();
    lpDD->Release();
    ReleaseMutex(hMutex);
    return;
  }

  if (g_info.hStatusBar) { ShowWindow(g_info.hStatusBar,SW_SHOW); }
  if (OldMenu) { 
    SetMenu(g_info.hWnd,OldMenu); 
    OldMenu = NULL;
  }
  SetWindowLong(g_info.hWnd, GWL_STYLE, OldStlye);
  SetWindowPos(g_info.hWnd, HWND_NOTOPMOST, wndpl.rcNormalPosition.left, 
                wndpl.rcNormalPosition.top, 640, 480, SWP_NOSIZE|SWP_SHOWWINDOW);
  ChangeWinSize(640, 480);
  ShowCursor(TRUE);
  ReleaseMutex(hMutex);
}

void* VI_WIN::getScreenPointer()
{
  DDSURFACEDESC   ddsd;       // Surface description
  HRESULT         hr;
  if(!locked) WaitForSingleObject(hMutex, INFINITE);
  locked = true;
  if (!lpDDSBack) {return 0;}
  
  ddsd.dwSize = sizeof(ddsd);
  hr = lpDDSBack->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL );
  if (FAILED(hr))
    {OutputDebugString("Royal fuckup\n");}
  szSurface = (short*)ddsd.lpSurface;

  return szSurface;
}


void VI_WIN::ConvertFormat(short * szSurface)
{
    if(screenBPP == 32)
    {
      unsigned char *surf = (unsigned char *)szSurface;
      
      for (int i=0; i<640*480; i++)
      {
        surf[640*480*4-(i+1)*4+2] = (szSurface[640*480-(i+1)] & 0x7C00) >> 7;
        surf[640*480*4-(i+1)*4+1] = (szSurface[640*480-(i+1)] & 0x03E0) >> 2;
        surf[640*480*4-(i+1)*4+0] = (szSurface[640*480-(i+1)] & 0x001F) << 3;
      } 
    }
    else
    {    
      for (int i=0;i<640*480;i++)
      {
        szSurface[i] = ((szSurface[i] << 1) &~ 0x1F ) | (szSurface[i] & 0x1F);
      }
    }
}

void VI_WIN::blit()
{

  if(!locked) WaitForSingleObject(hMutex, INFINITE);
  OutputDebugString("start: blit\n");

  ConvertFormat(szSurface);
  lpDDSBack->Unlock( NULL );
  locked = false;

  lpDDSPrimary->Blt(&rcRectDest, lpDDSBack, NULL,DDBLT_WAIT, NULL);
    
  ReleaseMutex(hMutex);
}

void VI_WIN::setGamma(float gamma)
{
  
}

void VI_WIN::moveScreen(int xpos, int ypos)
{
  POINT p;
  p.x = 0; p.y = 0;
  ClientToScreen(g_info.hWnd, &p);
  GetClientRect(g_info.hWnd, &rcRectDest);
  OffsetRect(&rcRectDest, p.x, p.y);
  if(g_info.hStatusBar != NULL && bIsWindowed)
  {
      RECT swrect;
      GetClientRect( g_info.hStatusBar, &swrect );
      rcRectDest.bottom = rcRectDest.bottom - swrect.bottom;
  }
  rcRectDest.top = rcRectDest.bottom - 480;
  SetRect(&rcRectSrc, 0, 0, 640, 480);
}
 
void VI_WIN::showFPS()
{
   
} 

void VI_WIN::ShowDebugMessage(char *Message, ... )
{
	char tempStr[1024];
	va_list ap;
	va_start( ap, Message);
	vsprintf( tempStr, Message, ap) ; 
	va_end( ap);
	strcat(tempStr, "\n");
	OutputDebugString( tempStr);
	
}


