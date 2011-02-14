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

#ifndef _CONFIG_H_INCLUDED__
#define _CONFIG_H_INCLUDED__

#define IDT_TIMER1 1
#define IDT_TIMER2 2

LRESULT CALLBACK ConfigDlgProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);
void WINAPI Start_Timer (HWND hDlg);
void WINAPI Initialize_Controller_Display (HWND hDlg, BYTE NController);
void WINAPI Dis_En_AbleApply (HWND hParent, BOOL bActive);
void WINAPI Dis_En_AbleControls (HWND hParent, BOOL bActive);
BOOL WINAPI GetAControl (HWND hDlg, DWORD ControlValue, BYTE NController, BYTE NControl);
BOOL WINAPI GetAControlValue (HWND hDlg, DWORD ControlValue, BYTE NController, BYTE NControl);
void WINAPI GetAControlName (BYTE NController, BYTE NControl, TCHAR ControlName[32]);
void WINAPI GetKeyName (BYTE Value, TCHAR KeyControlName[16]);

LRESULT CALLBACK MacroDlgProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);
void WINAPI InitializeMacroChecks(HWND hDlg, BYTE NController, BYTE NControl);
void WINAPI SetMacroButtonValue(HWND hDlg, BYTE NController, BYTE NControl);

#endif