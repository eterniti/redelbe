#include <windows.h>
#include <psapi.h>
#include <wininet.h>

#include <unordered_map>

#include "IniFile.h"
#include "EPatchFile.h"
#include "PatchUtils.h"
#include "redelbe.h"
#include "rdbemu.h"
#include "game.h"
#include "debug.h"

static HMODULE myself_dll;
static HMODULE patched_dll;

#ifdef DEV_BUILD
static std::unordered_map<uintptr_t, std::string> script_functions;
static std::unordered_set<uint32_t> hunted_hashes;
#endif

std::string myself_path;
IniFile ini;
DWORD initial_tick;

typedef uintptr_t (* script_function_type)(void *);
uintptr_t on_script_called(void *, script_function_type, void *);

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

/*PUBLIC void NsiConnectToServer() { OutputDebugString("dummy\n"); }
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
PUBLIC void NsiRpcSetParameterEx() { OutputDebugString("dummy\n"); }*/

PUBLIC HRESULT DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, void *punkOuter)
{
	DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
	return E_INVALIDARG;
}

PUBLIC HRESULT DllCanUnloadNow()
{
	DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
	return -1;
}

PUBLIC HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
	return 0x80040111;
}

PUBLIC HRESULT DllRegisterServer()
{
	DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
	return E_UNEXPECTED;
}

PUBLIC HRESULT DllUnregisterServer()
{
	DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
	return -1;
}

typedef void (* VFType)(void *, const char16_t *, size_t);
VFType version_func; 

PUBLIC void SetupBranding(VFType orig)
{
	version_func = orig;
}

PUBLIC void branding(void *rcx, const char16_t *version, size_t lengh)
{
	std::u16string my_version = version + std::u16string(u" REDELBE ") + REDELBE_VERSION;
	version_func(rcx, my_version.c_str(), my_version.length()+1);
}

#ifdef DEV_BUILD

typedef uintptr_t (* RSFType)(void *, uintptr_t *);
static RSFType register_script_function;

PUBLIC void SetupRegisterScriptFunc(RSFType orig)
{
	register_script_function = orig;
}

PUBLIC uintptr_t register_script_function_patched(void *obj, uintptr_t *reginfo)
{
	script_functions[reginfo[0]] = (const char *)reginfo[2];
	return register_script_function(obj, reginfo);
}

PUBLIC void OnDebugScriptPatchLocated(uint8_t *addr)
{
	// Ok, we got 12 instructions to do it
	uint8_t patch[12];
	
	patch[0] = 0x48, patch[1] = 0x89, patch[2] = 0xC2; // mov rdx, rax
	patch[3] = 0x49, patch[4] = 0x89, patch[5] = 0xD8; // mov r8, rbx
	patch[6] = 0xE8; // call XXXXXXXX
	patch[11] = 0x90;
	
	PatchUtils::Copy(addr, patch, sizeof(patch));
	PatchUtils::HookCall(addr+6, nullptr, (void *)on_script_called);
}

PUBLIC uint32_t game_hash_func_patched(const char *a1, int a2, int a3)
{
	int v3; 
	int first_char = (a2 == 0) ? a2 : a2 / 31;
	std::string s = a1;

	while ( 1 )
	{
		v3 = a3;

		char c = *a1;
		if (!c)
			break;

		a3 *= 31;
		a2 += 31 * v3 * c;
		a1++;
	}
  
	uint32_t hash = (uint32_t)a2;
  
	auto it = hunted_hashes.find(hash);
	if (it == hunted_hashes.end())
	{
		if (first_char == 0)
			DPRINTF("0x%08x, \"%s\" (called from %p)\n", hash, s.c_str(), BRA(0));
		else
			DPRINTF("0x%08x, \"%c%s\" (called from %p)\n", hash, first_char, s.c_str(), BRA(0));

		hunted_hashes.insert(hash);
	}
  
	return hash;
}


#endif

typedef uintptr_t (* CPPExceptionType)(const char *);
static CPPExceptionType CPPException;
int cpp_exception_stack_trace_level;

PUBLIC void SetupCppExceptionHook(CPPExceptionType orig)
{
	CPPException = orig;
	
	ini.GetIntegerValue("Debug", "cpp_exception_stack_trace_level", &cpp_exception_stack_trace_level, 3);
}

