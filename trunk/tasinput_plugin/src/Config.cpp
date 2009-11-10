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
#include <commctrl.h>
#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>
#include "Controller.h"
#include "DI.h"
#include "DefDI.h"
#include "Config.h"
#include "resource.h"


LRESULT CALLBACK ConfigDlgProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    static BYTE NController, NControl, NDeviceCount;
	static HANDLE hFile;
	static HKEY hKey;
	static BOOL bApply[NUMBER_OF_CONTROLS];
	static TCHAR szDeviceNum[MAX_PATH] = TEXT(""), szInitialDir[MAX_PATH] = TEXT(""), szFilename[MAX_PATH] = TEXT("");
	static TCHAR CSWindowText[MAX_PATH];
	static DWORD dwSize, dwType, ControlValue, dwMacroParam;
	static ULONG lNumBytes;
	static DEFCONTROLLER tempController;
	static OPENFILENAME ofn = {0};

	dwType = REG_BINARY;
	dwSize = sizeof(DEFCONTROLLER);
	
	switch (Message) 
    {
        case WM_INITDIALOG:
			if (RegOpenKeyEx(HKEY_CURRENT_USER, SUBKEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS ) {
				for (NController=0; NController<NUMBER_OF_CONTROLS; NController++) {
					RegQueryValueEx(hKey, Controller[NController].szName, 0, &dwType, (LPBYTE)&Controller[NController], &dwSize);
				}
			}
			RegCloseKey(hKey);
			
			for (NController=0; NController<NUMBER_OF_CONTROLS; NController++) {
				SendDlgItemMessage(hDlg, IDC_COMBOCONT, CB_ADDSTRING, 0, (LPARAM)Controller[NController].szName);
			}
			NController = (BYTE) SendDlgItemMessage(hDlg, IDC_COMBOCONT, CB_SETCURSEL, 0, 0);
			
			SetWindowText(hDlg, Controller[NController].szName);
			wsprintf(CSWindowText,"%s:  Choose a button (Esc to Disable)", Controller[NController].szName);

			for(NDeviceCount=0; NDeviceCount<nCurrentDevices; NDeviceCount++) {
				if ( DInputDev[NDeviceCount].lpDIDevice != NULL ) {
					wsprintf(szDeviceNum,"%d:  %s", NDeviceCount, DInputDev[NDeviceCount].DIDevInst.tszInstanceName);
					SendDlgItemMessage(hDlg, IDC_LDEVICES, LB_ADDSTRING, 0, (LPARAM)szDeviceNum);
				}
			}

			if(Controller[NController].NDevices == 0 && nCurrentDevices > 0)
				Controller[NController].NDevices = 1;

			Initialize_Controller_Display(hDlg, NController);
			Dis_En_AbleApply(hDlg, bApply[NController]=FALSE);

			return TRUE;

		case WM_TIMER:
			if ( GetKeyState(VK_ESCAPE) & 0x8000 ) {
				Controller[NController].Input[NControl].Device = 0;
				Controller[NController].Input[NControl].type = INPUT_TYPE_NOT_USED;
				Controller[NController].Input[NControl].vkey = 0;			
				SetDlgItemText(hDlg,ControlValue," ");
				KillTimer(hDlg, IDT_TIMER1); 
				KillTimer(hDlg, IDT_TIMER2);
				SetWindowText(hDlg, Controller[NController].szName);
				Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
			}
			else {
				switch (wParam)
				{
					case IDT_TIMER1:
						if ( GetAControl(hDlg, ControlValue, NController, NControl) ) {
							KillTimer(hDlg, IDT_TIMER1); 
							KillTimer(hDlg, IDT_TIMER2);
							SetWindowText(hDlg, Controller[NController].szName);
							Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
						}
						break;

					case IDT_TIMER2: // time over; cancel
						KillTimer(hDlg, IDT_TIMER1); 
						KillTimer(hDlg, IDT_TIMER2);
						SetWindowText(hDlg, Controller[NController].szName);
						break;
				}
			}
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
            {
				case IDC_COMBOCONT:
					if (HIWORD(wParam) == CBN_SELENDOK) {
						NController = (BYTE) SendDlgItemMessage(hDlg, IDC_COMBOCONT, CB_GETCURSEL, 0, 0);
								
						SetWindowText(hDlg, Controller[NController].szName);
						wsprintf(CSWindowText,"%s:  Choose a button (Esc to Disable)", Controller[NController].szName);

						//if (RegOpenKeyEx(HKEY_CURRENT_USER, SUBKEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
						//	RegQueryValueEx(hKey, Controller[NController].szName, 0, &dwType, (LPBYTE)&Controller[NController], &dwSize);
						//RegCloseKey(hKey);

						Initialize_Controller_Display(hDlg, NController);
						Dis_En_AbleApply(hDlg, bApply[NController]);
					}
					break;

				case IDC_CHECKACTIVE:
				{
					LRESULT lActive = SendDlgItemMessage(hDlg, IDC_CHECKACTIVE, BM_GETCHECK, 0, 0);
					if ( lActive == BST_CHECKED) {
						Controller[NController].bActive = TRUE;
						Dis_En_AbleControls(hDlg, Controller[NController].bActive);
						Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
					}
					else if ( lActive == BST_UNCHECKED) {
						Controller[NController].bActive = FALSE;
						Dis_En_AbleControls(hDlg, Controller[NController].bActive);
						Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
					}
				}	break;

				case IDC_CHECKMEMPAK:
				{
					LRESULT lMemPak = SendDlgItemMessage(hDlg, IDC_CHECKMEMPAK, BM_GETCHECK, 0, 0);
					if ( lMemPak == BST_CHECKED) {
						Controller[NController].bMemPak = TRUE;
					}
					else if ( lMemPak == BST_UNCHECKED) {
						Controller[NController].bMemPak = FALSE;
					}
				}	break;

				case IDC_LDEVICES:
					ZeroMemory( &Controller[NController].Devices, sizeof(Controller[NController].Devices) );
					Controller[NController].NDevices = (BYTE) SendDlgItemMessage(hDlg, IDC_LDEVICES, LB_GETSELCOUNT, 0, 0);
					SendDlgItemMessage(hDlg, IDC_LDEVICES, LB_GETSELITEMS, MAX_DEVICES, (LPARAM)Controller[NController].Devices);
					Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
					break;

				case IDC_BSAVE:					
					GetModuleFileName(NULL,szInitialDir,sizeof(szInitialDir));
					ofn.lStructSize = sizeof (OPENFILENAME);
					ofn.hwndOwner = hDlg; 
					ofn.lpstrFilter = TEXT("N64 Controller Definition (*.cdf)\0*.cdf\0\0");
					ofn.lpstrFile = szFilename;
					ofn.lpstrTitle = TEXT("Save Definition As");
					ofn.lpstrInitialDir = szInitialDir;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrDefExt = TEXT("cdf");
					ofn.Flags = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_HIDEREADONLY | 
								OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

					if ( !GetSaveFileName (&ofn) ) {
						break;
					}
					hFile = CreateFile(szFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hFile == INVALID_HANDLE_VALUE) {
						MessageBox(NULL,"Could not Save File.","Error",MB_ICONERROR | MB_OK);
						CloseHandle(hFile);
						break;
					}
					memcpy(&tempController, &Controller[NController], sizeof(DEFCONTROLLER));
					lstrcpy(tempController.szName, TEXT("cdf Save File") );
					if ( !WriteFile(hFile, &tempController, sizeof(DEFCONTROLLER), &lNumBytes, NULL) ) {
						MessageBox(NULL,"Could not Save File.","Error",MB_ICONERROR | MB_OK);
					}
					CloseHandle(hFile);	
					break;

				case IDC_BLOAD:
					GetModuleFileName(NULL,szInitialDir,sizeof(szInitialDir));
					ofn.lStructSize = sizeof (OPENFILENAME);
					ofn.hwndOwner = hDlg; 
					ofn.lpstrFilter = TEXT("N64 Controller Definition (*.cdf)\0*.cdf\0\0");
					ofn.lpstrFile = szFilename;
					ofn.lpstrTitle = TEXT("Save Definition As");
					ofn.lpstrInitialDir = szInitialDir;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrDefExt = TEXT("cdf");
					ofn.Flags = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_HIDEREADONLY | 
								OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;

					if ( !GetOpenFileName (&ofn) ) {
						return TRUE;
					}
					hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hFile == INVALID_HANDLE_VALUE) {
						MessageBox(NULL,"Could not Open File.","Error",MB_ICONERROR | MB_OK);
						CloseHandle(hFile);
						return TRUE;
					}
					if ( !ReadFile(hFile, &tempController, sizeof(DEFCONTROLLER), &lNumBytes, NULL) ) {
						MessageBox(NULL,"Could not Open File.","Error",MB_ICONERROR | MB_OK);
					}
					else {
						if ( lNumBytes == sizeof(DEFCONTROLLER) ) {
							lstrcpy(tempController.szName, Controller[NController].szName);
							memcpy(&Controller[NController], &tempController, sizeof(DEFCONTROLLER));
							Initialize_Controller_Display(hDlg, NController);
							Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
						}
						else
							MessageBox(NULL,"Could not Open File.\n Older version.","Error",MB_ICONERROR | MB_OK);
					}
					CloseHandle(hFile);
					break;

				case IDC_E_MAX:
					SetFocus(hDlg);
					Controller[NController].SensMax = 
						(BYTE) SendDlgItemMessage(hDlg, IDC_SPINMAX, UDM_GETPOS, 0, 0);
					GetAControlValue(hDlg, IDC_EAS_UP, NController, 0);
					GetAControlValue(hDlg, IDC_EAS_DOWN, NController, 1);
					GetAControlValue(hDlg, IDC_EAS_LEFT, NController, 2);
					GetAControlValue(hDlg, IDC_EAS_RIGHT, NController, 3);
					Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
					break;

				case IDC_E_MIN:
					SetFocus(hDlg);
					Controller[NController].SensMin = 
						(BYTE) SendDlgItemMessage(hDlg, IDC_SPINMIN, UDM_GETPOS, 0, 0);
					Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
					break;

				case IDC_B_CLEAR:
					for (NControl=0; NControl<NUMBER_OF_BUTTONS; NControl++) {
						Controller[NController].Input[NControl].Device = 0;
						Controller[NController].Input[NControl].type = INPUT_TYPE_NOT_USED;
						Controller[NController].Input[NControl].vkey = 0;
						Controller[NController].Input[NControl].button = 0;
					}
					Controller[NController].NDevices = 0;
					Initialize_Controller_Display(hDlg, NController);
					
					SendDlgItemMessage(hDlg, IDC_SPINMAX, UDM_SETPOS, 0, (LPARAM) MAKELONG(80, 0));
					SendDlgItemMessage(hDlg, IDC_SPINMIN, UDM_SETPOS, 0, (LPARAM) MAKELONG(30, 0));
					SendDlgItemMessage(hDlg, IDC_SPINM1, UDM_SETPOS, 0, (LPARAM) MAKELONG(40, 0));
					SendDlgItemMessage(hDlg, IDC_SPINM2, UDM_SETPOS, 0, (LPARAM) MAKELONG(20, 0));
					Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
					break;

				case IDC_BAS_UP:
					ControlValue = IDC_EAS_UP;
					NControl = 0;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_BAS_DOWN:
					ControlValue = IDC_EAS_DOWN;
					NControl = 1;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_BAS_LEFT:
					ControlValue = IDC_EAS_LEFT;
					NControl = 2;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_BAS_RIGHT:
					ControlValue = IDC_EAS_RIGHT;
					NControl = 3;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_START:
					ControlValue = IDC_E_START;
					NControl = 4;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_A:
					ControlValue = IDC_E_A;
					NControl = 5;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_B:
					ControlValue = IDC_E_B;
					NControl = 6;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_ZTRIG:
					ControlValue = IDC_E_ZTRIG;
					NControl = 7;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_LTRIG:
					ControlValue = IDC_E_LTRIG;
					NControl = 8;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_RTRIG:
					ControlValue = IDC_E_RTRIG;
					NControl = 9;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_CUP:
					ControlValue = IDC_E_CUP;
					NControl = 10;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_CDOWN:
					ControlValue = IDC_E_CDOWN;
					NControl = 11;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_CLEFT:
					ControlValue = IDC_E_CLEFT;
					NControl = 12;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_CRIGHT:
					ControlValue = IDC_E_CRIGHT;
					NControl = 13;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_DPUP:
					ControlValue = IDC_E_DPUP;
					NControl = 14;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_DPDOWN:
					ControlValue = IDC_E_DPDOWN;
					NControl = 15;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_DPLEFT:
					ControlValue = IDC_E_DPLEFT;
					NControl = 16;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_DPRIGHT:
					ControlValue = IDC_E_DPRIGHT;
					NControl = 17;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_B_M1:
					ControlValue = IDC_E_M1;
					NControl = 18;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_E_M1VAL:
					SetFocus(hDlg);
					Controller[NController].Input[18].button = 
						(DWORD) SendDlgItemMessage(hDlg, IDC_SPINM1, UDM_GETPOS, 0, 0);
					Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
					break;
				
				case IDC_B_M2:
					ControlValue = IDC_E_M2;
					NControl = 19;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_E_M2VAL:
					SetFocus(hDlg);
					Controller[NController].Input[19].button = 
						(DWORD) SendDlgItemMessage(hDlg, IDC_SPINM2, UDM_GETPOS, 0, 0);
					Dis_En_AbleApply(hDlg, bApply[NController]=TRUE);
					break;

				case IDC_B_MAC1:
					ControlValue = IDC_E_MAC1;
					NControl = 20;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_BTOG_MAC1:
					dwMacroParam = MAKELPARAM( 20, (WORD) NController);
					DialogBoxParam ( g_hInstance, MAKEINTRESOURCE(IDD_MACRODLG), hDlg, (DLGPROC)MacroDlgProc, dwMacroParam);
					break;

				case IDC_B_MAC2:
					ControlValue = IDC_E_MAC2;
					NControl = 21;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_BTOG_MAC2:
					dwMacroParam = MAKELPARAM( 21, (WORD) NController);
					DialogBoxParam ( g_hInstance, MAKEINTRESOURCE(IDD_MACRODLG), hDlg, (DLGPROC)MacroDlgProc, dwMacroParam);
					break;

				case IDC_B_MAC3:
					ControlValue = IDC_E_MAC3;
					NControl = 22;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_BTOG_MAC3:
					dwMacroParam = MAKELPARAM( 22, (WORD) NController);
					DialogBoxParam ( g_hInstance, MAKEINTRESOURCE(IDD_MACRODLG), hDlg, (DLGPROC)MacroDlgProc, dwMacroParam);
					break;

				case IDC_B_MAC4:
					ControlValue = IDC_E_MAC4;
					NControl = 23;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_BTOG_MAC4:
					dwMacroParam = MAKELPARAM( 23, (WORD) NController);
					DialogBoxParam ( g_hInstance, MAKEINTRESOURCE(IDD_MACRODLG), hDlg, (DLGPROC)MacroDlgProc, dwMacroParam);
					break;

				case IDC_B_MAC5:
					ControlValue = IDC_E_MAC5;
					NControl = 24;
					SetWindowText(hDlg, CSWindowText);
					Start_Timer(hDlg);
					break;

				case IDC_BTOG_MAC5:
					dwMacroParam = MAKELPARAM( 24, (WORD) NController);
					DialogBoxParam ( g_hInstance, MAKEINTRESOURCE(IDD_MACRODLG), hDlg, (DLGPROC)MacroDlgProc, dwMacroParam);
					break;


				case IDC_B_APPLY:
					RegCreateKeyEx(HKEY_CURRENT_USER, SUBKEY, 0, NULL, 0, KEY_WRITE, NULL, &hKey, 0);
					if(RegSetValueEx(hKey, Controller[NController].szName, 0, dwType, (LPBYTE)&Controller[NController], dwSize) != ERROR_SUCCESS)
						MessageBox(hDlg, "Error: Could not save current Contoller Config!", Controller[NController].szName, MB_ICONERROR | MB_OK);					
					RegCloseKey(hKey);
					Dis_En_AbleApply(hDlg, bApply[NController]=FALSE);
					break;
	
				case IDOK:
                    //SendDlgItemMessage(hDlg, IDC_VALUE, WM_GETTEXT, 10, (LPARAM) (LPSTR) no);          
					//MessageBox(NULL, "Cool", "Note:", MB_OK);
	
					RegCreateKeyEx(HKEY_CURRENT_USER, SUBKEY, 0, NULL, 0, KEY_WRITE, NULL, &hKey, 0);
					for (NController=0; NController<NUMBER_OF_CONTROLS; NController++) {
						if(RegSetValueEx(hKey, Controller[NController].szName, 0, dwType, (LPBYTE)&Controller[NController], dwSize) != ERROR_SUCCESS)
							MessageBox(hDlg, "Error: Could not save configuration!", Controller[NController].szName, MB_ICONERROR | MB_OK);					
					}
					RegCloseKey(hKey);
					EndDialog(hDlg, TRUE);
					break;

                case IDCANCEL:
					// hack: prevent cancel from closing dialog, since it's the key used to deactivate a control
					if ( GetKeyState(VK_ESCAPE) & 0x8000 ) {
						break;
					}

					if (RegOpenKeyEx(HKEY_CURRENT_USER, SUBKEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
						for (NController=0; NController<NUMBER_OF_CONTROLS; NController++) {
							RegQueryValueEx(hKey, Controller[NController].szName, 0, &dwType, (LPBYTE)&Controller[NController], &dwSize);
						}
					}
					RegCloseKey(hKey);
					EndDialog(hDlg, TRUE);
                    break;

				default:
					SetFocus(hDlg);
					break;
            }
			return TRUE;
	}
	return FALSE;
}

