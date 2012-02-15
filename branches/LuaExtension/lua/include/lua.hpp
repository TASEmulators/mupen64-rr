// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++
#include "LuaConsole.h"

#ifdef LUA_LIB
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#pragma comment(lib, "lua5.1.lib")
#else
#include "src/lua.h"
#include "src/lualib.h"
#include "src/lauxlib.h"
#endif

