
#ifndef MUPEN64RR_DEBUGINFO_HPP
#define MUPEN64RR_DEBUGINFO_HPP

#include <string>
#include <sstream>

#if defined(__cplusplus) && !defined(_MSC_VER)
extern "C" {
#endif
#include <tchar.h>

#include "win/GUI_LogWindow.h"
#if defined(__cplusplus) && !defined(_MSC_VER)
}
#endif

class StringBuilder
{
	typedef StringBuilder this_type;
	typedef std::basic_string<TCHAR> string_type;
	typedef std::basic_ostream<TCHAR> ostream_type;
	typedef std::basic_ostringstream<TCHAR> ostringstream_type;
	typedef std::basic_stringstream<TCHAR> stringstream_type;
	
	stringstream_type _ss;
public:
	StringBuilder()
		: _ss()
	{
	}
	
	~StringBuilder()
	{
	}
	
	template<typename T>
	this_type& Append(T const& str)
	{
		_ss << str;
		return *this;
	}

	this_type& Append(ostream_type& (*pf)(ostream_type&))
	{
		_ss << pf;
		return *this;
	}

	template<typename T>
	this_type& operator<<(T const& str)
	{
		_ss << str;
		return *this;
	}

	this_type& operator<<(ostream_type& (*pf)(ostream_type&))
	{
		_ss << pf;
		return *this;
	}

	string_type ToString() { return _ss.str(); }

	void Clear()
	{
		_ss.str(_T("")); //clear the buffer
		_ss.clear(ostringstream_type::goodbit); //clear the stream state.
	}
};

#ifdef _MSC_VER
#define MUPEN64_RR_FUNCTION_SIG __FUNCTION__
#else
#define MUPEN64_RR_FUNCTION_SIG __PRETTY_FUNCTION__
#endif
#define MUPEN64RR_DEBUGINFO(message) \
	ShowInfo(const_cast<TCHAR*>(StringBuilder().Append(message).ToString().c_str())); \
	ShowInfo(const_cast<TCHAR*>(StringBuilder() \
		.Append("at: ").Append(MUPEN64_RR_FUNCTION_SIG).Append(" ") \
		.Append(__LINE__).Append(": ").Append(__FILE__).ToString().c_str()))
		
#endif