void WINAPI Start_Timer(HWND hDlg)
{
	SetTimer(hDlg, IDT_TIMER1, 10, (TIMERPROC) NULL);
	SetTimer(hDlg, IDT_TIMER2, 5000, (TIMERPROC) NULL);
}

void WINAPI Initialize_Controller_Display(HWND hDlg, BYTE NController)
{
	HWND hChild;
	TCHAR ControlName[32];

	//IDC_CHECKACTIVE
	SendDlgItemMessage(hDlg, IDC_CHECKACTIVE, BM_SETCHECK, Controller[NController].bActive ? BST_CHECKED : BST_UNCHECKED, 0);
	Dis_En_AbleControls(hDlg, Controller[NController].bActive);

	//IDC_CHECKMEMPAK
	SendDlgItemMessage(hDlg, IDC_CHECKMEMPAK, BM_SETCHECK, Controller[NController].bMemPak ? BST_CHECKED : BST_UNCHECKED, 0);

	//IDC_LDEVICES
	for(BYTE count=0; count<MAX_DEVICES; count++)
		SendDlgItemMessage(hDlg, IDC_LDEVICES, LB_SETSEL, BST_UNCHECKED, count);
	for(count=0; count<Controller[NController].NDevices; count++)
		SendDlgItemMessage(hDlg, IDC_LDEVICES, LB_SETSEL, BST_CHECKED, Controller[NController].Devices[count]);

	//IDC_E_MAX
	hChild = GetDlgItem(hDlg, IDC_E_MAX);

	//IDC_SPINMAX
	SendDlgItemMessage(hDlg, IDC_SPINMAX, UDM_SETBUDDY, (WPARAM) hChild, 0);
	SendDlgItemMessage(hDlg, IDC_SPINMAX, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short) 181, (short) 50));
	SendDlgItemMessage(hDlg, IDC_SPINMAX, UDM_SETPOS, 0, (LPARAM) MAKELONG(Controller[NController].SensMax, 0));

	//IDC_E_MIN
	hChild = GetDlgItem(hDlg, IDC_E_MIN);

	//IDC_SPINMIN
	SendDlgItemMessage(hDlg, IDC_SPINMIN, UDM_SETBUDDY, (WPARAM) hChild, 0);
	SendDlgItemMessage(hDlg, IDC_SPINMIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short) 50, (short) 0));
	SendDlgItemMessage(hDlg, IDC_SPINMIN, UDM_SETPOS, 0, (LPARAM) MAKELONG(Controller[NController].SensMin, 0));

	//IDC_BAS_UP
	GetAControlName(NController, 0, ControlName);
	SetDlgItemText(hDlg,IDC_EAS_UP,ControlName);

	//IDC_BAS_DOWN
	GetAControlName(NController, 1, ControlName);
	SetDlgItemText(hDlg,IDC_EAS_DOWN,ControlName);

	//IDC_BAS_LEFT
	GetAControlName(NController, 2, ControlName);
	SetDlgItemText(hDlg,IDC_EAS_LEFT,ControlName);

	//IDC_BAS_RIGHT
	GetAControlName(NController, 3, ControlName);
	SetDlgItemText(hDlg,IDC_EAS_RIGHT,ControlName);

	//IDC_B_START
	GetAControlName(NController, 4, ControlName);
	SetDlgItemText(hDlg,IDC_E_START,ControlName);

	//IDC_B_A
	GetAControlName(NController, 5, ControlName);
	SetDlgItemText(hDlg,IDC_E_A,ControlName);

	//IDC_B_B
	GetAControlName(NController, 6, ControlName);
	SetDlgItemText(hDlg,IDC_E_B,ControlName);

	//IDC_B_ZTRIG
	GetAControlName(NController, 7, ControlName);
	SetDlgItemText(hDlg,IDC_E_ZTRIG,ControlName);

	//IDC_B_LTRIG
	GetAControlName(NController, 8, ControlName);
	SetDlgItemText(hDlg,IDC_E_LTRIG,ControlName);

	//IDC_B_RTRIG
	GetAControlName(NController, 9, ControlName);
	SetDlgItemText(hDlg,IDC_E_RTRIG,ControlName);

	//IDC_B_CUP
	GetAControlName(NController, 10, ControlName);
	SetDlgItemText(hDlg,IDC_E_CUP,ControlName);

	//IDC_B_CDOWN
	GetAControlName(NController, 11, ControlName);
	SetDlgItemText(hDlg,IDC_E_CDOWN,ControlName);

	//IDC_B_CLEFT
	GetAControlName(NController, 12, ControlName);
	SetDlgItemText(hDlg,IDC_E_CLEFT,ControlName);

	//IDC_B_CRIGHT
	GetAControlName(NController, 13, ControlName);
	SetDlgItemText(hDlg,IDC_E_CRIGHT,ControlName);

	//IDC_B_DPUP
	GetAControlName(NController, 14, ControlName);
	SetDlgItemText(hDlg,IDC_E_DPUP,ControlName);

	//IDC_B_DPDOWN
	GetAControlName(NController, 15, ControlName);
	SetDlgItemText(hDlg,IDC_E_DPDOWN,ControlName);

	//IDC_B_DPLEFT
	GetAControlName(NController, 16, ControlName);
	SetDlgItemText(hDlg,IDC_E_DPLEFT,ControlName);

	//IDC_B_DPRIGHT
	GetAControlName(NController, 17, ControlName);
	SetDlgItemText(hDlg,IDC_E_DPRIGHT,ControlName);

	//IDC_B_M1
	GetAControlName(NController, 18, ControlName);
	SetDlgItemText(hDlg,IDC_E_M1,ControlName);

	//IDC_E_M1VAL
	hChild = GetDlgItem(hDlg, IDC_E_M1VAL);

	//IDC_SPINM1
	SendDlgItemMessage(hDlg, IDC_SPINM1, UDM_SETBUDDY, (WPARAM) hChild, 0);
	SendDlgItemMessage(hDlg, IDC_SPINM1, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short) 127, (short) 1));
	SendDlgItemMessage(hDlg, IDC_SPINM1, UDM_SETPOS, 0, (LPARAM) MAKELONG((short)Controller[NController].Input[18].button, (short) 0));
	
	//IDC_B_M2
	GetAControlName(NController, 19, ControlName);
	SetDlgItemText(hDlg,IDC_E_M2,ControlName);

	//IDC_E_M2VAL
	hChild = GetDlgItem(hDlg, IDC_E_M2VAL);

	//IDC_SPINM2
	SendDlgItemMessage(hDlg, IDC_SPINM2, UDM_SETBUDDY, (WPARAM) hChild, 0);
	SendDlgItemMessage(hDlg, IDC_SPINM2, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short) 127, (short) 1));
	SendDlgItemMessage(hDlg, IDC_SPINM2, UDM_SETPOS, 0, (LPARAM) MAKELONG((short)Controller[NController].Input[19].button, (short) 0));

	//IDC_B_MAC1
	GetAControlName(NController, 20, ControlName);
	SetDlgItemText(hDlg,IDC_E_MAC1,ControlName);

	//IDC_B_MAC2
	GetAControlName(NController, 21, ControlName);
	SetDlgItemText(hDlg,IDC_E_MAC2,ControlName);

	//IDC_B_MAC3
	GetAControlName(NController, 22, ControlName);
	SetDlgItemText(hDlg,IDC_E_MAC3,ControlName);

	//IDC_B_MAC4
	GetAControlName(NController, 23, ControlName);
	SetDlgItemText(hDlg,IDC_E_MAC4,ControlName);

	//IDC_B_MAC5
	GetAControlName(NController, 24, ControlName);
	SetDlgItemText(hDlg,IDC_E_MAC5,ControlName);

}

