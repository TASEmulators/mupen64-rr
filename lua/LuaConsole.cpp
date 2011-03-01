
#include "LuaConsole.h"

#include <tchar.h>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include "../../winproject/resource.h"
#include "win/DebugInfo.hpp"

#include <assert.h>
#if defined(__cplusplus) && !defined(_MSC_VER)
extern "C"
{
#endif
#include "win/main_win.h"
#include "../../memory/memory.h"
#if defined(__cplusplus) && !defined(_MSC_VER)
}
#endif

#ifdef _MSC_VER
//#ifdef _DEBUG
//#pargma comment(lib, "lua5.1_debug.lib")
//#else
#pragma comment(lib, "lua5.1.lib")
//#endif
#endif


#define MUPEN64RR_ASSERT(expr) assert(expr)


struct EmulationLock
{
	EmulationLock()
	{
		MUPEN64RR_DEBUGINFO("Emulation Lock");
		pauseEmu(FALSE);
	}
	~EmulationLock()
	{
		resumeEmu(FALSE);
		MUPEN64RR_DEBUGINFO("Emulation Unlock");
	}
};

String OpenFileDialog()
{
	::EmulationLock lock;

	OPENFILENAME ofn;
	char filename[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = mainHWND;

	ofn.lpstrFilter = 
		"Lua Script Files (*.lua)\0*.lua\0"
		"\0"
		;

	ofn.nFilterIndex = 1;
	ofn.lpstrFile =  filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "lua";
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	ofn.lpstrInitialDir = NULL;
	
	if(!GetOpenFileName(&ofn))
		return "";
	return ofn.lpstrFile;
}

namespace EmuLua
{
	//private
	LuaWindow* GetCurrentWindow(lua_State* lua)
	{
		MUPEN64RR_ASSERT(lua != NULL);
		::lua_getfield(lua, LUA_REGISTRYINDEX, LuaWindow::LuaRegistryKey);
		 LuaWindow* window = static_cast<LuaWindow*>(lua_touserdata(lua, -1));
		 MUPEN64RR_ASSERT(window != NULL);
		 return window;
	}

	//public
	int Print(lua_State* lua)
	{
		MUPEN64RR_DEBUGINFO("print");
		const char* message = ::lua_tostring(lua, -1);
		MUPEN64RR_ASSERT(message != NULL);

		EmuLua::GetCurrentWindow(lua)->ConsoleWrite(message);

		MUPEN64RR_DEBUGINFO("print end");
		return 0;
	}
}

	Lua::Lua()
		: _lua(lua_open()), _isRunning(false)
	{
		luaL_openlibs(_lua);
	}
	Lua::~Lua()
	{
		MUPEN64RR_ASSERT(_lua != NULL);
		lua_close(_lua);
		_lua = NULL;
		MUPEN64RR_DEBUGINFO("lua destructed.");
	}

	void Lua::PushLightUserData(void* data)
	{
		::lua_pushlightuserdata(_lua, data);
	}

	void Lua::SetField(int index, const char* key)
	{
		::lua_setfield(_lua, index, key);
	}

	void Lua::Register(const String& name, lua_CFunction function)
	{
		this->Register(name.c_str(), function);
	}

	void Lua::Register(const char* name, lua_CFunction function)
	{
		lua_register(_lua, name, function);
	}

	boost::optional<int> Lua::Run(String scriptFilePath)
	{
		MUPEN64RR_DEBUGINFO("Lua::Run");
		int result = luaL_dofile(_lua, scriptFilePath.c_str());
		if(result == 0)
		{
			return boost::none;
		}
		return result;
	}

	const char* LuaWindow::LuaRegistryKey = "LuaWindow::LuaRegistryKey";

	LuaWindow::LuaWindow(HINSTANCE hInst, HWND parent)
		: _handle(CreateDialog(hInst, "IDD_LUAWINDOW", parent, LuaWindowProc)),
		_vm(), _scriptFilePath()
	{
		this->Initialize();
	}

	LuaWindow::LuaWindow(HWND hDialog)
		: _handle(hDialog),
		_vm(), _scriptFilePath()
	{
		this->Initialize();
	}

	void LuaWindow::Initialize()
	{
		MUPEN64RR_ASSERT(_handle != NULL);
		//Register the this pointer
		_vm.PushLightUserData(static_cast<void*>(this));
		_vm.SetField(LUA_REGISTRYINDEX, LuaWindow::LuaRegistryKey);

		_vm.Register("print", EmuLua::Print);
	}

	LuaWindow::~LuaWindow()
	{
		DestroyWindow(_handle);
	}
	
	bool LuaWindow::Show()
	{
		return !!ShowWindow(_handle, SW_SHOW);
	}
	
	void LuaWindow::ConsoleWrite(const char* str)
	{
		HWND hConsole = ::GetDlgItem(_handle, IDC_TEXTBOX_LUACONSOLE);
		MUPEN64RR_ASSERT(hConsole != NULL);

		int length = GetWindowTextLength(hConsole);
		if(length >= 250000)
		{
			SendMessage(hConsole, EM_SETSEL, 0, length/2);
			SendMessage(hConsole, EM_REPLACESEL, false, (LPARAM)"");
			length = GetWindowTextLength(hConsole);
		}
		SendMessage(hConsole, EM_SETSEL, length, length);
		SendMessage(hConsole, EM_REPLACESEL, false, (LPARAM)str);
	}

	void LuaWindow::StateChangeButton_Clicked(HWND sender)
	{
		MUPEN64RR_DEBUGINFO("begin");
		
		if(_scriptFilePath.empty())
		{
			ShowMessage("script filepath empty");
			return;
		}
		_vm.Run(_scriptFilePath);
		MUPEN64RR_DEBUGINFO("end");
	}

	String LuaWindow::ScriptFilePath() const
	{
		return this->_scriptFilePath;
	}

	void LuaWindow::ScriptFilePath(String filePath)
	{
		_scriptFilePath = filePath;
		HWND hTextPath = ::GetDlgItem(_handle, IDC_TEXTBOX_LUASCRIPTPATH);
		MUPEN64RR_ASSERT(hTextPath != NULL);
		::SetWindowTextA(hTextPath, _scriptFilePath.c_str());
	}
	
	bool LuaWindow::operator<(const LuaWindow& other) const
	{
		return _handle < other._handle;
	}


typedef boost::ptr_map<HWND, LuaWindow> map_type;
map_type luaWindows;

BOOL CALLBACK LuaWindowProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool handled = false;
	map_type::iterator it = luaWindows.find(hDlg);
	if(it == luaWindows.end())
	{
		if(uMsg == WM_INITDIALOG)
		{
			MUPEN64RR_DEBUGINFO("WM_INITDIALOG");
			handled = true;
		}
		return handled ? TRUE : FALSE;
	}
	LuaWindow& target = *(it->second);
	
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			MUPEN64RR_ASSERT(false);
			handled = true;
		}	break;
		case WM_COMMAND:
		{
			int controlID =  wParam & 0xFFFF;
			int notifyCode = (wParam >> 16) & 0xFFFF;
			HWND hControl = (HWND)lParam;
			
			if(hControl == NULL)
				return false;

			switch(controlID)
			{
				case IDC_BUTTON_LUASTATE:
				{
					MUPEN64RR_ASSERT(hControl == GetDlgItem(hDlg, IDC_BUTTON_LUASTATE));

					target.StateChangeButton_Clicked(hControl);
					handled = true;
				}	break;
				case IDC_BUTTON_LUABROWSE:
				{
					String filePath = ::OpenFileDialog();
					target.ScriptFilePath(filePath);
				}	break;
			}
		}	break;
		case WM_CLOSE:
		{
			luaWindows.erase(it);
			handled = true;
		}	break;
		case WM_DESTROY:
		{
			MUPEN64RR_DEBUGINFO("call WM_DESTROY");
		}	break;
	}
	
	return handled ? TRUE : FALSE;
}

bool IsLuaConsoleMessage(MSG* msg)
{
	if(luaWindows.find(msg->hwnd) != luaWindows.end())
	{
		IsDialogMessage(msg->hwnd, msg);
		return true;
	}
	return false;
}

void NewLuaScript(HINSTANCE hInst, HWND parent)
{
	HWND hDialog = CreateDialog(hInst, "IDD_LUAWINDOW", parent, LuaWindowProc);
	LuaWindow* window = new LuaWindow(hDialog);
	window->Show();
	luaWindows.insert(hDialog, window);
}

void CloseAllLuaScript()
{
	luaWindows.clear();
}
