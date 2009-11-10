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
#include "Config.h"
#include "resource.h"

LRESULT CALLBACK MacroDlgProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam) {
	static BYTE NController, NControl;

	switch (Message) 
    {
        case WM_INITDIALOG:
			
			NController = (BYTE) HIWORD(lParam);
			NControl = (BYTE) LOWORD(lParam);

			if ( NControl == 20 )
				SetWindowText(hDlg, "Macro C1");
			else if ( NControl == 21 )
				SetWindowText(hDlg, "Macro C2");
			else if ( NControl == 22 )
				SetWindowText(hDlg, "Macro C3");
			else if ( NControl == 23 )
				SetWindowText(hDlg, "Macro C4");
			else if ( NControl == 24 )
				SetWindowText(hDlg, "Macro C5");
			
			InitializeMacroChecks(hDlg, NController, NControl);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
            {
				case IDC_B_MAC_CLEAR:
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DR, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DL, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DD, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DU, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_S, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_Z, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_B, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_A, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CR, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CL, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CD, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CU, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_TR, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_TL, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AL, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AR, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AD, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AU, BM_SETCHECK, BST_UNCHECKED, 0);
					break;
			
				case IDOK:
					SetMacroButtonValue(hDlg, NController, NControl);

				case IDCANCEL:
					EndDialog(hDlg, TRUE);
					break;
			}
			return TRUE;
	}
	return FALSE;
}


void WINAPI InitializeMacroChecks(HWND hDlg, BYTE NController, BYTE NControl) {
	BUTTONS Buttons;

	Buttons.Value = Controller[NController].Input[NControl].button;

	if ( Buttons.R_DPAD )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DR, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.L_DPAD )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DL, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.D_DPAD )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DD, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.U_DPAD )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DU, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.START_BUTTON )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_S, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.Z_TRIG )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_Z, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.B_BUTTON )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_B, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.A_BUTTON )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_A, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.R_CBUTTON )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CR, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.L_CBUTTON )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CL, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.D_CBUTTON )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CD, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.U_CBUTTON )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CU, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.R_TRIG )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_TR, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.L_TRIG )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_TL, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.X_AXIS < 0 )
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AL, BM_SETCHECK, BST_CHECKED, 0);
	else if ( Buttons.X_AXIS > 0)
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AR, BM_SETCHECK, BST_CHECKED, 0);

	if ( Buttons.Y_AXIS < 0)
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AD, BM_SETCHECK, BST_CHECKED, 0);
	else if ( Buttons.Y_AXIS > 0)
		SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AU, BM_SETCHECK, BST_CHECKED, 0);
}

void WINAPI SetMacroButtonValue(HWND hDlg, BYTE NController, BYTE NControl) {
	BUTTONS Buttons;

	Buttons.Value = 0;
	
	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DR, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.R_DPAD = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DL, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.L_DPAD = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DD, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.D_DPAD = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_DU, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.U_DPAD = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_S, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.START_BUTTON = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_Z, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.Z_TRIG = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_B, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.B_BUTTON = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_A, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.A_BUTTON = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CR, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.R_CBUTTON = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CL, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.L_CBUTTON = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CD, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.D_CBUTTON = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_CU, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.U_CBUTTON = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_TR, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.R_TRIG = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_TL, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.L_TRIG = 1;

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AL, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.X_AXIS = -min(128,Controller[NController].SensMax);
	else if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AR, BM_GETCHECK, 0, 0) == BST_CHECKED )
		Buttons.X_AXIS = min(127,Controller[NController].SensMax);

	if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AD, BM_GETCHECK, 0, 0) == BST_CHECKED)
		Buttons.Y_AXIS = -min(128,Controller[NController].SensMax);
	else if ( SendDlgItemMessage(hDlg, IDC_CHECK_MAC_AU, BM_GETCHECK, 0, 0) == BST_CHECKED)
		Buttons.Y_AXIS = min(127,Controller[NController].SensMax);

	Controller[NController].Input[NControl].button = Buttons.Value;
}