void WINAPI Dis_En_AbleApply(HWND hParent, BOOL bActive) {
	HWND hChild;

	hChild = GetDlgItem(hParent, IDC_B_APPLY);
	EnableWindow(hChild, bActive);
}

void WINAPI Dis_En_AbleControls(HWND hParent, BOOL bActive) {
	HWND hChild;

	hChild = GetDlgItem(hParent, IDC_LDEVICES);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_SPINMAX);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_MAX);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_SPINMIN);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_MIN);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_BAS_UP);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_EAS_UP);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_BAS_DOWN);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_EAS_DOWN);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_BAS_LEFT);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_EAS_LEFT);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_BAS_RIGHT);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_EAS_RIGHT);
	EnableWindow(hChild, bActive);
	
	hChild = GetDlgItem(hParent, IDC_B_START);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_START);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_A);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_A);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_B);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_B);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_ZTRIG);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_ZTRIG);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_LTRIG);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_LTRIG);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_RTRIG);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_RTRIG);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_CUP);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_CUP);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_CDOWN);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_CDOWN);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_CLEFT);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_CLEFT);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_CRIGHT);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_CRIGHT);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_DPUP);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_DPUP);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_DPDOWN);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_DPDOWN);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_DPLEFT);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_DPLEFT);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_DPRIGHT);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_DPRIGHT);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_M1);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_M1);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_SPINM1);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_M1VAL);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_M2);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_M2);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_SPINM2);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_M2VAL);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_MAC1);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_MAC1);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_BTOG_MAC1);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_MAC2);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_MAC2);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_BTOG_MAC2);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_MAC3);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_MAC3);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_BTOG_MAC3);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_MAC4);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_MAC4);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_BTOG_MAC4);
	EnableWindow(hChild, bActive);

	hChild = GetDlgItem(hParent, IDC_B_MAC5);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_E_MAC5);
	EnableWindow(hChild, bActive);
	hChild = GetDlgItem(hParent, IDC_BTOG_MAC5);
	EnableWindow(hChild, bActive);
}