PUBLIC uintptr_t CPPExceptionPatched(const char *error)
{
	if (cpp_exception_stack_trace_level > 1)
	{
		DPRINTF("----------C++ exception: %s\n", error);
		PrintStackTrace(cpp_exception_stack_trace_level);
	}
	else
	{
		DPRINTF("----------C++ exception (called from %p): %s\n", BRA(0), error);
	}
	
	return CPPException(error);
}

/*PUBLIC bool testo(void *pthis)
{
	typedef struct
	{
		uint64_t u1, u2;
	} UnkStruct;
	CHECK_STRUCT_SIZE(UnkStruct, 0x10);
	
	uintptr_t ptr = PatchUtils::Read64(pthis, 0x35A8) + 2;
	PatchUtils::WriteData64(pthis, ptr, 0x35A8);
	
	uint8_t player_index = PatchUtils::Read8(pthis, 0x3560); 
	void *player = (void *)PatchUtils::InvokeRegisterFunction(0x195B5D0, player_index);
	if (!player)
		return true;
	
	void *obj = ((uint8_t *)player) + 0x1AD0;
	uint8_t x;
	UnkStruct st;
	
	PatchUtils::InvokeRegisterFunction(0x195B280, (uintptr_t)&x, PatchUtils::Read64(obj, 8));
	PatchUtils::InvokeRegisterFunction(0x19650A0, (uintptr_t)&st, x);
	
	void *damaged_event = (void *)PatchUtils::Read64(0x266FCC8);
	uint64_t val = st.u1;
	
	while (damaged_event)
	{
		PatchUtils::InvokeVirtualRegisterFunction(damaged_event, 8, (uintptr_t)&val);
		damaged_event = (void *)PatchUtils::Read64(damaged_event, 8);
	}
	
	return true;
}*/

} // extern "C"


