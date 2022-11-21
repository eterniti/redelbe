#include <windows.h>
#include <psapi.h>
#include <wininet.h>
#include "IniFile.h"
#include "EPatchFile.h"
#include "PatchUtils.h"
#include "redelbe.h"
#include "rdbemu.h"
#include "debug.h"

static HMODULE myself_dll;
static HMODULE patched_dll;
std::string myself_path;

static bool in_game_process()
{
	char szPath[MAX_PATH];
	
	GetModuleFileName(NULL, szPath, MAX_PATH);
	_strlwr(szPath);
	
	// A very poor aproach, I know
	return (strstr(szPath, PROCESS_NAME) != NULL);
}

extern "C"
{

PUBLIC void NsiConnectToServer() { OutputDebugString("dummy\n"); }
PUBLIC void NsiDisconnectFromServer() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcDeregisterChangeNotification() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcDeregisterChangeNotificationEx() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcEnumerateObjectsAllParameters() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcGetAllParameters() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcGetAllParametersEx() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcGetParameter() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcGetParameterEx() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcRegisterChangeNotification() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcRegisterChangeNotificationEx() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcSetAllParameters() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcSetAllParametersEx() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcSetParameter() { OutputDebugString("dummy\n"); }
PUBLIC void NsiRpcSetParameterEx() { OutputDebugString("dummy\n"); }

} // extern "C"


static bool load_dll(bool critical)
{
	static const std::vector<const char *> exports =
	{
		"NsiConnectToServer",
		"NsiDisconnectFromServer",
		"NsiRpcDeregisterChangeNotification",
		"NsiRpcDeregisterChangeNotificationEx",
		"NsiRpcEnumerateObjectsAllParameters",
		"NsiRpcGetAllParameters",
		"NsiRpcGetAllParametersEx",
		"NsiRpcGetParameter",
		"NsiRpcGetParameterEx",
		"NsiRpcRegisterChangeNotification",
		"NsiRpcRegisterChangeNotificationEx",
		"NsiRpcSetAllParameters",
		"NsiRpcSetAllParametersEx",
		"NsiRpcSetParameter",
		"NsiRpcSetParameterEx",
	};	
	
	static char mod_path[2048];
	static char original_path[256];	
	static bool loaded = false;
	
	if (loaded)
		return true;
	
	HMODULE myself = myself_dll;

	GetModuleFileNameA(myself, mod_path, sizeof(mod_path));
	
	myself_path = Utils::NormalizePath(mod_path);
	myself_path = myself_path.substr(0, myself_path.rfind('/')+1);	
	DPRINTF("Myself path = %s\n", myself_path.c_str());
	
	{	
		if (GetSystemDirectory(original_path, sizeof(original_path)) == 0)
			return false;

		strncat(original_path, "\\winnsi.dll", sizeof(original_path));
	}
	
	patched_dll = LoadLibrary(original_path);		
	if (!patched_dll)
	{
		if (critical)
			UPRINTF("Cannot load original DLL (%s).\nLoadLibrary failed with error %u\n", original_path, (unsigned int)GetLastError());
				
		return false;
	}
	
	DPRINTF("original DLL path: %s   base=%p\n", original_path, patched_dll);
		
	for (auto &export_name : exports)
	{
		uintptr_t ordinal = (uintptr_t)export_name;
		
		uint8_t *orig_func = (uint8_t *)GetProcAddress(patched_dll, export_name);
		
		if (!orig_func)
		{
			if (ordinal < 0x1000)			
			{
				DPRINTF("Warning: ordinal function %Id doesn't exist in this system.\n", ordinal);
				continue;		
			}
			else
			{
				UPRINTF("Failed to get original function: %s\n", export_name);			
				return false;
			}
		}
		
		uint8_t *my_func = (uint8_t *)GetProcAddress(myself, export_name);
		
		
		if (!my_func)
		{
			if (ordinal < 0x1000)			
				UPRINTF("Failed to get my function: %Id\n", ordinal);
			else
				UPRINTF("Failed to get my function: %s\n", export_name);
			
			return false;
		}
		
		if (ordinal < 0x1000)
			;//DPRINTF("%Id: address of microsoft: %p, address of mine: %p\n", ordinal, orig_func, my_func);
		else
			;//DPRINTF("%s: address of microsoft: %p, address of mine: %p\n", export_name, orig_func, my_func);
		
		//DPRINTF("Content of microsoft func: %02X%02X%02X%02X%02X\n", orig_func[0], orig_func[1], orig_func[2], orig_func[3], orig_func[4]);
		//DPRINTF("Content of my func: %02X%02X%02X%02X%02X\n", my_func[0], my_func[1], my_func[2], my_func[3], my_func[4]);
		
		PatchUtils::Hook(my_func, nullptr, orig_func);
		//DPRINTF("Content of my func after patch: %02X%02X%02X%02X%02X\n", my_func[0], my_func[1], my_func[2], my_func[3], my_func[4]);
	}
	
	loaded = true;
	return true;
}