BOOL WINAPI GetAControl(HWND hDlg, DWORD ControlValue, BYTE NController, BYTE NControl) {
	HRESULT  hr;
	TCHAR ControlName[32];
	BYTE devicecount, DeviceNum;
	LONG  count;
	BYTE     buffer[256];   //Keyboard Info 
	DIJOYSTATE js;			//Joystick Info

	for(devicecount=0; devicecount<Controller[NController].NDevices; devicecount++) {		
		DeviceNum = (BYTE) Controller[NController].Devices[devicecount];
		if (DInputDev[DeviceNum].lpDIDevice != NULL) {
		
			if((DInputDev[DeviceNum].DIDevInst.dwDevType & DIDEVTYPE_KEYBOARD) == DIDEVTYPE_KEYBOARD){
				ZeroMemory( &buffer, sizeof(buffer) );	
				if FAILED(hr = DInputDev[DeviceNum].lpDIDevice->GetDeviceState(sizeof(buffer),&buffer)) { 
					hr = DInputDev[DeviceNum].lpDIDevice->Acquire();
						while( hr == DIERR_INPUTLOST ) 
							hr = DInputDev[DeviceNum].lpDIDevice->Acquire();
					return FALSE; 
				}

				for(count=0;count<256;count++) {
					if (BUTTONDOWN(buffer, count)) {
						Controller[NController].Input[NControl].Device = DeviceNum;
						Controller[NController].Input[NControl].type = INPUT_TYPE_KEY_BUT;
						Controller[NController].Input[NControl].vkey = (BYTE) count;
						GetAControlValue(hDlg, ControlValue, NController, NControl);
						GetAControlName(NController, NControl, ControlName);
						SetDlgItemText(hDlg,ControlValue,ControlName);
						return TRUE;
					}
				}
			}
	
			if((DInputDev[DeviceNum].DIDevInst.dwDevType & DIDEVTYPE_JOYSTICK) == DIDEVTYPE_JOYSTICK){
				if FAILED(hr = DInputDev[DeviceNum].lpDIDevice->Poll()) { 
					hr = DInputDev[DeviceNum].lpDIDevice->Acquire();
					while( hr == DIERR_INPUTLOST ) 
						hr = DInputDev[DeviceNum].lpDIDevice->Acquire();
					return FALSE; 
				}
				if FAILED( hr = DInputDev[DeviceNum].lpDIDevice->GetDeviceState( sizeof(DIJOYSTATE), &js ) )
					return FALSE;

				for(count=0;count<32;count++) {
					if (BUTTONDOWN(js.rgbButtons, count)) {
						Controller[NController].Input[NControl].Device = DeviceNum;
						Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_BUT;
						Controller[NController].Input[NControl].vkey = (BYTE) count;
						GetAControlValue(hDlg, ControlValue, NController, NControl);
						GetAControlName(NController, NControl, ControlName);
						SetDlgItemText(hDlg,ControlValue,ControlName);
						return TRUE;
					}
				}
	
				if( js.lY < (LONG) -Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_YN;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}
	
				if( js.lY > (LONG) Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_YP;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}
	
				if( js.lX < (LONG) -Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_XN;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}
		
				if( js.lX > (LONG) Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_XP;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}

				if( js.lZ < (LONG) -Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_ZN;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}
	
				if( js.lZ > (LONG) Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_ZP;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}

				if( js.lRy < (LONG) -Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_RYN;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}
	
				if( js.lRy > (LONG) Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_RYP;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}
	
				if( js.lRx < (LONG) -Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_RXN;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}
		
				if( js.lRx > (LONG) Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_RXP;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}

				if( js.lRz < (LONG) -Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_RZN;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}
	
				if( js.lRz > (LONG) Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_RZP;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}

				if( js.rglSlider[0] < (LONG) -Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_SLIDER0N;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}
		
				if( js.rglSlider[0] > (LONG) Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_SLIDER0P;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}

				if( js.rglSlider[1] < (LONG) -Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_SLIDER1N;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}
	
				if( js.rglSlider[1] > (LONG) Controller[NController].SensMin ) {
					Controller[NController].Input[NControl].Device = DeviceNum;
					Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_AXIS;
					Controller[NController].Input[NControl].vkey = DIJOFS_SLIDER1P;
					GetAControlValue(hDlg, ControlValue, NController, NControl);
					GetAControlName(NController, NControl, ControlName);
					SetDlgItemText(hDlg,ControlValue,ControlName);
					return TRUE;
				}

				for (count=0;count<1;count++) {
					if ( (js.rgdwPOV[count] != -1)  && (LOWORD(js.rgdwPOV[count]) != 0xFFFF) ) {
						Controller[NController].Input[NControl].Device = DeviceNum;
						Controller[NController].Input[NControl].type = INPUT_TYPE_JOY_POV;

						if ( js.rgdwPOV[count] == 0 ) {
							switch(count)
							{
								case 0:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV0N; break;
								case 1:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV1N; break;
								case 2:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV2N; break;
								case 3:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV3N; break;
							}	
							GetAControlValue(hDlg, ControlValue, NController, NControl);
							GetAControlName(NController, NControl, ControlName);
							SetDlgItemText(hDlg,ControlValue,ControlName);
							return TRUE;
						}
						if ( js.rgdwPOV[count] == 9000 ) {
							switch(count)
							{
								case 0:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV0E; break;
								case 1:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV1E; break;
								case 2:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV2E; break;
								case 3:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV3E; break;
							}
							GetAControlValue(hDlg, ControlValue, NController, NControl);
							GetAControlName(NController, NControl, ControlName);
							SetDlgItemText(hDlg,ControlValue,ControlName);
							return TRUE;
						}
						if ( js.rgdwPOV[count] == 18000 ) {
							switch(count)
							{
								case 0:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV0S; break;
								case 1:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV1S; break;
								case 2:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV2S; break;
								case 3:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV3S; break;
							}
							GetAControlValue(hDlg, ControlValue, NController, NControl);
							GetAControlName(NController, NControl, ControlName);
							SetDlgItemText(hDlg,ControlValue,ControlName);
							return TRUE;
						}
						if ( js.rgdwPOV[count] == 27000 ) {
							switch(count)
							{
								case 0:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV0W; break;
								case 1:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV1W; break;
								case 2:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV2W; break;
								case 3:
									Controller[NController].Input[NControl].vkey = DIJOFS_POV3W; break;
							}
							GetAControlValue(hDlg, ControlValue, NController, NControl);
							GetAControlName(NController, NControl, ControlName);
							SetDlgItemText(hDlg,ControlValue,ControlName);
							return TRUE;
						}
					}
				}

			}
		}
	}

	return FALSE;
}

