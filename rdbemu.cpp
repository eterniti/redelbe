#include <windows.h>
#include <unordered_map>
#include "PatchUtils.h"
#include "DOA6/RdbFile.h"
#include "redelbe.h"
#include "rdbemu.h"
#include "layer2.h"
#include "game.h"
#include "debug.h"

#define NUM_LAYER2	5

static RdbFile *CharacterEditor_rdb, *EventMaker3_rdb, *FieldEditor4_rdb, *KIDSSystemResource_rdb, *MaterialEditor_rdb, *RRPreview_rdb, *ScreenLayout_rdb, *system_rdb;

static Layer2Mod *l2_mods[NUM_LAYER2];
std::vector<int> l2_indexes = { 1, 0, 3, 2, 4 };

static std::unordered_map<HANDLE, RdbVirtualFile *> handle_map;
static std::unordered_map<std::string, RdbVirtualFile *> memory_rdb_files; 

static std::unordered_map<std::string, VirtualRdbEntry> virtual_files;

static std::unordered_map<uint32_t,std::string> log_external_files;
static std::unordered_map<std::string, RdbDebugInternal *> log_internal_files;
static std::unordered_map<HANDLE, RdbDebugInternal *> log_internal_handles;
static std::unordered_map<HANDLE, std::string> log_numeric_handles;

static std::string data_dir, rdb_dir, l2_dir;
static bool log_virtual, log_external, log_internal, log_numeric;

static bool get_hex_name(const std::string &fn, uint32_t *ret)
{
	if (!Utils::BeginsWith(fn, "0x", false))
		return false;
	
	if (fn.length() == 2)
		return false;
	
	for (size_t i = 2; i < fn.length(); i++)
	{
		char ch = tolower(fn[i]);
		
		if (ch == '.')
			break;
		
		bool ok = false;
		
		if (ch >= '0' && ch <= '9')
			ok = true;
		else if (ch >= 'a' && ch <= 'f')
			ok = true;		
		
		if (!ok)
			return false;
	}
	
	*ret = Utils::GetUnsigned(fn);
	return true;
}

static HANDLE get_next_handle()
{
	static uint64_t next_handle = 0x1000000000000000; // Should be safe to use
	
	HANDLE ret = (HANDLE)next_handle;
	next_handle++;
	
	if (next_handle >= 0x8000000000000000) // Will never happen...
		next_handle = 0x1000000000000000;
	
	return ret;
}

