#ifndef REDELBE_H
#define REDELBE_H

#include <windows.h>

#include "IniFile.h"
#include "PatchUtils.h"

#define DLL_NAME "dinput8.dll"
#define REDELBE_VERSION u"3.1"

#define EXPORT 	__declspec(dllexport)
#define PUBLIC	EXPORT

#define PROCESS_NAME "doa6.exe"

#define PATCHES_PATH	"REDELBE/Patches"
#define INI_PATH		"REDELBE/REDELBE.ini"
#define EXE_PATH	"doa6.exe" /* Must be in lower case */

#define CONTENT_ROOT	""

extern std::string myself_path;
extern IniFile ini;
extern DWORD initial_tick;

static inline void *GetAddrFromRel(void *addr, uint32_t offset=0)
{
	uint8_t *addr_8 = (uint8_t *)addr;
	return (void *)(addr_8+4+offset + *(int32_t *)addr);
}

static inline void SetRelAddr(void *addr, void *final_addr)
{
	PatchUtils::Write32(addr, Utils::DifPointer(final_addr, addr) - 4);
}

static inline size_t RelAddress(void *addr)
{
	return (size_t)addr - (size_t)GetModuleHandle(nullptr);
}

#endif //REDELBE_H
