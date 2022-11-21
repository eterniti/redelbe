#include <unordered_map>

#include "Utils.h"
#include "redelbe.h"
#include "misc.h"
#include "game.h"
#include "debug.h"

typedef bool (* VCTType)(uint32_t);
typedef uint32_t * (* GCBTType)();

static VCTType validate_character_track;
static GCBTType get_character_bgm_table;

static int random_music_mode = RM_NONE;
static float random_music_stage_probability = 0.5f;
static std::vector<uint32_t> full_random_bgms;
static std::vector<uint32_t> fm_char[PCHAR_NUM];
static std::unordered_map<uint32_t, std::vector<uint32_t>> fm_stages;

uint32_t misc_process_bgm()
{
	if (random_music_mode == RM_FULLMIX)
	{
		uint32_t stage = get_current_stage();
		uint32_t bgm = 0;
		
		auto it = fm_stages.find(stage);
		if (it != fm_stages.end())
		{
			auto &list = it->second;
			if (list.size() == 1)
			{
				bgm = list.front();
			}
			else if (list.size() > 1)
			{
				size_t rnd = (size_t)Utils::RandomInt(0, list.size()-1);
				bgm = list[rnd];
			}
		}
		
		return bgm;
	}
	
	if (random_music_mode != RM_FULL)
		return 0;
	
	if (full_random_bgms.size() == 0)
		return 0;
	
	if (full_random_bgms.size() == 1)
		return full_random_bgms.front();
	
	size_t rnd = (size_t)Utils::RandomInt(0, full_random_bgms.size()-1);
	if (rnd >= full_random_bgms.size()) // Shouldn't happen, but just to be in safe side
		return 0;
		
	return full_random_bgms[rnd];
}

extern "C"
{
	
PUBLIC void SetupRandomMusic(VCTType orig)
{
	validate_character_track = orig;
	
	std::string mode;
	int stage_probability;
	
	if (!ini.GetStringValue("RandomMusic", "mode", mode))
		return;
	
	if (mode == "mix")
		random_music_mode = RM_MIX;
	else if (mode == "full")
		random_music_mode = RM_FULL;
	else if (mode == "fullmix")
		random_music_mode = RM_FULLMIX;
	else
	{
		FTPRINTF(true, "Unknown RandomMusic:Mode \"%s\"\n", mode.c_str());
		return;
	}
	
	ini.GetIntegerValue("RandomMusic", "stage_probability", &stage_probability, 50);
	
	if (stage_probability >= 100)
		random_music_stage_probability = 1.0f;
	else if (stage_probability <= 0)
		random_music_stage_probability = 0.0f;
	else
		random_music_stage_probability = ((float)stage_probability) / 100.0f;
	
	if (random_music_mode == RM_FULL)
	{
		FILE *f = fopen("REDELBE/random_tracks.txt", "r");
		if (f)
		{
			char line[512];
			
			while (fgets(line, sizeof(line), f))
			{
				std::string str = Utils::ToLowerCase(line);
				Utils::TrimString(str);
				
				if (str.length() == 0 || str.front() == ';')
					continue;
				
				full_random_bgms.push_back(hash_func(str));
			}
			
			fclose(f);
			
			if (full_random_bgms.size() == 0)
			{
				DPRINTF("WARNING: random_tracks.txt doesn't contain any tracks! (random music will be disabled).\n");
				random_music_mode = RM_NONE;
			}
		}
		else
		{
			DPRINTF("WARNING: cannot open random_tracks.txt, random music will be disabled.\n");
			random_music_mode = RM_NONE;
		}
	}
	else if (random_music_mode == RM_FULLMIX)
	{
		IniFile fm_ini;
		
		if (fm_ini.LoadFromFile("REDELBE/fullmix.ini"))
		{
			for (uint8_t ch = PCHAR_ZAC; ch < PCHAR_NUM; ch++)
			{
				std::string str;
				
				if (fm_ini.GetStringValue(pchar_to_prefix(ch), "list", str))
				{
					Utils::TrimString(str);
					if (str.length() > 0)
					{					
						std::vector<std::string> list;
						Utils::GetMultipleStrings(str, list);
						
						for (const std::string &track : list)
						{
							fm_char[ch].push_back(hash_func(track));
						}
					}					
				}
				
				//DPRINTF("Character %s has %Iu tracks.\n", character_id_to_prefix(ch).c_str(), fm_char[ch].size());
			}
			
			static const std::vector<std::string> music_stages = 
			{
				"S0101PIR",
				"S0102PIR",
				"S0201BST",
				"S0301CRM",
				"S0302CRM",
				"S0401CLS",
				"S0411CLS",				
				"S0501PAS",
				"S0502PAS",
				"S0601LBN",
				"S0701MUS",
				"S0702MUS",				
				"S0801LOS",
				"S0802LOS",
				"S0901WAY",
				"S1101BAM",				
				"S1102BAM",
				"S1201PRO",
				"S1301GYM",
				"S1501GRD",				
				"S1601ISL",
			};
			
			for (size_t i = 0; i < music_stages.size(); i++)
			{
				uint32_t hash = hash_func(music_stages[i]);
				fm_stages[hash] = std::vector<uint32_t>();
				
				std::string str;
				if (fm_ini.GetStringValue(music_stages[i], "list", str))
				{
					Utils::TrimString(str);
					if (str.length() > 0)
					{					
						std::vector<std::string> list;
						Utils::GetMultipleStrings(str, list);
						
						for (const std::string &track : list)
						{
							fm_stages[hash].push_back(hash_func(track));
						}
					}
				}
				
				//DPRINTF("Stage %s has %Iu tracks.\n", music_stages[i].c_str(), fm_stages[hash].size());
			}
		}
		else
		{
			DPRINTF("WARNING: cannot open fullmix.ini, random music will be disabled.\n");
			random_music_mode = RM_NONE;
		}
	}
}

PUBLIC void SetupGCBT(GCBTType orig)
{
	get_character_bgm_table = orig;
}

PUBLIC bool validate_character_track_patched(uint32_t bgm)
{
	//DPRINTF("Validate character track 0x%x\n", bgm);
	
	bool ret = validate_character_track(bgm);
	
	if (random_music_mode == RM_FULL)
		return false;
	
	if (random_music_mode == RM_MIX || random_music_mode == RM_FULLMIX)
	{
		ret = !(Utils::RandomProbability() <= random_music_stage_probability);
	}
	
	return ret;
}

PUBLIC uint32_t *get_character_bgm_table_patched()
{
	if (random_music_mode != RM_FULLMIX)
		return get_character_bgm_table();
	
	static uint32_t table[100];
	memset(table, 0, sizeof(table));
	
	for (uint8_t ch = PCHAR_ZAC; ch < PCHAR_NUM; ch++)
	{		
		const auto &list = fm_char[ch];
		uint32_t track = 0;
		
		if (list.size() == 1)
		{
			track = list.front();
		}
		else if (list.size() > 1)
		{
			size_t rnd = (size_t)Utils::RandomInt(0, list.size()-1);
			track = list[rnd];
		}
		
		table[ch] = track;
	}
	
	return table;
}
	
} // extern "C"