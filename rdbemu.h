#ifndef RDBEMU_H
#define RDBEMU_H

#include <windows.h>
#include "DOA6/RdbFile.h"
#include "Utils.h"
#include "layer2.h"

struct Layer2Mod;

void rdbemu_init();
void rdbemu_set_layer2(Layer2Mod *mod, int index);
void rdbemu_set_layer2_hair(Layer2Mod *mod, int index);
void rdbemu_set_layer2_stage(Layer2Mod *mod);
void rdbemu_set_layer2_mode(int index);

struct VirtualRdbEntry
{
	RDBEntry *entry1;
	std::vector<uint8_t> entry2;
	RdbFile *rdb;
	std::string real_path;
	uint32_t original_flags;
	
	uint64_t region_start;
	uint64_t region_size;
	
	VirtualRdbEntry()
	{
		entry1 = nullptr;
		rdb = nullptr;
		region_start = region_size = 0;
		original_flags = 0;
	}
};

class RdbVirtualFile
{
private:

	uint8_t *mem;	
	uint64_t mem_size;
	
	HANDLE real_file;
	uint64_t file_start;
	uint64_t file_size;
	uint64_t file_end;
	
	uint64_t file_pointer;
	

public:	

////// User data
	RdbFile *rdb;
/////

	RdbVirtualFile()
	{
		mem = nullptr;
		mem_size = file_size = file_start = file_end = file_pointer = 0;
		real_file = INVALID_HANDLE_VALUE;
	}
	
	RdbVirtualFile(uint8_t *mem, uint64_t mem_size) : mem(mem), mem_size(mem_size)
	{
		real_file = INVALID_HANDLE_VALUE;
		file_size = file_start = file_end = file_pointer = 0;
	}
	
	RdbVirtualFile(uint8_t *mem, uint64_t mem_size, HANDLE real_file) : mem(mem), mem_size(mem_size), real_file(real_file)
	{
		LARGE_INTEGER fileSize;
		
		if (GetFileSizeEx(real_file, &fileSize))
		{
			file_size = file_end = fileSize.QuadPart;
		}
		else
		{
			file_size = file_end = 0;
		}
		
		file_start = file_pointer = 0;
	}
	
	RdbVirtualFile(uint8_t *mem, uint64_t mem_size, HANDLE real_file, uint64_t region_file_start, uint64_t region_file_size) : mem(mem), mem_size(mem_size), real_file(real_file)
	{
		file_start = region_file_start;
		file_size = region_file_size;
		file_end = file_start + file_size;
		file_pointer = 0;
	}
	
	~RdbVirtualFile()
	{
		if (real_file != INVALID_HANDLE_VALUE)
			CloseHandle(real_file);
	}
	
	BOOL Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);	
	inline uint64_t GetSize() const { return mem_size + file_size; }
	
	inline BOOL GetFileTime(LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) 
	{
		if (real_file != INVALID_HANDLE_VALUE)
			return ::GetFileTime(real_file, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
		
		FILETIME tm;
		GetSystemTimeAsFileTime(&tm);
		
		if (lpCreationTime)
			*lpCreationTime = tm;
		
		if (lpLastAccessTime)
			*lpLastAccessTime = tm;
		
		if (lpLastWriteTime)
			*lpLastWriteTime = tm;
		
		return true;
	}
	
	inline void SetFilePointer(uint64_t new_pointer) { file_pointer = new_pointer; }
};

struct RdbDebugInternal
{
	RdbFile *rdb;
	std::unordered_map<uint64_t, std::string> offsets_map;
};

#endif // RDBEMU_H