BOOL WINAPI GetAControlValue(HWND hDlg, DWORD ControlValue, BYTE NController, BYTE NControl) {
	BUTTONS Buttons;
	
	Buttons.Value  = 0;
	
	switch (ControlValue)
	{
		case IDC_E_DPRIGHT:
			Buttons.R_DPAD = 1;
			break;

		case IDC_E_DPLEFT:
			Buttons.L_DPAD = 1;
			break;

		case IDC_E_DPDOWN:
			Buttons.D_DPAD = 1;
			break;

		case IDC_E_DPUP:
			Buttons.U_DPAD = 1;
			break;

		case IDC_E_START:
			Buttons.START_BUTTON = 1;
			break;

		case IDC_E_ZTRIG:
			Buttons.Z_TRIG = 1;
			break;

		case IDC_E_B:
			Buttons.B_BUTTON = 1;
			break;

		case IDC_E_A:
			Buttons.A_BUTTON = 1;
			break;

		case IDC_E_CRIGHT:
			Buttons.R_CBUTTON = 1;
			break;

		case IDC_E_CLEFT:
			Buttons.L_CBUTTON = 1;
			break;

		case IDC_E_CDOWN:
			Buttons.D_CBUTTON = 1;
			break;

		case IDC_E_CUP:
			Buttons.U_CBUTTON = 1;
			break;

		case IDC_E_RTRIG:
			Buttons.R_TRIG = 1;
			break;

		case IDC_E_LTRIG:
			Buttons.L_TRIG = 1;
			break;

		case IDC_EAS_LEFT:
			Buttons.X_AXIS = -min(128,Controller[NController].SensMax);
			break;

		case IDC_EAS_RIGHT:
			Buttons.X_AXIS = min(127,Controller[NController].SensMax);
			break;

		case IDC_EAS_DOWN:
			Buttons.Y_AXIS = -min(128,Controller[NController].SensMax);
			break;

		case IDC_EAS_UP:
			Buttons.Y_AXIS = min(127,Controller[NController].SensMax);
			break;

		case IDC_E_M1:
			Controller[NController].Input[NControl].button = (DWORD) SendDlgItemMessage(hDlg, IDC_SPINM1, UDM_GETPOS, 0, 0);
			return TRUE;

		case IDC_E_M2:
			Controller[NController].Input[NControl].button = (DWORD) SendDlgItemMessage(hDlg, IDC_SPINM2, UDM_GETPOS, 0, 0);
			return TRUE;

		default:
			return FALSE;
			break;
	}
	Controller[NController].Input[NControl].button = Buttons.Value;

	return TRUE;
}

