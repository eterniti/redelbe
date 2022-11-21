#ifndef ____GAME_H_
#define ____GAME_H_

#include "Basefile.h"

// Put all game related shit here

#define NUM_PLAYERS	2

// Matches id used by game
enum 
{
	CHAR_CMN,
	CHAR_ZAC,
	CHAR_TIN,
	CHAR_JAN,
	CHAR_RYU,
	CHAR_KAS,
	CHAR_HEL,
	CHAR_BAS,
	CHAR_KOK,
	CHAR_HYT,
	CHAR_LEI, // 10
	CHAR_AYA,
	CHAR_ELI, 
	CHAR_LIS,
	CHAR_BRA,
	CHAR_CRI,
	CHAR_HTM,
	CHAR_BAY,
	CHAR_RIG,
	CHAR_MIL,
	CHAR_MAR, // 20
	CHAR_NYO,
	CHAR_HON,
	CHAR_RID,
	CHAR_DGO,
	CHAR_NIC,
	CHAR_ARD, // 26, Story mode only, can't be used in all contexts
	CHAR_SRD, // 27, Story mode only, can't be used in all contexts
	CHAR_PHF, // 28 
	CHAR_MPP, // 29, Story mode only, can't be used in all contexts
	CHAR_MAI, // 30
	CHAR_SNK, // 31
	CHAR_MOM, // 32
	CHAR_RAC, // 33
	CHAR_SKD, // 34
	CHAR_NUM, // Convenience to store characters num
	CHAR_RANDOM = 0xFF
};

// This is used in some other parts in game, it only has playable chars
enum 
{
	PCHAR_ZAC,
	PCHAR_TIN,
	PCHAR_JAN,
	PCHAR_RYU,
	PCHAR_KAS,
	PCHAR_HEL,
	PCHAR_BAS,
	PCHAR_KOK,
	PCHAR_HYT,
	PCHAR_LEI,
	PCHAR_AYA, //10
	PCHAR_ELI, 
	PCHAR_LIS,
	PCHAR_BRA,
	PCHAR_CRI,
	PCHAR_HTM,
	PCHAR_BAY,
	PCHAR_RIG,
	PCHAR_MIL,
	PCHAR_MAR, 
	PCHAR_NYO, // 20
	PCHAR_HON,
	PCHAR_RID,
	PCHAR_DGO,
	PCHAR_NIC,	
	PCHAR_PHF, 	
	PCHAR_MAI,
	PCHAR_SNK,
	PCHAR_MOM, 
	PCHAR_RAC, 
	PCHAR_SKD, // 30
	PCHAR_NUM, // Convenience to store characters num
};

enum
{
	SCRIPT_DT_INT32 = 1,
	SCRIPT_DT_FLOAT = 2,
	SCRIPT_DT_BOOL = 3,
	SCRIPT_DT_STRING = 4,
};

// Structure used by the game for slot data
struct GameCharSlot
{
	uint8_t char_id; // Numeric id as above. 
	uint8_t pad_01[3];
	uint32_t costume_hash; // 4
	uint32_t face_hash; // 8
	uint32_t hair_hash; // 0x0C;
	uint32_t color; // 0x10
	uint32_t glasses_hash; // 0x14
	uint32_t underwear; // 0x18
	uint32_t hair_color; // 0x1C - Was added in game version 1.19, before being implemented in 1.20
};
CHECK_STRUCT_SIZE(GameCharSlot, 0x20);

struct PACKED GameCharSlot2
{
	uint32_t costume_hash; // 0
	uint32_t face_hash; // 4
	uint32_t hair_hash; // 8
	uint32_t color; // 0xC
	uint32_t glasses_hash; // 0x10
	uint32_t underwear; // 0x14
	uint32_t hair_color; // 0x18 - Was added in game version 1.19, before being implemented in 1.20
};
CHECK_STRUCT_SIZE(GameCharSlot2, 0x1C);

struct PACKED StageDetail
{
	int32_t param0; // 0
	int32_t param1; // 4
	uint32_t param2; // 8
	uint32_t param3; // 0xC
	uint32_t param4; // 0x10
	uint32_t param5; // 0x14
	bool no_mob_flag; // 0x18
	uint8_t pad_19[3];
};
CHECK_STRUCT_SIZE(StageDetail, 0x1C);

struct ScriptValue
{
	int type;
	
	union
	{
		uint32_t val32;
		uint64_t val64;
		float val_float;
		bool val_bool;
		const char **val_str;
	} value;
};
CHECK_STRUCT_SIZE(ScriptValue, 0x10);

struct CostumeLinkedList
{
	CostumeLinkedList *next; // 0
	CostumeLinkedList *next2; // 8
	CostumeLinkedList *next3; // 10
	uint8_t unk_18;
	uint8_t unk_19;
	uint8_t unk_1A;
	uint8_t unk_1B; 
	uint32_t char_hash; // 1C
	uint32_t costume_hash; // 20
	uint32_t unk_hash; // 24
	uint32_t face_hash; // 28
	uint32_t hair_hash; // 2C
	uint32_t unk_30; //
	// ...
};


// The hash func used by the game
uint32_t hash_func(const char *str);
inline uint32_t hash_func(const std::string &str) { return hash_func(str.c_str()); }

std::string character_id_to_prefix(uint8_t id);
uint8_t character_prefix_to_id(const std::string &prefix);
uint8_t character_hash_to_id(uint32_t hash);

std::string pchar_to_prefix(uint8_t id);

uint32_t get_current_stage();

uint16_t script_get_num_params(void *script_object);
bool script_get_params(void *script, ScriptValue *params);

#endif
