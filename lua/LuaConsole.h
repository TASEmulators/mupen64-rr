
#ifndef MUPEN64RR_LUACONSOLE_H
#define MUPEN64RR_LUACONSOLE_H

#include <windows.h>
#include <string>

#include "include/lua.hpp"

#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
namespace boost
{
template<typename T> class shared_ptr;
}

typedef std::string String;

class Lua : private boost::noncopyable
{
	::lua_State* _lua;
	bool _isRunning;
	
public:
	Lua();
	~Lua();

	void PushLightUserData(void* data);
	void SetField(int index, const char* key);
	void Register(const String& name, lua_CFunction function);
	void Register(const char* name, lua_CFunction function);

	boost::optional<int> Run(String scriptFilePath);
};

class LuaWindow
{
	HWND _handle;
	Lua _vm;
	String _scriptFilePath;
	
private:
	void Initialize();

public:
	static const char* LuaRegistryKey;

	String ScriptFilePath() const;
	void ScriptFilePath(String filePath);

	LuaWindow(HINSTANCE hInst, HWND parent);
	LuaWindow(HWND hDialog);
	~LuaWindow();
	
	bool Show();

	void ConsoleWrite(const char* str);
	void StateChangeButton_Clicked(HWND sender);
	
	bool operator<(const LuaWindow& other) const;
};

BOOL CALLBACK LuaWindowProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool IsLuaConsoleMessage(MSG* msg);

void NewLuaScript(HINSTANCE hInst, HWND parent);

void CloseAllLuaScript();

#endif
