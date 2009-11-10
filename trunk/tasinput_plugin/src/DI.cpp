/*
Copyright (C) 2001 Deflection

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*
	Modified for TAS by Nitsuja
*/

#include <windows.h>
#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>
#include "Controller.h"
#include "DI.h"
#include "DefDI.h"
#include "resource.h"


LPDIRECTINPUT7        g_lpDI          = NULL;
DIINPUTDEVICE DInputDev[MAX_DEVICES];
BYTE nCurrentDevices;

BOOL WINAPI InitDirectInput(HWND hMainWindow) 
{ 
    HRESULT hr;
    
    FreeDirectInput();

// Create the DirectInput object.
	nCurrentDevices = 0;
     
	if FAILED(hr = DirectInputCreateEx(g_hInstance, DIRECTINPUT_VERSION, 
			IID_IDirectInput7, (void**)&g_lpDI, NULL) ) {
		if (hr == DIERR_OLDDIRECTINPUTVERSION)
			MessageBox(NULL,"Old version of DirectX detected. Use DirectX 7 or higher!","Error",MB_ICONERROR | MB_OK);
		return FALSE;
	}
	else {
		g_lpDI->EnumDevices(DIDEVTYPE_KEYBOARD, DIEnumDevicesCallback,
                       (LPVOID)hMainWindow, DIEDFL_ATTACHEDONLY);
		g_lpDI->EnumDevices(DIDEVTYPE_JOYSTICK, DIEnumDevicesCallback,
                       (LPVOID)hMainWindow, DIEDFL_ATTACHEDONLY);
		if (nCurrentDevices == 0)
			return FALSE;
	
	}
  return TRUE;
}

BOOL CALLBACK DIEnumDevicesCallback( LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
	HRESULT hr;
	HWND hMainWindow = (HWND)pvRef;
	BOOL bOK = TRUE;


	if(nCurrentDevices>=MAX_DEVICES)
		return DIENUM_STOP;

    //if( (GET_DIDEVICE_TYPE(lpddi->dwDevType) == DIDEVTYPE_KEYBOARD) ) 
	if((lpddi->dwDevType & DIDEVTYPE_KEYBOARD) == DIDEVTYPE_KEYBOARD){

		memcpy(&DInputDev[nCurrentDevices].DIDevInst, lpddi, sizeof(DIDEVICEINSTANCE));
		if (!FAILED( hr = g_lpDI->CreateDeviceEx( lpddi->guidInstance, IID_IDirectInputDevice7,
										(void **) &DInputDev[nCurrentDevices].lpDIDevice, NULL ) ) ){
		
			if FAILED( hr = DInputDev[nCurrentDevices].lpDIDevice->SetDataFormat(&c_dfDIKeyboard) ) 
				bOK = FALSE; 
			if FAILED( hr = DInputDev[nCurrentDevices].lpDIDevice->SetCooperativeLevel(hMainWindow, 
				                 DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) ) 
				bOK = FALSE;
		}
	}
	
	if((lpddi->dwDevType & DIDEVTYPE_JOYSTICK) == DIDEVTYPE_JOYSTICK){

		memcpy(&DInputDev[nCurrentDevices].DIDevInst, lpddi, sizeof(DIDEVICEINSTANCE));
		if (!FAILED( hr = g_lpDI->CreateDeviceEx( lpddi->guidInstance, IID_IDirectInputDevice7,
									(void **) &DInputDev[nCurrentDevices].lpDIDevice, NULL ) ) ){
		
			if FAILED( hr = DInputDev[nCurrentDevices].lpDIDevice->SetDataFormat(&c_dfDIJoystick) ) 
				bOK = FALSE; 
			if FAILED( hr = DInputDev[nCurrentDevices].lpDIDevice->SetCooperativeLevel(hMainWindow, 
				                 DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) ) 
				bOK = FALSE;
			if FAILED( hr = DInputDev[nCurrentDevices].lpDIDevice->EnumObjects( EnumAxesCallback, 
                                                (LPVOID)hMainWindow, DIDFT_AXIS ) )
				bOK = FALSE;
		}
	}

	if (bOK == TRUE) {
		DInputDev[nCurrentDevices].lpDIDevice->Acquire();
		nCurrentDevices++;
	}
	else {
		DInputDev[nCurrentDevices].lpDIDevice->Release();
		DInputDev[nCurrentDevices].lpDIDevice = NULL;
	}

	return DIENUM_CONTINUE;
}

BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                LPVOID pContext ) {
    HWND hDlg = (HWND)pContext;

    DIPROPRANGE diprg; 
    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYID; 
    diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
    diprg.lMin              = -127; 
    diprg.lMax              = +128; 
    
	// Set the range for the axis
	if FAILED( DInputDev[nCurrentDevices].lpDIDevice->SetProperty( DIPROP_RANGE, &diprg.diph ) )
		return DIENUM_STOP;

	return TRUE;
}

void WINAPI FreeDirectInput()  { 
    if (g_lpDI) { 
		for(int i=0; i<MAX_DEVICES; i++) {
			if (DInputDev[i].lpDIDevice != NULL) {
				DInputDev[i].lpDIDevice->Unacquire();
				DInputDev[i].lpDIDevice->Release();
				DInputDev[i].lpDIDevice = NULL;
			}
		}
        g_lpDI->Release();
        g_lpDI = NULL; 
    }
}