static void unload_dll()
{
	if (patched_dll)
	{
		FreeLibrary((HMODULE)patched_dll);
		patched_dll = nullptr;
	}
}

static bool load_patch_file(const std::string &path, bool, void *)
{
	EPatchFile epf(myself_path+"winnsi.dll");

	int enabled;
	bool enabled_b = false;
	std::string setting;
	
	if (!epf.CompileFromFile(path))
	{
		UPRINTF("Failed to compile file \"%s\"\n", path.c_str());
		exit(-1);
	}
	
	//DPRINTF("File %s compiled.\n", path.c_str());		
	if ((enabled = epf.GetEnabled(setting)) < 0)
	{	
		//ini.GetBooleanValue("Patches", setting, &enabled_b, false);
		enabled = enabled_b;
	}
	
	if (!enabled)
		return true;
	
	for (EPatch &patch : epf)
	{
		if ((enabled = patch.GetEnabled(setting)) < 0)
		{
			//ini.GetBooleanValue("Patches", setting, &enabled_b, false);
			enabled = enabled_b;
		}
		
		if (!enabled)
			continue;
		
		if (!patch.Apply())
		{
			UPRINTF("Failed to apply patch \"%s:%s\"\n", epf.GetName().c_str(), patch.GetName().c_str());
			exit(-1);
		}
	}	
	
	return true;
}

static void load_patches()
{
	Utils::VisitDirectory(myself_path+PATCHES_PATH, true, false, false, load_patch_file);
}

LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
	static const std::vector<std::string> ignore_modules = { "KERNELBASE.dll" };
	
	void *fault_addr;
	void *fault_addr_rel = nullptr;	
	HMODULE fault_mod;
	std::string fault_mod_name;
	char path[MAX_PATH];
	
	fault_addr = (void *)ExceptionInfo->ExceptionRecord->ExceptionAddress;	
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)fault_addr, &fault_mod);	
	
	if (fault_mod)
	{
		fault_addr_rel = (void *)((uintptr_t)fault_addr - (uintptr_t)fault_mod);
		GetModuleFileNameA(fault_mod, path, MAX_PATH);
		fault_mod_name = path;
		
		std::string fn = Utils::GetFileNameString(fault_mod_name);
		
		for (const std::string &mod : ignore_modules)
		{
			if (Utils::ToLowerCase(mod) == Utils::ToLowerCase(fn))
				return EXCEPTION_CONTINUE_SEARCH;
		}
		
	}
	
	DPRINTF("*************************** EXCEPTION CAUGHT ***************************\n");
	
	if (fault_mod)
	{
		DPRINTF("Exception code=0x%x at address %p (%p) Faulting module = %s\n", (unsigned int)ExceptionInfo->ExceptionRecord->ExceptionCode, fault_addr, fault_addr_rel, fault_mod_name.c_str());
	}
	else
	{
		DPRINTF("Exception code=0x%x at address %p\n", (unsigned int)ExceptionInfo->ExceptionRecord->ExceptionCode, fault_addr);
	}

	DPRINTF("RIP = %p, RSP = %p, RAX = %p, RBX = %p\n", (void *)ExceptionInfo->ContextRecord->Rip, (void *)ExceptionInfo->ContextRecord->Rsp, 
														(void *)ExceptionInfo->ContextRecord->Rax, (void *)ExceptionInfo->ContextRecord->Rbx);
														
	DPRINTF("RCX = %p, RDX = %p, R8 = %p,  R9 = %p\n", 	(void *)ExceptionInfo->ContextRecord->Rcx, (void *)ExceptionInfo->ContextRecord->Rdx, 
														(void *)ExceptionInfo->ContextRecord->R8,  (void *)ExceptionInfo->ContextRecord->R9);
														
	DPRINTF("R10 = %p, R11 = %p, R12 = %p, R13 = %p\n", (void *)ExceptionInfo->ContextRecord->R10, (void *)ExceptionInfo->ContextRecord->R11, 
														(void *)ExceptionInfo->ContextRecord->R12, (void *)ExceptionInfo->ContextRecord->R13);
														
	DPRINTF("R14 = %p, R15 = %p, RDI = %p, RSI = %p\n", (void *)ExceptionInfo->ContextRecord->R14, (void *)ExceptionInfo->ContextRecord->R15, 
														(void *)ExceptionInfo->ContextRecord->Rdi, (void *)ExceptionInfo->ContextRecord->Rsi);
														
	DPRINTF("************************************************************************\n");
	
	return EXCEPTION_CONTINUE_SEARCH;
}

