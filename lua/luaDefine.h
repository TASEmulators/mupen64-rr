#ifndef LUA_DEFINE_H
#define LUA_DEFINE_H

#ifdef WIN32


#define LUA_CONSOLE

#ifdef LUA_CONSOLE

#define LUA_TRACEINTERP
#define LUA_GUI
#define LUA_SPEEDMODE
#define LUA_JOYPAD

#define LUA_BREAKPOINTSYNC_PURE
#define LUA_BREAKPOINTSYNC_INTERP
#define LUA_PCBREAK_PURE
#define LUA_PCBREAK_INTERP
#define LUA_TRACELOG
#define LUA_TRACEPURE

#define LUA_EMUPAUSED_WORK
#define LUA_WINDOWMESSAGE

//#define LUA_LIB


#endif

#endif

#endif
