
# Project: Mupen 64 Windows
# Makefile created by Dev-C++ 4.9.9.2

BOOSTLIBINC = -I"E:\Programming\C++\Boost C++ Libraries"
# BOOSTLIBINC = -I"../include"
# CPP  = g++.exe -finput-charset=cp932 -fexec-charset=cp932 -g -O0
# CC   = gcc.exe -finput-charset=cp932 -fexec-charset=cp932 -g -O0
CXX = g++.exe
CPP  = $(CXX)
CC   = gcc.exe

src_dir = ..

WINDRES = windres.exe
RES  = mupen64_private.res
OBJTEMP = main/rom.o memory/memory.o r4300/x86/debug.o main/win/configdialog.o r4300/r4300.o main/unzip.o r4300/interupt.o memory/tlb.o memory/dma.o memory/pif.o r4300/exception.o r4300/recomp.o r4300/pure_interp.o r4300/x86/rjump.o main/ioapi.o r4300/x86/assemble.o r4300/x86/gr4300.o r4300/special.o r4300/x86/gspecial.o r4300/regimm.o r4300/x86/gregimm.o r4300/tlb.o r4300/x86/gtlb.o r4300/cop0.o r4300/x86/gcop0.o r4300/bc.o r4300/x86/gbc.o r4300/cop1_s.o r4300/x86/gcop1_s.o r4300/cop1_d.o r4300/x86/gcop1_d.o r4300/cop1_w.o r4300/x86/gcop1_w.o r4300/cop1_l.o r4300/x86/gcop1_l.o r4300/cop1.o r4300/x86/gcop1.o memory/flashram.o main/md5.o main/mupenIniApi.o main/win/dumplist.o main/win/rombrowser.o main/win/timers.o main/win/translation.o main/win/main_win.o main/win/inifunctions.o main/savestates.o main/win/Config.o main/win/guifuncs.o main/win/RomSettings.o main/win/GUI_LogWindow.o main/win/kaillera.o main/win/commandline.o main/vcr.o r4300/x86/regcache.o main/win/vcr_compress.o main/vcr_resample.o main/adler32.o lua/LuaConsole.o
objs = $(foreach objfile, $(OBJTEMP), $(src_dir)/$(objfile)) $(RES)

LIBS =  -L"lib" -L"zlib" -L"../lua" -mwindows -L. -lz -llua5.1 -lcomctl32 -lwinmm -lvfw32 -s -march=pentium2 -mmmx
INCS =  -I"include"  -I"zlib" -I"../lua"
CXXINCS =  -I"lib/gcc/mingw32/3.4.2/include"  -I"include/c++/3.4.2/backward"  -I"include/c++/3.4.2/mingw32"  -I"include/c++/3.4.2"  -I"include"  -I"zlib" -I"../main" $(BOOSTLIBINC)

bin  = mupen64.exe
CFLAGS = $(INCS) -D__WIN32__ -DX86 -DVCR_SUPPORT -Wall -fno-strict-aliasing -fomit-frame-pointer   -fexpensive-optimizations -O3 -march=pentium2 -mmmx
CXXFLAGS = $(CFLAGS) $(CXXINCS) -fexpensive-optimizations -O3 -march=pentium2 -mmmx
CPPFLAGS = $(CXXFLAGS)
RM = rm -f


.SUFFIXES: .o .c .cpp
.PHONY: all all-before all-after clean clean-custom

all: all-before mupen64.exe all-after

clean: clean-custom
	${RM} $(objs) $(bin)

$(bin): $(objs)
	$(CXX) $(objs) -o "mupen64.exe" $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

mupen64_private.res: mupen64_private.rc rsrc.rc 
	$(WINDRES) -i mupen64_private.rc --input-format=rc -o mupen64_private.res -O coff 