static void start()
{
	const char *my_dll_name = "winnsi.dll";	
	DPRINTF("REDELBE. Exe base = %p. My Dll base = %p. My dll name: %s\n", GetModuleHandle(NULL), GetModuleHandle(my_dll_name), my_dll_name);	
	
	load_patches();	
	rdbemu_init();
	//AddVectoredExceptionHandler(1, ExceptionHandler);
}

VOID WINAPI GetStartupInfoW_Patched(LPSTARTUPINFOW lpStartupInfo)
{
	static bool started = false;
	
	// This function is only called once by the game but... just in case
	if (!started)
	{	
		set_debug_level(2);	
		
		if (!load_dll(true))
			exit(-1);	
		
		start();
		started = true;		
	}
	
	GetStartupInfoW(lpStartupInfo);
}

typedef BOOL WINAPI (* K32EnumProcessModulesExType)(HANDLE, HMODULE *, DWORD, LPDWORD, DWORD);
K32EnumProcessModulesExType K32EnumProcessModulesEx;

BOOL WINAPI K32EnumProcessModulesExPatched(HANDLE  hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded, DWORD dwFilterFlag)
{
	//DPRINTF("Called from %p\n", BRA(0));
	BOOL ret = K32EnumProcessModulesEx(hProcess, lphModule, cb, lpcbNeeded, dwFilterFlag);
	
	if (ret && lpcbNeeded && lphModule)
	{
		DWORD num = *lpcbNeeded / sizeof(HMODULE);		
				
		for (DWORD i = 0; i < num; i++)
		{
			//DPRINTF("Moudle %p\n", lphModule[i]);
			
			if (lphModule[i] == myself_dll)
			{
				//DPRINTF("Found myself.\n");
				
				if (i != (num-1))
				{
					memmove(&lphModule[i], &lphModule[i+1], (num-i-1)*sizeof(HMODULE));
				}
				
				*lpcbNeeded = (num-1) * sizeof(HMODULE);
				//DPRINTF("---Removed myself from modules list\n");
				return ret;
			}
		}
	}
	
	return ret;
}

extern "C" BOOL WINAPI EXPORT DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		
			static bool done = false;
			
			if (done)
				return TRUE;
			
			set_debug_level(2); 	

			if (in_game_process())
			{					
				done = true;
				
				myself_dll = GetModuleHandle("winnsi.dll");
				K32EnumProcessModulesEx = (K32EnumProcessModulesExType)GetProcAddress(GetModuleHandle("KERNEL32.dll"), "K32EnumProcessModulesEx");
				
				// We need this patch or the game will shut down us.
				if (!PatchUtils::HookImport("KERNEL32.DLL", "K32EnumProcessModulesEx", (void *)K32EnumProcessModulesExPatched))
				{
					DPRINTF("Failed to hook import of K32EnumProcessModulesEx");
				}
				
				CreateMutexA(nullptr, FALSE, "REDELBE_INSTANCE");
				if (GetLastError() == ERROR_ALREADY_EXISTS)
				{
					UPRINTF("An instance of redelbe already exists.\n");							
					exit(-1);
				}
				
				load_dll(false);
				
				if (!PatchUtils::HookImport("KERNEL32.dll", "GetStartupInfoW", (void *)GetStartupInfoW_Patched))
				{
					UPRINTF("GetStartupInfoW hook failed.\n");
					return TRUE;
				}
				
				//Main();				
			}	
			
		break;
		
		case DLL_PROCESS_DETACH:		
			
			if (!lpvReserved)
				unload_dll();
			
		break;
	}
	
	return TRUE;
}
