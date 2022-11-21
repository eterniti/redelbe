#include "redelbe.h"
#include "debug.h"

typedef size_t (* CBC_Decryption_ProcessData_Type)(void *, uint8_t *, const uint8_t *, size_t);
static CBC_Decryption_ProcessData_Type CBC_Decryption_ProcessData;

static bool uncompress;

extern "C"
{

PUBLIC void SetupDumpDecrypted(CBC_Decryption_ProcessData_Type orig)
{
	CBC_Decryption_ProcessData = orig;
	
	ini.GetBooleanValue("Debug", "auto_uncompress_decrypted", &uncompress, false);
}

PUBLIC size_t CBC_Decryption_ProcessData_Patched(void *pthis, uint8_t *outString, const uint8_t *inString, size_t length)
{
	static int file_counter = 0;
	
	size_t ret = CBC_Decryption_ProcessData(pthis, outString, inString, length);
	
	/*DPRINTF("Called from %p\n", BRA(0));
	DPRINTF("%s\n", Utils::BinaryString(inString, 0x10, false).c_str());
	DPRINTF("%s\n", Utils::BinaryString(inString+0x10, 0x10, false).c_str());*/
	
	std::string file_name = "REDELBE/DECRYPTED/" + Utils::ToString(file_counter) + ".bin";
	
	if (uncompress && outString[0] == 0x78 && outString[1] == 0x9C)
	{
		uint32_t uncomp_size = length*4;
		uint8_t *uncomp = new uint8_t[uncomp_size];
		
		if (Utils::UncompressZlib(uncomp, &uncomp_size, outString, length))
		{
			Utils::WriteFileBool(file_name, uncomp, uncomp_size, true, true);
			delete[] uncomp;
			
			file_counter++;
			return ret;
		}
		else
		{
			DPRINTF("Warning: failed to uncompress %d. It will be dumped compressed instead.\n", file_counter);
			delete[] uncomp;
		}
	}	
	
	// Dump uncompressed
	Utils::WriteFileBool(file_name, outString, length, true, true);
	file_counter++;
	
	return ret;
}

} // extern "C"
