#include "redelbe.h"
#include "game.h"
#include "debug.h"

typedef bool (* SGPType)(void *, ScriptValue *);
static SGPType script_get_params_impl; 

static void **ptask_detail_manager_singleton;

static const std::vector<std::string> char_prefixes =
{
	"CMN",
	"ZAC",
	"TIN",
	"JAN",
	"RYU",
	"KAS",
	"HEL",
	"BAS",
	"KOK",
	"HYT",
	"LEI", // 10
	"AYA",
	"ELI", 
	"LIS",
	"BRA",
	"CRI",
	"HTM",
	"BAY",
	"RIG",
	"MIL",
	"MAR", // 20
	"NYO",
	"HON",
	"RID",
	"DGO",
	"NIC",
	"ARD", 
	"SRD", 
	"PHF", 
	"MPP", 
	"MAI", // 30
	"SNK",
	"MOM",
	"RAC",
	"SKD",
};

static const std::vector<std::string> pchar_prefixes =
{
	"ZAC",
	"TIN",
	"JAN",
	"RYU",
	"KAS",
	"HEL",
	"BAS",
	"KOK",
	"HYT",
	"LEI", 
	"AYA",
	"ELI", 
	"LIS",
	"BRA",
	"CRI",
	"HTM",
	"BAY",
	"RIG",
	"MIL",
	"MAR", 
	"NYO",
	"HON",
	"RID",
	"DGO",
	"NIC",
	"PHF", 
	"MAI", 
	"SNK",
	"MOM",
	"RAC",
	"SKD",
};

// The crappy hash func used by the game
static uint32_t hash_func(const char *a1, int a2, int a3)
{
  int v3; 

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
  return (uint32_t)a2;
}

uint32_t hash_func(const char *a1)
{
    return hash_func(a1+1, a1[0]*31, 31);
}

std::string character_id_to_prefix(uint8_t id)
{
	if (id < char_prefixes.size())
		return char_prefixes[id];
	
	return "ERR";
}

uint8_t character_prefix_to_id(const std::string &prefix)
{
	for (uint8_t i = 0; i < (uint8_t)char_prefixes.size(); i++)
	{
		if (prefix == char_prefixes[i])
			return i;
	}
	
	return CHAR_RANDOM;
}

uint8_t character_hash_to_id(uint32_t hash)
{
	for (uint8_t i = 0; i < (uint8_t)char_prefixes.size(); i++)
	{
		if (hash == hash_func(char_prefixes[i]))
			return i;
	}
	
	return CHAR_RANDOM;
}

std::string pchar_to_prefix(uint8_t id)
{
	if (id < pchar_prefixes.size())
		return pchar_prefixes[id];
	
	return "ERR";
}

uint32_t get_current_stage()
{
	if (!ptask_detail_manager_singleton || !(*ptask_detail_manager_singleton))
		return 0;
	
	uintptr_t *ptr = (uintptr_t *)*ptask_detail_manager_singleton;
	uint32_t *ptr2 = (uint32_t *)ptr[0x708/8];
	
	if (!ptr2)
		return 0;
	
	return *ptr2;
}

uint16_t script_get_num_params(void *script_object)
{
	uintptr_t **so = (uintptr_t **)script_object;
	uint16_t *p16 = (uint16_t *) (so[1])[1];
	
	return p16[0xA6/2];
}

bool script_get_params(void *script, ScriptValue *params)
{
	if (!script_get_params_impl)
		return false;
	
	void *obj = ((void **)script)[1];
	uintptr_t *premaining = ((uintptr_t **)obj)[0x20/8];
	uintptr_t remaining = premaining ? *premaining : 0;
	
	bool ret = script_get_params_impl(obj, params);
	
	if (premaining)
		*premaining = remaining;
	
	return ret;
}

extern "C"
{
	
PUBLIC void OnSGPLocated(SGPType orig)
{
	script_get_params_impl = orig;
}

PUBLIC void OnTDMLocated(void *buf)
{
	ptask_detail_manager_singleton = (void **)GetAddrFromRel(buf);
}
	
} // extern "C"