static bool load_dll(bool critical)
{
	static const std::vector<const char *> exports =
	{
		/*"NsiConnectToServer",
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
		"NsiRpcSetParameterEx",*/
		
		"DirectInput8Create",
		"DllCanUnloadNow",
		"DllGetClassObject",
		"DllRegisterServer",
		"DllUnregisterServer"
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

		strncat(original_path, "\\" DLL_NAME, sizeof(original_path));
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

static bool evaluate_setting(const std::string &setting, const std::string &path)
{
	bool enabled_b = true;
	
	std::vector<std::string> comp;
	if (Utils::GetMultipleStrings(setting, comp, ':') == 2)
	{
		ini.GetBooleanValue(comp[0], comp[1], &enabled_b, false);
	}
	else if (Utils::EndsWith(setting, "()"))
	{
		typedef bool (* ConditionalFunction)();
		
		const std::string function_name = setting.substr(0, setting.rfind("()"));
		ConditionalFunction condition = (ConditionalFunction) GetProcAddress(myself_dll, function_name.c_str());
		
		if (!condition)
		{
			DPRINTF("Warning: condition function \"%s\" referenced in file \"%s\" doesn't exist in this .dll; it will be evaluated to false.\n", function_name.c_str(), Utils::GetFileNameString(path).c_str());
		}
		
		if (condition && condition())
		{
			enabled_b = true;
		}
		else
		{
			enabled_b = false;
		}
	}		
	else
	{
		DPRINTF("Warning: file \"%s\" referenced setting \"%s\" that has no ':'; in these cases, it always evaluates to true.\n", Utils::GetFileNameString(path).c_str(), setting.c_str());
	}
	
	return enabled_b;
}

static bool load_patch_file(const std::string &path, bool, void *)
{
	EPatchFile epf(myself_path+DLL_NAME);

	int enabled;
	std::string setting;
	
	if (!epf.CompileFromFile(path))
	{
		UPRINTF("Failed to compile file \"%s\"\n", path.c_str());
		exit(-1);
	}
	
	//DPRINTF("File %s compiled.\n", path.c_str());		
	if ((enabled = epf.GetEnabled(setting)) < 0)
	{	
		enabled = evaluate_setting(setting, path);
	}
	
	if (!enabled)
		return true;
	
	for (EPatch &patch : epf)
	{
		if ((enabled = patch.GetEnabled(setting)) < 0)
		{
			enabled = evaluate_setting(setting, path);
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

	DPRINTF("RBP = %p, RSP = %p, RAX = %p, RBX = %p\n", (void *)ExceptionInfo->ContextRecord->Rbp, (void *)ExceptionInfo->ContextRecord->Rsp, 
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

#ifdef DEV_BUILD

std::string script_params(void *obj)
{
	uint16_t num_params = script_get_num_params(obj);
	ScriptValue params[16];
	
	if (num_params == 0 || num_params > 16 || !script_get_params(obj, params))
		return "";
	
	std::string ret;
	
	for (uint16_t i = 0; i < num_params; i++)
	{
		if (params[i].type == SCRIPT_DT_INT32)
		{
			ret += Utils::UnsignedToHexString(params[i].value.val32, true);
		}
		else if (params[i].type == SCRIPT_DT_FLOAT)
		{
			ret += Utils::ToString(params[i].value.val_float);
		}
		else if (params[i].type == SCRIPT_DT_BOOL)
		{
			ret += (params[i].value.val_bool) ? "true" : "false";
		}
		else if (params[i].type == SCRIPT_DT_STRING)
		{
			ret += *params[i].value.val_str;
		}
		else
		{
			ret += "UT:" + Utils::ToString(params[i].type) + ":" + Utils::UnsignedToHexString(params[i].value.val64, true);
		}
		
		if (i != (num_params-1))
			ret += ", ";
	}
	
	return ret;
}

uintptr_t on_script_called(void *obj, script_function_type func, void *saved_rbx)
{
	uint64_t *obj64 = (uint64_t *)obj;
	uint8_t *obj8 = (uint8_t *)obj;
	
	// Restore thigns destroyed by patch
	obj64[8/8] = (uint64_t)saved_rbx;
	obj8[0x10] = 0;
	
	auto it = script_functions.find((uintptr_t)func);
	if (it != script_functions.end())
	{
		DPRINTF("Script called: %s %s\n", it->second.c_str(), script_params(obj).c_str());
	}
	else
	{
		uint8_t *base = (uint8_t *)GetModuleHandle(nullptr);
		DPRINTF("Script called: %x %s\n", (uint32_t) ((uintptr_t)func - (uintptr_t)base), script_params(obj).c_str());
	}
	
	return func(obj);
}

static DWORD WaitForSingleObject_Patched(HANDLE hHandle, DWORD dwMilliseconds)
{
	DPRINTF("WaitForSingleObject %p (called from 0x%x, thread 0x%x)\n", hHandle, Utils::DifPointer(BRA(0), GetModuleHandle(NULL)), (uint32_t)GetCurrentThreadId());
	return WaitForSingleObject(hHandle, dwMilliseconds);
}

static BOOL SetEvent_Patched(HANDLE hEvent)
{
	uint32_t ret_addr = Utils::DifPointer(BRA(0), GetModuleHandle(NULL));
	
	DPRINTF("SetEvent %p (called from 0x%x, thread 0x%x)\n", hEvent, ret_addr, (uint32_t)GetCurrentThreadId());
		
	if (ret_addr == 0x264cba)
	{
		PrintStackTrace(8);
	}
	
	return SetEvent(hEvent);
}

static DWORD SignalObjectAndWait_Patched(HANDLE hObjectToSignal, HANDLE hObjectToWaitOn, DWORD  dwMilliseconds, BOOL bAlertable)
{
	DPRINTF("SignalObjectAndWait %p %p (called from 0x%x, thread 0x%x)\n", hObjectToSignal, hObjectToWaitOn, Utils::DifPointer(BRA(0), GetModuleHandle(NULL)), (uint32_t)GetCurrentThreadId());
	return SignalObjectAndWait(hObjectToSignal, hObjectToWaitOn, dwMilliseconds, bAlertable);
}

/*static DWORD testo(LPVOID)
{
	Sleep(40*1000);
	
	uint8_t *base = (uint8_t *)GetModuleHandle(nullptr);
	uintptr_t *pobj = (uintptr_t *)(base + 0x25EDA90);
	typedef void (* set_visibility_type)(uintptr_t, uintptr_t, uint32_t, uint32_t, bool);
	set_visibility_type set_visibility = (set_visibility_type)(base + 0xFFC9D0);
	typedef void (* set_text_utf8_type)(uintptr_t, uintptr_t, uint32_t, uint32_t, uint32_t, const char *);
	set_text_utf8_type set_text_utf8 = (set_text_utf8_type)(base + 0xFFCDE0);
	
	for (int i = 0; i < 0x1; i++)
	{
		//set_visibility(*pobj, 0, 0x67e8cce, i, true);
		set_text_utf8(*pobj, 0, 0xF121F112, 0, 0x4, "Slot: KAS_COS_004 (Work: KAS_COS_006) Mod: Leifang Deluxe Breakable to Nude bigger butt (in slot 10-12) ^10~DECIDE~ Aceptar ^10~CANCEL~ Atrás.");
	}
	
	return 0;
}*/

/*typedef void (* AnimFuncType)(float *, uint64_t *, float, float);
static AnimFuncType anim_func;

void anim_func_patched(float *ret, uint64_t * data, float a, float b)
{
	anim_func(ret, data, a, b);
	
	if (data[0] == 0x3FFF2F5851DFE101ULL && data[1] == 0x4BFDFF0D203D99ULL && data[2] == 0xFEDDB00C8D02B72ULL && data[3] == 0xFF714FF8AEFE677)
	{
		static int counter = 0;
		
		DPRINTF("(%d) % f %f - %f %f %f\n", counter, a, b, ret[0], ret[1], ret[2]);
		counter++;
	}
}*/

static void dev_patches()
{
	bool debug_threading;
	
	ini.GetBooleanValue("Debug", "debug_threading", &debug_threading, false);
	
	if (debug_threading)
	{
		PatchUtils::HookImport("KERNEL32.dll", "WaitForSingleObject", (void *)WaitForSingleObject_Patched);
		PatchUtils::HookImport("KERNEL32.dll", "SetEvent", (void *)SetEvent_Patched);
		PatchUtils::HookImport("KERNEL32.dll", "SignalObjectAndWait", (void *)SignalObjectAndWait_Patched);
	}
	
	//CreateThread(NULL, 0, testo, NULL, 0, NULL);
	
	//uint8_t *base = (uint8_t *)GetModuleHandle(nullptr);
	//PatchUtils::Hook(base+0xB505D0, (void **)&anim_func, (void *)anim_func_patched);
}

#endif

static void start()
{
	const char *my_dll_name = DLL_NAME;	
	DPRINTF("REDELBE. Exe base = %p. My Dll base = %p. My dll name: %s\n", GetModuleHandle(NULL), GetModuleHandle(my_dll_name), my_dll_name);	
	
	ini.LoadFromFile(INI_PATH);
	
	load_patches();	
	rdbemu_init();	
	
#ifdef DEV_BUILD
	dev_patches();
#endif
	
	bool enable_exception_handler;
	
	ini.GetBooleanValue("Debug", "enable_exception_handler", &enable_exception_handler, false);
	
	if (enable_exception_handler)
	{
		//DPRINTF("Exception handler enabled.\n");
		AddVectoredExceptionHandler(1, ExceptionHandler);
	}
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
			
			initial_tick = GetTickCount();
			set_debug_level(2); 	

			if (in_game_process())
			{					
				done = true;
				
				myself_dll = GetModuleHandle(DLL_NAME);
				K32EnumProcessModulesEx = (K32EnumProcessModulesExType)GetProcAddress(GetModuleHandle("KERNEL32.dll"), "K32EnumProcessModulesEx");
				
				// We need this patch or the game will shut down us.
				if (!PatchUtils::HookImport("KERNEL32.DLL", "K32EnumProcessModulesEx", (void *)K32EnumProcessModulesExPatched))
				{
					DPRINTF("Failed to hook import of K32EnumProcessModulesEx");
				}
				
				CreateMutexA(nullptr, FALSE, "REDELBE_INSTANCE");
				if (GetLastError() == ERROR_ALREADY_EXISTS)
				{
					UPRINTF("An instance of REDELBE already exists.\n");							
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