static HANDLE handle_layer2(const std::string &file, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	for (int i : l2_indexes)
	{
		if (i < 0)
			continue;
		
		Layer2Mod *mod = l2_mods[i];
		if (!mod)
			continue;
		
		std::string file_name = Utils::GetFileNameString(file);
		uint32_t hash;
		
		if (!get_hex_name(file_name, &hash))
			continue;
		
		VirtualRdbEntry *ventry = nullptr;
		
		auto it_c = mod->character_files->find(hash);
		if (it_c != mod->character_files->end())
		{
			ventry = &it_c->second;
		}
		else
		{
			auto it_m = mod->material_files->find(hash);
			if (it_m != mod->material_files->end())
			{
				ventry = &it_m->second;
			}
			else
			{
				auto it_k = mod->kids_files->find(hash);
				if (it_k != mod->kids_files->end())
				{
					ventry = &it_k->second;
				}
				else
				{
					auto it_f = mod->field4_files->find(hash);
					if (it_f != mod->field4_files->end())
					{
						ventry = &it_f->second;
					}
					else
					{
						auto it_r = mod->rrp_files->find(hash);
						if (it_r != mod->rrp_files->end())
						{
							ventry = &it_r->second;
						}
					}
				}
			}
		}
		
		if (!ventry)
			continue;
		
		//DPRINTF("%s\n", ventry->real_path.c_str());
		
		HANDLE hReal = CreateFileA(ventry->real_path.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		if (hReal == INVALID_HANDLE_VALUE)
			return hReal;
		
		/* Added in 1.04, to update the file size (eg, to support live-modding) */
		LARGE_INTEGER file_size;
		if (ventry->region_size == 0 && GetFileSizeEx(hReal, &file_size))
		{
			RDBEntry *entry2 = (RDBEntry *)ventry->entry2.data();
			
			entry2->c_size = entry2->file_size = ventry->entry1->file_size = file_size.QuadPart;					
			entry2->entry_size = entry2->file_size + (uint32_t)ventry->entry2.size();
		}
		/* */
		RdbVirtualFile *vfile = new RdbVirtualFile(ventry->entry2.data(), ventry->entry2.size(), hReal);
		vfile->rdb = ventry->rdb;
		
		if (log_virtual)
		{
			DPRINTF("Open virtual (L2 \"%s\") %s\n", mod->name.c_str(), Utils::GetFileNameString(ventry->real_path).c_str());
		}
						
		HANDLE hRet = get_next_handle();				
		handle_map[hRet] = vfile;
		
		return hRet;
	}
	
	return INVALID_HANDLE_VALUE;
}

static HANDLE CreateFileW_Patched(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if (lpFileName && dwDesiredAccess == GENERIC_READ)
	{
		std::string file_path = Utils::NormalizePath(Utils::Ucs2ToUtf8((const char16_t *)lpFileName));	// TODO: I shouldn't convert to UTF8, but to local charset...
		//DPRINTF("Open file %s   %x \n", file_path.c_str(), (unsigned int)dwDesiredAccess);

		if (Utils::BeginsWith(file_path, myself_path, false))
		{
			auto it = memory_rdb_files.find(Utils::ToLowerCase(file_path));
			if (it != memory_rdb_files.end())
			{
				//DPRINTF("File %s is memory rdb\n", file_path.c_str());
				
				HANDLE hRet = get_next_handle();				
				handle_map[hRet] = it->second;
				return hRet;
			}
			
			HANDLE hRet = handle_layer2(file_path, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
			if (hRet != INVALID_HANDLE_VALUE)
				return hRet;
			
			auto it2 = virtual_files.find(Utils::ToLowerCase(file_path));
			if (it2 != virtual_files.end())
			{				
				VirtualRdbEntry &ventry = it2->second;	
				
				//DPRINTF("Virtual in (%s)\n", ventry.real_path.c_str());

				HANDLE hReal = CreateFileA(ventry.real_path.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
				if (hReal == INVALID_HANDLE_VALUE)
					return hReal;
				
				/* Added in 1.04, to update the file size (eg, to support live-modding) */
				LARGE_INTEGER file_size;
				if (ventry.region_size == 0 && GetFileSizeEx(hReal, &file_size))
				{
					RDBEntry *entry2 = (RDBEntry *)ventry.entry2.data();
					
					entry2->c_size = entry2->file_size = ventry.entry1->file_size = file_size.QuadPart;					
					entry2->entry_size = entry2->file_size + (uint32_t)ventry.entry2.size();
				}
				/* */
				
				RdbVirtualFile *vfile;

				if (ventry.region_size == 0)
				{
					vfile = new RdbVirtualFile(ventry.entry2.data(), ventry.entry2.size(), hReal);
					
					if (log_virtual)
					{
						DPRINTF("Open virtual %s\n", Utils::GetFileNameString(ventry.real_path).c_str());
					}
				}
				else
				{					
					if ((ventry.original_flags & RDB_FLAG_EXTERNAL))
					{
						vfile = new RdbVirtualFile(nullptr, 0, hReal);
						
						if (log_external)
						{
							std::string name;
							ventry.rdb->GetFileNameByID(ventry.entry1->file_id, name);
							DPRINTF("Open external %s\n", name.c_str());
						}
					}
					else
					{
						vfile = new RdbVirtualFile(ventry.entry2.data(), ventry.entry2.size(), hReal, ventry.region_start, ventry.region_size);
						
						if (log_internal) // This is really a internal file 
						{
							std::string name;
							ventry.rdb->GetFileNameByID(ventry.entry1->file_id, name);
							DPRINTF("Open internal %s\n", name.c_str());
						}
					}
				}
				
				vfile->rdb = ventry.rdb;
				

								
				HANDLE hRet = get_next_handle();				
				handle_map[hRet] = vfile;
				//DPRINTF("File %s is virtualized. (%s) Handle = %p\nn", file_path.c_str(), Utils::GetFileNameString(ventry.real_path).c_str(), hRet);
				return hRet;
			}
			else if (log_external && Utils::BeginsWith(file_path, data_dir, false))
			{
				uint32_t hash;
				
				if (get_hex_name(Utils::GetFileNameString(file_path), &hash))
				{
					auto it3 = log_external_files.find(hash);
					if (it3 != log_external_files.end())
					{
						DPRINTF("Open external %s\n", it3->second.c_str());
					}
				}
			}
			else if (log_numeric && Utils::EndsWith(file_path, ".bin", false) && !Utils::EndsWith(file_path, ".rdb.bin", false))
			{
				HANDLE hRet = CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
				if (hRet != INVALID_HANDLE_VALUE)
				{
					log_numeric_handles[hRet] = Utils::GetFileNameString(file_path);
				}
				
				return hRet;
			}
		}
	}
	
	return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

static HANDLE CreateFileW_Patched_Log(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE hRet = CreateFileW_Patched(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	
	if (hRet != INVALID_HANDLE_VALUE && dwDesiredAccess == GENERIC_READ && lpFileName && wcsstr(lpFileName, L".rdb.bin"))
	{
		std::string file_path = Utils::ToLowerCase(Utils::NormalizePath(Utils::Ucs2ToUtf8((const char16_t *)lpFileName)));	// TODO: I shouldn't convert to UTF8, but to local charset...
		auto it = log_internal_files.find(file_path);
		if (it != log_internal_files.end())
		{
			log_internal_handles[hRet] = it->second;
		}
	}
	
	return hRet;
}

static BOOL CloseHandle_Patched(HANDLE hObject)
{
	auto it = handle_map.find(hObject);
	if (it != handle_map.end())
	{
		RdbVirtualFile *vfile = it->second;
		//DPRINTF("Closing handle %p\n", it->first);
		
		if (!vfile->rdb)
		{
			// File is a memory rdb, just reset file pointer			
			vfile->SetFilePointer(0);
		}
		else
		{
			delete vfile;
		}
		
		handle_map.erase(it);
		return TRUE;
	}
	else if (log_numeric)
	{
		auto it = log_numeric_handles.find(hObject);
		if (it != log_numeric_handles.end())
		{
			log_numeric_handles.erase(it);
		}
	}
	
	//DPRINTF("Close %p %d\n", hObject, bRet);
	return CloseHandle(hObject);	
}

static BOOL CloseHandle_Patched_Log(HANDLE hObject)
{
	auto it = log_internal_handles.find(hObject);
	if (it != log_internal_handles.end())
	{
		log_internal_handles.erase(it);
	}
	
	return CloseHandle_Patched(hObject);
}

static BOOL ReadFile_Patched(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	//DPRINTF("Read %u bytes\n", (unsigned int)nNumberOfBytesToRead);
	
	auto it = handle_map.find(hFile);
	if (it != handle_map.end())
	{
		//DPRINTF("Forwarding ReadFile %p %x to vfile\n", hFile, (unsigned int)nNumberOfBytesToRead);
		return it->second->Read(lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);		
	}
	else if (log_numeric)
	{
		auto it = log_numeric_handles.find(hFile);
		if (it != log_numeric_handles.end())
		{
			uint64_t pos = 0;
			
			if (lpOverlapped)
			{
				pos = lpOverlapped->Offset;
				pos |= ((uint64_t)lpOverlapped->OffsetHigh << 32);
			}
			else
			{
				LARGE_INTEGER cur = { };
				LARGE_INTEGER after = { };
				
				if (SetFilePointerEx(hFile, cur, &after, FILE_CURRENT))
				{
					pos = after.QuadPart;
				}
			}
			
			DPRINTF("%s READ offset=0x%I64x bytes=0x%x\n", it->second.c_str(), pos, (uint32_t)nNumberOfBytesToRead);
		}
	}
	
	//DPRINTF("ReadFile %p %x\n", hFile, (unsigned int)nNumberOfBytesToRead);
	return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);	
}

static BOOL ReadFile_Patched_Log(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	auto it = log_internal_handles.find(hFile);
	if (it != log_internal_handles.end())
	{
		if (lpOverlapped)
		{
			uint64_t file_pointer = lpOverlapped->Offset;
			file_pointer |= ((uint64_t)lpOverlapped->OffsetHigh << 32);
			
			auto it2 = it->second->offsets_map.find(file_pointer);
			if (it2 != it->second->offsets_map.end())
			{
				DPRINTF("Open internal %s\n", it2->second.c_str());
			}
		}
		else
		{
			//DPRINTF("********Warning not overlapped.\n");
		}
	}
	
	return ReadFile_Patched(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

// Game doesn't use it. They use instead the overlapped in read if they need to advance the file pointer.
/*static BOOL SetFilePointerEx_Patched(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod)
{
	DPRINTF("SetFilePointer %p %Ix %x\n", hFile, liDistanceToMove.QuadPart, (unsigned int)dwMoveMethod);
	return SetFilePointerEx(hFile, liDistanceToMove, lpNewFilePointer, dwMoveMethod);
}*/

static BOOL GetFileSizeEx_Patched(HANDLE hFile, PLARGE_INTEGER lpFileSize)
{
	auto it = handle_map.find(hFile);
	if (it != handle_map.end() && lpFileSize)
	{
		//DPRINTF("Forwarding GetFileSize %p vfile\n", hFile);
		lpFileSize->QuadPart = it->second->GetSize();
		return TRUE;
	}
	
	//DPRINTF("GetFileSizeEx %p\n", hFile);
	return GetFileSizeEx(hFile, lpFileSize);
}

/*static HANDLE FindFirstFileW_Patched(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData)
{
	if (lpFileName)
		DPRINTF("FindFirstFileW %S\n", lpFileName);
	
	return FindFirstFileW(lpFileName, lpFindFileData);
}

static HANDLE FindFirstFileExA_Patched(LPCSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{
	if (lpFileName)
		DPRINTF("FindFirstFileExA %s\n", lpFileName);
	
	return FindFirstFileExA(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);
}

static HANDLE FindFirstFileExW_Patched(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{
	if (lpFileName)
		DPRINTF("FindFirstFileExW %S\n", lpFileName);
	
	return FindFirstFileExW(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);
}*/

static BOOL GetFileTime_Patched(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime)
{
	auto it = handle_map.find(hFile);
	if (it != handle_map.end())
	{
		//DPRINTF("Forwarding GetFileTime %p to vfile\n", hFile);
		return it->second->GetFileTime(lpCreationTime, lpLastAccessTime, lpLastWriteTime);
	}
	
	//DPRINTF("GetFileTime %p\n", hFile);
	return GetFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
}

/*static DWORD GetFileAttributesW_Patched(LPCWSTR lpFileName)
{
	if (lpFileName)
		DPRINTF("GetFileAttributesW %S\n", lpFileName);
	
	return GetFileAttributesW(lpFileName);
}

static DWORD GetFileType_Patched(HANDLE hFile)
{
	DPRINTF("GetFileType %p\n", hFile);
	return GetFileType(hFile);
}*/

static void patch_funcs()
{
	if (!log_internal)
	{
		if (!PatchUtils::HookImport("KERNEL32.DLL", "CreateFileW", (void *)CreateFileW_Patched))
		{
			UPRINTF("Failed to hook CreateFileW");
			exit(-1);
		}
		
		if (!PatchUtils::HookImport("KERNEL32.DLL", "CloseHandle", (void *)CloseHandle_Patched))
		{
			UPRINTF("Failed to hook CloseHandle");
			exit(-1);
		}
		
		if (!PatchUtils::HookImport("KERNEL32.DLL", "ReadFile", (void *)ReadFile_Patched))
		{
			UPRINTF("Failed to hook ReadFile");
			exit(-1);
		}
	}
	else
	{
		if (!PatchUtils::HookImport("KERNEL32.DLL", "CreateFileW", (void *)CreateFileW_Patched_Log))
		{
			UPRINTF("Failed to hook CreateFileW");
			exit(-1);
		}
		
		if (!PatchUtils::HookImport("KERNEL32.DLL", "CloseHandle", (void *)CloseHandle_Patched_Log))
		{
			UPRINTF("Failed to hook CloseHandle");
			exit(-1);
		}
		
		if (!PatchUtils::HookImport("KERNEL32.DLL", "ReadFile", (void *)ReadFile_Patched_Log))
		{
			UPRINTF("Failed to hook ReadFile");
			exit(-1);
		}
	}
	
	/*if (!PatchUtils::HookImport("KERNEL32.DLL", "SetFilePointerEx", (void *)SetFilePointerEx_Patched))
	{
		UPRINTF("Failed to hook SetFilePointerEx");
		exit(-1);
	}*/
		
	if (!PatchUtils::HookImport("KERNEL32.DLL", "GetFileSizeEx", (void *)GetFileSizeEx_Patched))
	{
		UPRINTF("Failed to hook GetFileSizeEx");
		exit(-1);
	}
	
	/*if (!PatchUtils::HookImport("KERNEL32.DLL", "FindFirstFileW", (void *)FindFirstFileW_Patched))
	{
		UPRINTF("Failed to hook FindFirstFileW");
		exit(-1);
	}
	
	if (!PatchUtils::HookImport("KERNEL32.DLL", "FindFirstFileExA", (void *)FindFirstFileExA_Patched))
	{
		UPRINTF("Failed to hook FindFirstFileExA");
		exit(-1);
	}
	
	if (!PatchUtils::HookImport("KERNEL32.DLL", "FindFirstFileExW", (void *)FindFirstFileExW_Patched))
	{
		UPRINTF("Failed to hook FindFirstFileExW");
		exit(-1);
	}*/
	
	if (!PatchUtils::HookImport("KERNEL32.DLL", "GetFileTime", (void *)GetFileTime_Patched))
	{
		UPRINTF("Failed to hook GetFileTime");
		exit(-1);
	}
	
	/*if (!PatchUtils::HookImport("KERNEL32.DLL", "GetFileAttributesW", (void *)GetFileAttributesW_Patched))
	{
		UPRINTF("Failed to hook GetFileAttributesW");
		exit(-1);
	}
	
	if (!PatchUtils::HookImport("KERNEL32.DLL", "GetFileType", (void *)GetFileType_Patched))
	{
		UPRINTF("Failed to hook GetFileType");
		exit(-1);
	}*/
}

BOOL RdbVirtualFile::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	if (nNumberOfBytesToRead == 0)
	{
		if (lpNumberOfBytesRead)
			*lpNumberOfBytesRead = 0;
		
		return TRUE;
	}
	
	if (!lpBuffer)
	{
		//DPRINTF("Warning: lpBuffer NULL\n");
		return FALSE;
	}
	
	
	DWORD total_read = 0;
	
	if (lpOverlapped)
	{
		file_pointer = lpOverlapped->Offset;
		file_pointer |= ((uint64_t)lpOverlapped->OffsetHigh << 32);
		//DPRINTF("Event %p\n", lpOverlapped->hEvent);
		
		if (lpOverlapped->hEvent)
		{
			DPRINTF("-----------WARNING: overlapped hEvent not implemented\n");
		}
	}
	
	if (file_pointer < mem_size)
	{
		DWORD max_read, n_read;
		
		max_read = mem_size - file_pointer;
		n_read = (nNumberOfBytesToRead > max_read) ? max_read : nNumberOfBytesToRead;
		
		//DPRINTF("File pointer %Ix. Max read = %x Mem Size = %Ix\n", file_pointer, (unsigned int)max_read, mem_size);
		
		memcpy(lpBuffer, mem+file_pointer, n_read);
		nNumberOfBytesToRead -= n_read;
		total_read += n_read;
		file_pointer += n_read;
		lpBuffer = ((uint8_t *)lpBuffer + n_read);
	}
	
	if (nNumberOfBytesToRead == 0 || real_file == INVALID_HANDLE_VALUE)
	{
		if (lpNumberOfBytesRead)
			*lpNumberOfBytesRead = total_read;
		
		return TRUE;
	}
	
	uint64_t real_file_pointer = file_pointer - mem_size + file_start;
	LARGE_INTEGER fp;
	
	fp.QuadPart = real_file_pointer;	
	if (!SetFilePointerEx(real_file, fp, nullptr, FILE_BEGIN))
		return FALSE;
	
	if (nNumberOfBytesToRead + real_file_pointer > file_end)
	{
		nNumberOfBytesToRead = file_end - real_file_pointer;
	}
	
	DWORD file_read;
	if (!::ReadFile(real_file, lpBuffer, nNumberOfBytesToRead, &file_read, nullptr))
		return FALSE;
	
	total_read += file_read;
	file_pointer += file_read;
	
	if (lpNumberOfBytesRead)
	{
		*lpNumberOfBytesRead = total_read;
	}
	
	return TRUE;
}

static bool files_listing_visitor(const std::string &path, bool, void *custom)
{
	std::vector<std::string> *plist = (std::vector<std::string> *)custom;	
	plist->push_back(path);
	
	return true;
}

static void prepare_rdb(const std::string &rdb_path, const std::string &virt_path, RdbFile **prdb, std::unordered_map<std::string, std::vector<std::string>> *pl2_files=nullptr, 
						const std::string &include_dir=".", std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*> *l2_entries=nullptr)
{
	std::vector<std::string> file_list;
	std::vector<RDBEntry *> entries;
	bool log = (log_external || log_internal);
	
	Utils::VisitDirectory(virt_path, true, false, false, files_listing_visitor, &file_list);
	
	*prdb = nullptr;
	
	if (file_list.size() == 0 && !log && !pl2_files)
		return;
	
	size_t file_size;
	uint8_t *file_buf = Utils::ReadFile(rdb_path, &file_size);
	if (!file_buf)
		return;
	
	RdbFile *rdb = new RdbFile(rdb_path);
	if (!rdb->LoadAndBuildMap(file_buf, file_size, &entries))
	{
		delete rdb;
		return;
	}
	
	rdb->BuildAdditionalLookup(true, false);
	
	size_t num_files = 0;
	
	for (const std::string &file : file_list)
	{
		std::string name = Utils::GetFileNameString(file);
		uint32_t hash;
		size_t idx = (size_t)-1;
		
		if (get_hex_name(name, &hash))
		{
			idx = rdb->FindFileByID(hash);
		}
		else
		{
			idx = rdb->FindFileByName(name);
			
			// some work around for double bug extension caused by rdbtool bug...
			if (idx == (size_t)-1 && Utils::EndsWith(name, ".rigbin.rigbin", false))
			{
				idx = rdb->FindFileByName(name.substr(0, name.length()-7));
			}
		}
		
		if (idx == (size_t)-1)
		{
			DPRINTF("Notice: file \"%s\" doesn't exist in this .rdb (%s)\n", name.c_str(), Utils::GetFileNameString(rdb_path).c_str());
			continue;
		}
		
		VirtualRdbEntry ventry;		
		RDBEntry *entry1 = entries[idx];
		
		ventry.original_flags = entry1->flags;
		ventry.entry1 = entry1;
		entry1->flags = RDB_FLAG_EXTERNAL;
		entry1->file_size = Utils::GetFileSize(file);
		
		ventry.entry2.resize(entry1->entry_size - entry1->c_size);
		memcpy(ventry.entry2.data(), entry1, ventry.entry2.size());
		
		RDBEntry *entry2 = (RDBEntry *)ventry.entry2.data();
		entry2->c_size = entry2->file_size;
		entry2->entry_size = entry2->file_size + (uint32_t)ventry.entry2.size();
		
		ventry.rdb = rdb;
		ventry.real_path = file;
		
		std::string virtual_path = Utils::ToLowerCase(data_dir + Utils::UnsignedToHexString(entry1->file_id, true) + ".file");
		virtual_files[virtual_path] = ventry;
		
		DPRINTF("File \"%s\" added to virtualization.\n", file.c_str());	
		
		num_files++;
	}
	
	if (log)
	{
		size_t num_files = rdb->GetNumFiles();
		std::string rdb_lc = Utils::ToLowerCase(rdb_path);
		
		for (size_t i = 0; i < num_files; i++)
		{
			const RdbEntry &entry = rdb->GetEntry(i);
			
			if (log_external && entry.external)
			{
				std::string name;
				
				if (rdb->GetFileName(i, name))
				{
					log_external_files[entry.file_id] = name;
				}
			}
			else if (log_internal && !entry.external)
			{
				std::string bin_file = rdb_lc + entry.bin_file;
				RdbDebugInternal *dint;
				std::string name;
				
				auto it = log_internal_files.find(bin_file);
				if (it == log_internal_files.end())
				{
					dint = new RdbDebugInternal();
					dint->rdb = rdb;	
					log_internal_files[bin_file] = dint;
				}
				else
				{
					dint = it->second;
				}
				
				if (rdb->GetFileName(i, name))
				{
					dint->offsets_map[entry.offset] = name;
				}
			}
		}
	}
	
	if (pl2_files)
	{	
		for (auto &it : *pl2_files)
		{
			std::unordered_map<uint32_t, VirtualRdbEntry> *v_entries = new std::unordered_map<uint32_t, VirtualRdbEntry>();
			std::string this_dir = l2_dir + it.first + "/" + include_dir + "/";
			
			for (const std::string &l2_file : it.second)
			{
				if (!Utils::BeginsWith(l2_file, this_dir, false))
					continue;
				
				std::string name = l2_file.substr(this_dir.length());
				uint32_t hash;
				size_t idx = (size_t)-1;
				
				if (get_hex_name(name, &hash))
				{
					idx = rdb->FindFileByID(hash);
				}
				else
				{
					idx = rdb->FindFileByName(name);
					
					// some work around for double bug extension caused by rdbtool bug...
					if (idx == (size_t)-1 && Utils::EndsWith(name, ".rigbin.rigbin", false))
					{
						idx = rdb->FindFileByName(name.substr(0, name.length()-7));
					}
				}
				
				if (idx == (size_t)-1)
				{
					DPRINTF("Notice: file \"%s\" doesn't exist in this .rdb (%s) (in Layer2 mod %s)\n", name.c_str(), Utils::GetFileNameString(rdb_path).c_str(), it.first.c_str());
					continue;
				}
				
				RDBEntry *entry1 = entries[idx];
				const RdbEntry &entry1_h = rdb->GetEntry(idx);
				std::string virtual_path = Utils::ToLowerCase(data_dir + Utils::UnsignedToHexString(entry1->file_id, true) + ".file");
				
				auto it = virtual_files.find(virtual_path);
				if (it == virtual_files.end())
				{
					VirtualRdbEntry ventry;	
					
					ventry.original_flags = entry1->flags;
					ventry.entry1 = entry1;
					entry1->flags |= RDB_FLAG_EXTERNAL;
					
					ventry.rdb = rdb;
					ventry.real_path = rdb_path + entry1_h.bin_file;
					ventry.region_start = entry1_h.offset;
					ventry.region_size = entry1_h.size;				
					
					/* Fix for external files */
					if (ventry.original_flags & RDB_FLAG_EXTERNAL)
					{						
						ventry.real_path = data_dir + Utils::UnsignedToHexString(entry1->file_id, true) + ".file";
						ventry.region_start = ventry.region_size = 0xDEADC0DE;
						//DPRINTF("Layer2 external file %s 0x%08x 0x%08x\n", ventry.real_path.c_str(), (uint32_t)ventry.region_start, (uint32_t)ventry.region_size);
					}
					/* */
					
					virtual_files[virtual_path] = ventry;
				}
				else
				{
					// In this case, the file is already being replaced in "Layer 1".
				}
				
				VirtualRdbEntry ventry;	
				
				ventry.entry1 = entry1;
				entry1->flags = RDB_FLAG_EXTERNAL;
				entry1->file_size = Utils::GetFileSize(l2_file);
				
				ventry.entry2.resize(entry1->entry_size - entry1->c_size);
				memcpy(ventry.entry2.data(), entry1, ventry.entry2.size());
				
				RDBEntry *entry2 = (RDBEntry *)ventry.entry2.data();
				entry2->c_size = entry2->file_size;
				entry2->entry_size = entry2->file_size + (uint32_t)ventry.entry2.size();
				
				ventry.rdb = rdb;
				ventry.real_path = l2_file;
				
				(*v_entries)[entry1->file_id] = ventry;
			}
			
			(*l2_entries)[it.first] = v_entries;
		}
	} // if pl2_files
	
	// Test, remove me
	/*
	if (rdb_path.find("CharacterEditor") != std::string::npos)
	{
		size_t idx = rdb->FindFileByID(0x3A144F2E);
		assert(idx != (size_t)-1);
		
		VirtualRdbEntry ventry;
		
		RDBEntry *entry1 = entries[idx];
		const RdbEntry &entry1_h = rdb->GetEntry(idx);
		
		ventry.entry1 = entry1;
		entry1->flags = RDB_FLAG_EXTERNAL | RDB_FLAG_COMPRESSED;
				
		ventry.rdb = rdb;
		ventry.real_path = "S:/SteamLibrary/steamapps/common/Dead or Alive 6/CharacterEditor.rdb.bin";
		ventry.region_start = entry1_h.offset;
		ventry.region_size = entry1_h.size;
		
		std::string virtual_path = Utils::ToLowerCase(data_dir + Utils::UnsignedToHexString(entry1->file_id, true) + ".file");
		virtual_files[virtual_path] = ventry;
	}*/
	
	if (num_files == 0 && !pl2_files)
	{
		if (!log)
			delete rdb;		
		
		return;
	}
	
	*prdb = rdb;
	
	RdbVirtualFile *vfile = new RdbVirtualFile(file_buf, file_size);
	vfile->rdb = nullptr; // this is a rdb, but not a file inside a rdb
	
	memory_rdb_files[Utils::ToLowerCase(Utils::NormalizePath(rdb_path))] = vfile;
}

/*static bool l2_visitor(const std::string &path, bool, void *param)
{
	//DPRINTF("***%s\n", path.c_str());
	
	if (!Utils::BeginsWith(path, l2_dir, false))
		return true;
	
	std::string rel = Utils::NormalizePath(path.substr(l2_dir.length()));
	std::vector<std::string> components;
	
	Utils::GetMultipleStrings(rel, components, '/');	
	std::unordered_map<std::string, std::vector<std::string>> *pl2_files = (std::unordered_map<std::string, std::vector<std::string>> *)param;
	
	//DPRINTF("%s, components %Id\n", path.c_str(), components.size());
	
	if (components.size() == 2)
	{
		auto it = pl2_files->find(components[0]);
		if (it == pl2_files->end())
		{
			std::vector<std::string> empty;
			(*pl2_files)[components[0]] = empty; 
			//DPRINTF("+++%s - %s\n", components[0].c_str(), path.c_str());
		}
		
		return true;
	}
	
	if (components.size() != 3)
		return true;
	
	components[1] = Utils::ToLowerCase(components[1]);
	if (components[1] != "material" && components[1] != "character" && components[1] != "kids" && components[1] != "field4")
		return true;	
	
	auto it = pl2_files->find(components[0]);
	if (it == pl2_files->end())
	{
		std::vector<std::string> files = { Utils::NormalizePath(path) };
		(*pl2_files)[components[0]] = files;
		//DPRINTF("---%s\n", components[0].c_str());
	}
	else
	{
		it->second.push_back(Utils::NormalizePath(path));
	}
	
	return true;
}*/

static bool l2_visitor(const std::string &path, bool, void *param)
{
	//DPRINTF("***%s\n", path.c_str());
	
	if (!Utils::BeginsWith(path, l2_dir, false))
		return true;
	
	std::string rel = Utils::NormalizePath(path.substr(l2_dir.length()));
	std::vector<std::string> components;
	
	Utils::GetMultipleStrings(rel, components, '/');	
	std::unordered_map<std::string, std::vector<std::string>> *pl2_files = (std::unordered_map<std::string, std::vector<std::string>> *)param;
	
	//DPRINTF("%s, components %Id\n", path.c_str(), components.size());
	
	if (components.size() == 2)
	{
		auto it = pl2_files->find(components[0]);
		if (it == pl2_files->end() && Utils::ToLowerCase(Utils::GetFileNameString(rel)) == "mod.ini")
		{
			std::vector<std::string> empty;
			(*pl2_files)[components[0]] = empty; 
			//UPRINTF("+++%s - %s\n", components[0].c_str(), path.c_str());
		}
		
		return true;
	}
	
	std::string mn, vf;
	
	if (components.size() == 3)
	{
		if (Utils::ToLowerCase(Utils::GetFileNameString(rel)) == "mod.ini")
		{
			mn = components[0] + "/" + components[1];
			
			auto it = pl2_files->find(mn);
			if (it == pl2_files->end())
			{
				std::vector<std::string> empty;
				(*pl2_files)[mn] = empty; 
			}
			
			return true;
		}
		else
		{
			mn = components[0];
			vf = Utils::ToLowerCase(components[1]);
		}
	}
	else if (components.size() == 4)
	{
		mn = components[0] + "/" + components[1];
		vf = Utils::ToLowerCase(components[2]);
	}
	else
	{
		return true;
	}	
	
	if (vf != "material" && vf != "character" && vf != "kids" && vf != "field4" && vf != "rrp")
		return true;		
		
	auto it = pl2_files->find(mn);
	if (it == pl2_files->end())
	{
		std::vector<std::string> files = { Utils::NormalizePath(path) };
		(*pl2_files)[mn] = files;
		//DPRINTF("---%s\n", components[0].c_str());
	}
	else
	{
		it->second.push_back(Utils::NormalizePath(path));
	}
	
	return true;
}

void prepare_layer2(std::unordered_map<std::string, std::vector<std::string>> &l2_files)
{	
	layer2_init();
	Utils::VisitDirectory(l2_dir, true, false, true, l2_visitor, (void *)&l2_files, true, true);
	
	for (auto it = l2_files.begin(); it != l2_files.end(); )
	{
		std::string cos_ini = l2_dir + it->first + "/mod.ini";
		
		if(!Utils::FileExists(cos_ini))
		{
			DPRINTF("Warning: file \"%s\" doesn't exist, so this mod folder won't be loaded.\n", cos_ini.c_str());	
			it = l2_files.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void load_layer2_mods(std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*> *c_entries, std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*> *m_entries,
					  std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*> *k_entries, std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*> *f_entries,
					  std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*> *r_entries)
{
	if (c_entries->size() != m_entries->size())
	{
		FTPRINTF(true, "Load layer2 mods, internal error (not same number in map)\n");
		return;
	}
	
	for (auto &it : *c_entries)
	{
		auto it2 = m_entries->find(it.first);
		if (it2 == m_entries->end())
		{
			FTPRINTF(true, "Load layer2 mods, internal error, not found mat %s\n", it.first.c_str());
			return;
		}
		
		auto it3 = k_entries->find(it.first);
		if (it3 == k_entries->end())
		{
			FTPRINTF(true, "Load layer2 mods, internal error, not found kids %s\n", it.first.c_str());
			return;
		}
		
		auto it4 = f_entries->find(it.first);
		if (it4 == f_entries->end())
		{
			FTPRINTF(true, "Load layer2 mods, internal error, not found field4 %s\n", it.first.c_str());
			return;
		}
		
		auto it5 = r_entries->find(it.first);
		if (it5 == r_entries->end())
		{
			FTPRINTF(true, "Load layer2 mods, internal error, not found rrp %s\n", it.first.c_str());
			return;
		}
		
		layer2_add_mod(it.first, l2_dir + it.first + "/mod.ini", it.second, it2->second, it3->second, it4->second, it5->second);
	}
	
	layer2_finalization();
}

static void prepare_virt()
{
	std::unordered_map<std::string, std::vector<std::string>> l2_files;
	
	data_dir = myself_path + "data/";
	rdb_dir = myself_path + "REDELBE/";
	l2_dir = rdb_dir + "Layer2/";	
	
	prepare_layer2(l2_files);
	
	if (l2_files.size() == 0)
	{		
		prepare_rdb(myself_path + "CharacterEditor.rdb", rdb_dir + "CharacterEditor", &CharacterEditor_rdb);
		prepare_rdb(myself_path + "MaterialEditor.rdb", rdb_dir + "MaterialEditor", &MaterialEditor_rdb);
		prepare_rdb(myself_path + "KIDSSystemResource.rdb", rdb_dir + "KIDSSystemResource", &KIDSSystemResource_rdb);
		prepare_rdb(myself_path + "FieldEditor4.rdb", rdb_dir + "FieldEditor4", &FieldEditor4_rdb);
		prepare_rdb(myself_path + "RRPreview.rdb", rdb_dir + "RRPreview", &RRPreview_rdb);
	}
	else
	{
		std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*> *c_entries, *m_entries, *k_entries, *f_entries, *r_entries;
		
		c_entries = new std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*>();
		m_entries = new std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*>();
		k_entries = new std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*>();
		f_entries = new std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*>();
		r_entries = new std::unordered_map<std::string, std::unordered_map<uint32_t, VirtualRdbEntry>*>();
		
		prepare_rdb(myself_path + "CharacterEditor.rdb", rdb_dir + "CharacterEditor", &CharacterEditor_rdb, &l2_files, "Character", c_entries);
		prepare_rdb(myself_path + "MaterialEditor.rdb", rdb_dir + "MaterialEditor", &MaterialEditor_rdb, &l2_files,  "Material", m_entries);
		prepare_rdb(myself_path + "KIDSSystemResource.rdb", rdb_dir + "KIDSSystemResource", &KIDSSystemResource_rdb, &l2_files, "KIDS", k_entries);
		prepare_rdb(myself_path + "FieldEditor4.rdb", rdb_dir + "FieldEditor4", &FieldEditor4_rdb, &l2_files, "Field4", f_entries);
		prepare_rdb(myself_path + "RRPreview.rdb", rdb_dir + "RRPreview", &RRPreview_rdb, &l2_files, "RRP", r_entries);
		
		load_layer2_mods(c_entries, m_entries, k_entries, f_entries, r_entries);
	}
	
	prepare_rdb(myself_path + "EventMaker3.rdb", rdb_dir + "EventMaker3", &EventMaker3_rdb);	
	prepare_rdb(myself_path + "ScreenLayout.rdb", rdb_dir + "ScreenLayout", &ScreenLayout_rdb);
	prepare_rdb(myself_path + "system.rdb", rdb_dir + "system", &system_rdb);
}

static void load_config()
{
	ini.GetBooleanValue("Debug", "log_virtual", &log_virtual, false);
	ini.GetBooleanValue("Debug", "log_external", &log_external, false);
	ini.GetBooleanValue("Debug", "log_internal", &log_internal, false);
	ini.GetBooleanValue("Debug", "log_numeric", &log_numeric, false);
	
	//DPRINTF("%d %d %d\n", log_virtual, log_external, log_internal);
}

void rdbemu_init()
{
	load_config();
	prepare_virt();
	patch_funcs();
}

void rdbemu_set_layer2(Layer2Mod *mod, int index)
{	
	/*if (mod)
		DPRINTF("Set layer2 mod %s %d\n", mod->name.c_str(), index);
	else
		DPRINTF("Unset layer 2 mod %d\n", index);*/
	
	l2_mods[(index*2)] = mod;
}

void rdbemu_set_layer2_hair(Layer2Mod *mod, int index)
{
	/*if (mod)
		DPRINTF("Set layer2 hair mod %s %d\n", mod->name.c_str(), index);
	else
		DPRINTF("Unset layer 2 hair mod %d\n", index);*/
	
	
	l2_mods[(index*2)+1] = mod;
}

void rdbemu_set_layer2_stage(Layer2Mod *mod)
{
	l2_mods[4] = mod;
}

void rdbemu_set_layer2_mode(int index)
{
	switch (index)
	{
		case 0:			
			l2_indexes = { 1, 0, -1, -1, 4 }; // Hair player 1, Costume player 1, stage
		break;
		
		case 1:
			l2_indexes = { 3, 2, -1, -1, 4 }; // Hair player 2, Costume player 2, stage
		break;
		
		default:
			l2_indexes = { 1, 0, 3, 2, 4 }; // Hair player 1, Costume player 1, Hair player 2, Costume player 2, stage
	}
}