void WINAPI GetAControlName(BYTE NController, BYTE NControl, TCHAR ControlName[32]) {
	TCHAR KeyControlName[16];
	
	switch(Controller[NController].Input[NControl].type)
	{
		case INPUT_TYPE_KEY_BUT:
			GetKeyName(Controller[NController].Input[NControl].vkey, KeyControlName);
			wsprintf(ControlName,"%s", KeyControlName);
			break;

		case INPUT_TYPE_JOY_BUT:
			wsprintf(ControlName,"Joy%d %d", Controller[NController].Input[NControl].Device, Controller[NController].Input[NControl].vkey);
			break;

		case INPUT_TYPE_JOY_AXIS:
			switch(Controller[NController].Input[NControl].vkey)
			{
				case DIJOFS_YN:
					wsprintf(ControlName,"Joy%d -Y", Controller[NController].Input[NControl].Device);
					break;
				
				case DIJOFS_YP:
					wsprintf(ControlName,"Joy%d +Y", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_XN:
					wsprintf(ControlName,"Joy%d -X", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_XP:
					wsprintf(ControlName,"Joy%d +X", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_ZN:
					wsprintf(ControlName,"Joy%d -Z", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_ZP:
					wsprintf(ControlName,"Joy%d +Z", Controller[NController].Input[NControl].Device);
					break;
				
				case DIJOFS_RYN:
					wsprintf(ControlName,"Joy%d -Ry", Controller[NController].Input[NControl].Device);
					break;
				
				case DIJOFS_RYP:
					wsprintf(ControlName,"Joy%d +Ry", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_RXN:
					wsprintf(ControlName,"Joy%d -Rx", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_RXP:
					wsprintf(ControlName,"Joy%d +Rx", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_RZN:
					wsprintf(ControlName,"Joy%d -Rz", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_RZP:
					wsprintf(ControlName,"Joy%d +Rz", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_SLIDER0N:
					wsprintf(ControlName,"Joy%d -Sl0", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_SLIDER0P:
					wsprintf(ControlName,"Joy%d +Sl0", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_SLIDER1N:
					wsprintf(ControlName,"Joy%d -Sl1", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_SLIDER1P:
					wsprintf(ControlName,"Joy%d +Sl1", Controller[NController].Input[NControl].Device);
					break;
			}
			break;
		
		case INPUT_TYPE_JOY_POV:
			switch(Controller[NController].Input[NControl].vkey)
			{
				case DIJOFS_POV0N:
					wsprintf(ControlName,"Joy%d NPoV0", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV0E:
					wsprintf(ControlName,"Joy%d EPoV0", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV0S:
					wsprintf(ControlName,"Joy%d SPoV0", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV0W:
					wsprintf(ControlName,"Joy%d WPoV0", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV1N:
					wsprintf(ControlName,"Joy%d NPoV1", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV1E:
					wsprintf(ControlName,"Joy%d EPoV1", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV1S:
					wsprintf(ControlName,"Joy%d SPoV1", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV1W:
					wsprintf(ControlName,"Joy%d WPoV1", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV2N:
					wsprintf(ControlName,"Joy%d NPoV2", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV2E:
					wsprintf(ControlName,"Joy%d EPoV2", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV2S:
					wsprintf(ControlName,"Joy%d SPoV2", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV2W:
					wsprintf(ControlName,"Joy%d WPoV2", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV3N:
					wsprintf(ControlName,"Joy%d NPoV3", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV3E:
					wsprintf(ControlName,"Joy%d EPoV3", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV3S:
					wsprintf(ControlName,"Joy%d SPoV3", Controller[NController].Input[NControl].Device);
					break;

				case DIJOFS_POV3W:
					wsprintf(ControlName,"Joy%d WPoV3", Controller[NController].Input[NControl].Device);
					break;
			}
			break;
		
		default:
			wsprintf(ControlName," ");
			break;
	}
}

void WINAPI GetKeyName(BYTE Value, TCHAR KeyControlName[16]) {
	switch (Value)
	{
		case DIK_0: wsprintf(KeyControlName,"0"); break;
		case DIK_1: wsprintf(KeyControlName,"1"); break;
		case DIK_2: wsprintf(KeyControlName,"2"); break;
		case DIK_3: wsprintf(KeyControlName,"3"); break;
		case DIK_4: wsprintf(KeyControlName,"4"); break;
		case DIK_5: wsprintf(KeyControlName,"5"); break;
		case DIK_6: wsprintf(KeyControlName,"6"); break;
		case DIK_7: wsprintf(KeyControlName,"7"); break;
		case DIK_8: wsprintf(KeyControlName,"8"); break;
		case DIK_9: wsprintf(KeyControlName,"9"); break;
		case DIK_A: wsprintf(KeyControlName,"A"); break;
		case DIK_B: wsprintf(KeyControlName,"B"); break;
		case DIK_C: wsprintf(KeyControlName,"C"); break;
		case DIK_D: wsprintf(KeyControlName,"D"); break;
		case DIK_E: wsprintf(KeyControlName,"E"); break;
		case DIK_F: wsprintf(KeyControlName,"F"); break;
		case DIK_G: wsprintf(KeyControlName,"G"); break;
		case DIK_H: wsprintf(KeyControlName,"H"); break;
		case DIK_I: wsprintf(KeyControlName,"I"); break;
		case DIK_J: wsprintf(KeyControlName,"J"); break;
		case DIK_K: wsprintf(KeyControlName,"K"); break;
		case DIK_L: wsprintf(KeyControlName,"L"); break;
		case DIK_M: wsprintf(KeyControlName,"M"); break;
		case DIK_N: wsprintf(KeyControlName,"N"); break;
		case DIK_O: wsprintf(KeyControlName,"O"); break;
		case DIK_P: wsprintf(KeyControlName,"P"); break;
		case DIK_Q: wsprintf(KeyControlName,"Q"); break;
		case DIK_R: wsprintf(KeyControlName,"R"); break;
		case DIK_S: wsprintf(KeyControlName,"S"); break;
		case DIK_T: wsprintf(KeyControlName,"T"); break;
		case DIK_U: wsprintf(KeyControlName,"U"); break;
		case DIK_V: wsprintf(KeyControlName,"V"); break;
		case DIK_W: wsprintf(KeyControlName,"W"); break;
		case DIK_X: wsprintf(KeyControlName,"X"); break;
		case DIK_Y: wsprintf(KeyControlName,"Y"); break;
		case DIK_Z: wsprintf(KeyControlName,"Z"); break;
		case DIK_F1: wsprintf(KeyControlName,"F1"); break;
		case DIK_F2: wsprintf(KeyControlName,"F2"); break;
		case DIK_F3: wsprintf(KeyControlName,"F3"); break;
		case DIK_F4: wsprintf(KeyControlName,"F4"); break;
		case DIK_F5: wsprintf(KeyControlName,"F5"); break;
		case DIK_F6: wsprintf(KeyControlName,"F6"); break;
		case DIK_F7: wsprintf(KeyControlName,"F7"); break;
		case DIK_F8: wsprintf(KeyControlName,"F8"); break;
		case DIK_F9: wsprintf(KeyControlName,"F9"); break;
		case DIK_F10: wsprintf(KeyControlName,"F10"); break;
		case DIK_F11: wsprintf(KeyControlName,"F11"); break;
		case DIK_F12: wsprintf(KeyControlName,"F12"); break;
		case DIK_LEFT: wsprintf(KeyControlName,"Key Left"); break;
		case DIK_RIGHT: wsprintf(KeyControlName,"Key Right"); break;
		case DIK_UP: wsprintf(KeyControlName,"Key Up"); break;
		case DIK_DOWN: wsprintf(KeyControlName,"Key Down"); break;
		case DIK_NUMPAD0: wsprintf(KeyControlName,"Num 0"); break;
		case DIK_NUMPAD1: wsprintf(KeyControlName,"Num 1"); break;
		case DIK_NUMPAD2: wsprintf(KeyControlName,"Num 2"); break;
		case DIK_NUMPAD3: wsprintf(KeyControlName,"Num 3"); break;
		case DIK_NUMPAD4: wsprintf(KeyControlName,"Num 4"); break;
		case DIK_NUMPAD5: wsprintf(KeyControlName,"Num 5"); break;
		case DIK_NUMPAD6: wsprintf(KeyControlName,"Num 6"); break;
		case DIK_NUMPAD7: wsprintf(KeyControlName,"Num 7"); break;
		case DIK_NUMPAD8: wsprintf(KeyControlName,"Num 8"); break;
		case DIK_NUMPAD9: wsprintf(KeyControlName,"Num 9"); break;
		case DIK_ADD: wsprintf(KeyControlName,"Num +"); break;
		case DIK_DECIMAL: wsprintf(KeyControlName,"Num ."); break;
		case DIK_DIVIDE: wsprintf(KeyControlName,"Num /"); break;
		case DIK_NUMLOCK: wsprintf(KeyControlName,"Num Lock"); break;
		case DIK_MULTIPLY: wsprintf(KeyControlName,"Num *"); break;
		case DIK_NUMPADENTER: wsprintf(KeyControlName,"Num Enter"); break;
		case DIK_NUMPADEQUALS: wsprintf(KeyControlName,"Num ="); break;
		case DIK_SUBTRACT: wsprintf(KeyControlName,"Num -"); break;
		case DIK_APOSTROPHE: wsprintf(KeyControlName,"'"); break;
		case DIK_APPS: wsprintf(KeyControlName,"App"); break;
		case DIK_BACK: wsprintf(KeyControlName,"Backspace"); break;
		case DIK_BACKSLASH: wsprintf(KeyControlName,"\\"); break;
		case DIK_CAPITAL: wsprintf(KeyControlName,"Caps Lock"); break;
		case DIK_COMMA: wsprintf(KeyControlName,","); break;
		case DIK_DELETE: wsprintf(KeyControlName,"Delete"); break;
		case DIK_END: wsprintf(KeyControlName,"End"); break;
		case DIK_EQUALS: wsprintf(KeyControlName,"="); break;
		case DIK_ESCAPE: wsprintf(KeyControlName,"Esc"); break;
		case DIK_GRAVE: wsprintf(KeyControlName,"`"); break;
		case DIK_HOME: wsprintf(KeyControlName,"Home"); break;
		case DIK_INSERT: wsprintf(KeyControlName,"Insert"); break;
		case DIK_LBRACKET: wsprintf(KeyControlName,"["); break;
		case DIK_LCONTROL: wsprintf(KeyControlName,"L Ctrl"); break;
		case DIK_LMENU: wsprintf(KeyControlName,"L Alt"); break;
		case DIK_LSHIFT: wsprintf(KeyControlName,"L Shift"); break;
		case DIK_LWIN: wsprintf(KeyControlName,"L Win"); break;
		case DIK_MINUS: wsprintf(KeyControlName,"-"); break;
		case DIK_NEXT: wsprintf(KeyControlName,"Page Down"); break;
		case DIK_PAUSE: wsprintf(KeyControlName,"Pause"); break;
		case DIK_PERIOD: wsprintf(KeyControlName,"."); break;
		case DIK_PRIOR: wsprintf(KeyControlName,"Page Up"); break;
		case DIK_RBRACKET: wsprintf(KeyControlName,"]"); break;
		case DIK_RCONTROL: wsprintf(KeyControlName,"R Ctrl"); break;
		case DIK_RETURN: wsprintf(KeyControlName,"Enter"); break;
		case DIK_RMENU: wsprintf(KeyControlName,"R Alt"); break;
		case DIK_RSHIFT: wsprintf(KeyControlName,"R Shift"); break;
		case DIK_RWIN: wsprintf(KeyControlName,"R Win"); break;
		case DIK_SCROLL: wsprintf(KeyControlName,"Scroll"); break;
		case DIK_SEMICOLON: wsprintf(KeyControlName,";"); break;
		case DIK_SLASH: wsprintf(KeyControlName,"/"); break;
		case DIK_SPACE: wsprintf(KeyControlName,"Space"); break;
		case DIK_SYSRQ: wsprintf(KeyControlName,"SysRq"); break;
		case DIK_TAB: wsprintf(KeyControlName,"Tab"); break;

		default: wsprintf(KeyControlName,"%d", Value); break;
	}
}