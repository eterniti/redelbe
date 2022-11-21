CC = gcc
CXX = g++

OUTPUT := dinput8.dll
OUTPUT_COPY := "/S/SteamLibrary/steamapps/common/Dead or Alive 6/dinput8.dll"

OBJS=main.o game.o rdbemu.o layer2.o ui.o misc.o debug_enc.o debug.o 
OBJS += ../eternity_common/crypto/sha1.o ../eternity_common/crypto/md5.o ../eternity_common/crypto/rijndael.o
OBJS += ../eternity_common/Utils.o ../eternity_common/BaseFile.o  
OBJS += ../eternity_common/Stream.o ../eternity_common/MemoryStream.o ../eternity_common/FixedMemoryStream.o ../eternity_common/FileStream.o
OBJS += ../eternity_common/IniFile.o
OBJS += ../eternity_common/DOA6/RdbFile.o ../eternity_common/DOA6/RnkFile.o
OBJS += ../eternity_common/PatchUtils.o ../eternity_common/EPatchFile.o
OBJS += ../eternity_common/tinyxml/tinyxml.o ../eternity_common/tinyxml/tinystr.o ../eternity_common/tinyxml/tinyxmlerror.o ../eternity_common/tinyxml/tinyxmlparser.o
CFLAGS +=-Wall -I. -I../eternity_common -std=gnu99 -mms-bitfields -s -O2 -masm=intel -shared -Wl,--subsystem,windows,--kill-at,--enable-stdcall-fixup
CFLAGS += -static -static-libgcc -static-libstdc++
CXXFLAGS +=-Wall -Wno-strict-aliasing -I../eternity_common -O2 -std=c++11 -mms-bitfields -DTIXML_USE_STL -DDEV_BUILD
CXXFLAGS += -static-libgcc -static-libstdc++ -static -Wl,--subsystem,windows,--kill-at 
LDFLAGS=-L. 
LIBS = -lstdc++ -lversion -lminhook -lz

all: $(OUTPUT)	
	cp $(OUTPUT) $(OUTPUT_COPY)

clean:
	rm -f $(OUTPUT) *.o
	rm -f ../eternity_common/*.o
	rm -f ../eternity_common/tinyxml/*.o	
	rm -f ../eternity_common/crypto/*.o
	rm -f ../eternity_common/DOA6/*.o

$(OUTPUT): $(OBJS)
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)

