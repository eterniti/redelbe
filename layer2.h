#ifndef __LAYER2_H__
#define __LAYER2_H__

#include <unordered_map>
#include "rdbemu.h"
#include "debug.h"

struct VirtualRdbEntry;

enum Layer2ModType
{
	MOD_TYPE_COSTUME,
	MOD_TYPE_HAIR,
	MOD_TYPE_STAGE,
};

struct Layer2Mod
{
	int type;
	
	std::string name;
	std::string friendly_name;
	uint32_t chara_owner; // Unused for stage
	
	uint32_t slot_costume; // Mandatory (in costume) Ignored for hair
	uint32_t work_costume; // Mandatory (in costume) Ignored for hair.  If not in ini, it is assigned same value as slot_costume.
	
	uint32_t slot_face; // Can be 0
	uint32_t work_face; // Can be 0
	
	uint32_t slot_hair; // Mandatory (in hair) Can be 0 otherwise
	uint32_t work_hair; // Mandatory (in hair) Can be 0 otherwise. If not in ini, it is assigned same value as slot_hair.
	
	uint32_t slot_stage;  // Mandatory in stage mod, ignored for rest.
	uint32_t work_stage; // Mandatory in stage mod, ignored for rest		
	
	bool is_random; // Only meaningful for random costumes
	bool is_transform; // Only meaningful for random costumes
	
	uint32_t default_hair; // Optional. Can only be used by costumes.
	Layer2Mod *default_hair_mod; // Optional. Can only be used by costumes.
	std::string default_hair_mod_name; // This is temporal for default_hair_mod, until default_hair_mod can be resolved.
	
	uint32_t default_glasses; // Optional. Can only be used by costumes.
	bool has_default_glasses; // This is to discriminate the case of default_glasses being set to no glasses, in that case default_glasses would be 0
	
	bool disable_npc; // Optional. Can only be used by stages
	uint32_t bgm; // Optional. Can only be used by stages.
	
	Layer2Mod(const std::string &mod_name) : name(mod_name)
	{
		type = MOD_TYPE_COSTUME;
		
		size_t pos = mod_name.rfind('/');
		friendly_name = (pos == std::string::npos) ? mod_name : mod_name.substr(pos+1);
				
		chara_owner = 0;
		slot_costume = work_costume = 0;
		slot_face = work_face = 0;
		slot_hair = work_hair = 0;
		
		is_random = is_transform = false;
		
		default_hair = 0;
		default_hair_mod = nullptr;
		
		default_glasses = 0;
		has_default_glasses = false;
		
		disable_npc = false;
		bgm = 0;
	}
	
	std::unordered_map<uint32_t, VirtualRdbEntry> *character_files;
	std::unordered_map<uint32_t, VirtualRdbEntry> *material_files;
	std::unordered_map<uint32_t, VirtualRdbEntry> *kids_files;
	std::unordered_map<uint32_t, VirtualRdbEntry> *field4_files;
	std::unordered_map<uint32_t, VirtualRdbEntry> *rrp_files;
};

struct Layer2HairSlot
{
	std::vector<Layer2Mod *> mods;
	int mod_index;
	
	Layer2HairSlot()
	{
		mod_index = -1;
	}
	
	inline bool Advance()
	{
		if (mods.size() == 0)
			return false;
		
		mod_index++;
		
		if (mod_index >= (int)mods.size())
			mod_index = -1;
		
		return true;
	}
	
	inline bool Rewind()
	{
		if (mods.size() == 0)
			return false;
		
		mod_index--;
		
		if (mod_index < -1)
			mod_index = (int)mods.size() - 1;
		
		return true;
	}
	
	inline bool Advance(bool direction_up)
	{
		if (direction_up)
			return Advance();
		
		return Rewind();
	}
};

struct Layer2Slot
{
	uint32_t chara_owner;
	
	// Combination of costume/player-index must be unique for each Layer2slot
	// These two must never be written
	uint32_t costume; 
	uint32_t player_index;
	
	uint32_t current_face;
	uint32_t current_hair;
	uint32_t current_color;
	int current_hair_color;
	uint32_t chara_hash;
	
	std::vector<Layer2Mod *> mods;
	int mod_index; // -1 for vanilla	
	std::unordered_map<uint32_t, Layer2HairSlot *> hair_map;
	
	Layer2Slot(uint32_t owner, uint32_t slot, uint32_t index) : chara_owner(owner), costume(slot), player_index(index)
	{
		current_face = 0;
		current_hair = 0;
		current_color = 0;
		current_hair_color = -1;
		chara_hash = 0;
		mod_index = -1;
	}
	
	Layer2Mod *GetCurrentMod()
	{
		if (mod_index < 0)
			return nullptr;
		
		return mods[mod_index];
	}
	
	Layer2Mod *GetCurrentHairMod() 
	{
		auto it = hair_map.find(current_hair);
		if (it == hair_map.end())
			return nullptr;
		
		Layer2HairSlot *hs = it->second;		
		if (hs->mod_index < 0)
			return nullptr;
		
		return hs->mods[hs->mod_index];
	}
	
	bool SetCurrentHairMod(Layer2Mod *mod)
	{
		auto it = hair_map.find(current_hair);
		if (it == hair_map.end())
			return false;
		
		Layer2HairSlot *hs = it->second;
		
		if (!mod)
		{
			hs->mod_index = -1;
			return true;
		}		
		
		for (int i = 0; i < (int)hs->mods.size(); i++)
		{
			if (mod == hs->mods[i])
			{
				hs->mod_index = i;
				return true;
			}
		}
		
		return false;
	}
	
	inline bool Advance() // returns true if index changed
	{
		if (mods.size() == 0)
			return false;
		
		mod_index++;
		
		if (mod_index >= (int)mods.size())
			mod_index = -1;
		
		return true;
	}
	
	inline bool Rewind() // returns true if index changed
	{
		if (mods.size() == 0)
			return false;
		
		mod_index--;
		
		if (mod_index < -1)
			mod_index = (int)mods.size() - 1;
		
		return true;
	}
	
	inline bool Advance(bool direction_up)
	{
		if (direction_up)
			return Advance();
		
		return Rewind();
	}
	
	inline bool AdvanceHair() // returns true if index changed
	{
		Layer2Mod *current = GetCurrentHairMod();
		
		auto it = hair_map.find(current_hair);
		if (it == hair_map.end())
			return false;
		
		it->second->Advance();
		return (GetCurrentHairMod() != current);
	}
	
	inline bool RewindHair() // returns true if index changed
	{
		Layer2Mod *current = GetCurrentHairMod();
		
		auto it = hair_map.find(current_hair);
		if (it == hair_map.end())
			return false;
		
		it->second->Rewind();
		return (GetCurrentHairMod() != current);
	}
	
	inline bool AdvanceHair(bool direction_up)
	{
		if (direction_up)
			return AdvanceHair();
		
		return RewindHair();
	}
	
	inline uint32_t GetWorkCostume() 
	{
		if (mod_index >= 0)
		{
			return mods[mod_index]->work_costume;
		}
		
		return costume;			
	}
	
	inline uint32_t GetWorkFace() 
	{
		// For this, we are giving priority to hair mods over costume ones
		Layer2Mod *chm = GetCurrentHairMod();
		
		if (chm)
		{
			uint32_t face = chm->work_face;
			if (face != 0 && chm->slot_face == current_face)
				return face;
		}

		Layer2Mod *cm = GetCurrentMod();		
		if (cm)
		{
			uint32_t face = cm->work_face;
			if (face != 0 && cm->slot_face == current_face)
				return face;
		}
		
		return current_face;			
	}
	
	inline uint32_t GetWorkHair()
	{
		Layer2Mod *chm = GetCurrentHairMod();
		
		if (chm)
		{
			return chm->work_hair;			
		}
		
		Layer2Mod *cm = GetCurrentMod();			
		if (cm)
		{
			uint32_t hair = cm->work_hair;
			if (hair != 0 && cm->slot_hair == current_hair)
				return hair;
		}
		
		return current_hair;			
	}
	
	void SetCurrentHairModIndex(int index)
	{
		auto it = hair_map.find(current_hair);
		if (it == hair_map.end())
			return;
		
		Layer2HairSlot *hs = it->second;
		
		if (index >= 0 && index < (int)hs->mods.size())
		{
			hs->mod_index = index;
		}
		else
		{
			hs->mod_index = -1;
		}
	}
	
	size_t GetNumHairModsForCurrentHair()
	{
		auto it = hair_map.find(current_hair);
		if (it == hair_map.end())
			return 0;
		
		return it->second->mods.size();
	}
	
	bool IsRandom() 
	{
		Layer2Mod *cm = GetCurrentMod();
		if (!cm)
			return false;
		
		return cm->is_random;
	}
	
	bool IsTransform() 
	{
		Layer2Mod *cm = GetCurrentMod();
		if (!cm)
			return false;
		
		return cm->is_transform;
	}
	
	bool operator==(const Layer2Slot &rhs) const
	{
		if (chara_owner != rhs.chara_owner)
			return false;
		
		if (costume != rhs.costume || player_index != rhs.player_index)
			return false;
		
		if (current_face != rhs.current_face || current_hair != rhs.current_hair || current_color != rhs.current_color || chara_hash != rhs.chara_hash)
			return false;
		
		if (mods != rhs.mods || mod_index != rhs.mod_index)
			return false;
		
		if (hair_map != rhs.hair_map)
			return false;
		
		return true;
	}

    inline bool operator!=(const Layer2Slot &rhs) const
    {
        return !(*this == rhs);
    }
	
private:

	Layer2Slot(); // Disable normal constructor.
};

struct Layer2StageSlot
{
	
	uint32_t stage;
	std::vector<Layer2Mod *> mods;
	int mod_index; // -1 for vanilla	
	
	Layer2StageSlot(uint32_t stg) : stage(stg)
	{
		mod_index = -1;
	}
	
	Layer2Mod *GetCurrentMod()
	{
		if (mod_index < 0)
			return nullptr;
		
		return mods[mod_index];
	}
	
	bool SetCurrentMod(Layer2Mod *mod)
	{
		if (mod == nullptr)
		{
			mod_index = -1;
			return true;
		}
		
		for (size_t i = 0; i < mods.size(); i++)
		{
			if (mods[i] == mod)
			{
				mod_index = (int)i;
				return true;
			}
		}
		
		return false;
	}
	
	inline bool Advance() // returns true if index changed
	{
		if (mods.size() == 0)
			return false;
		
		mod_index++;
		
		if (mod_index >= (int)mods.size())
			mod_index = -1;
		
		return true;
	}
	
	inline bool Rewind() // returns true if index changed
	{
		if (mods.size() == 0)
			return false;
		
		mod_index--;
		
		if (mod_index < -1)
			mod_index = (int)mods.size() - 1;
		
		return true;
	}
	
	inline bool Advance(bool direction_up)
	{
		if (direction_up)
			return Advance();
		
		return Rewind();
	}
	
	inline uint32_t GetWorkStage() 
	{
		if (mod_index >= 0)
		{
			return mods[mod_index]->work_stage;
		}
		
		return stage;			
	}
	
private:
	Layer2StageSlot(); // Disable normal constructor.
};

enum
{
	MODE_VS,
	MODE_ARCADE, // also time trial
	MODE_SURVIVAL,
	MODE_FREE_TRAINING
};

struct RandomQueue
{
	int mode;
	
	std::vector<Layer2Slot> rslots1;
	std::vector<Layer2Slot> rslots2;
	
	int next_p1;
	int next_p2;	
	
	RandomQueue()
	{
		mode = -1;
		next_p1 = next_p2 = 0;
	}
	
	void Reset()
	{
		mode = -1;
		next_p1 = next_p2 = 0;
		rslots1.clear();
		rslots2.clear();
	}
	
	inline int GetNext(uint32_t player_index) const
	{
		return (player_index == 0) ? next_p1 : next_p2;
	}
	
	inline std::vector<Layer2Slot> &GetQueue(uint32_t player_index)
	{
		return (player_index == 0) ? rslots1 : rslots2;
	}
	
	inline size_t QueueSize(uint32_t player_index) const
	{
		return (player_index == 0) ? rslots1.size() : rslots2.size();
	}
	
	inline Layer2Slot *GetNextSlot(uint32_t player_index)
	{
		int next = GetNext(player_index);
		std::vector<Layer2Slot> &queue = GetQueue(player_index);
		
		if (next < 0 || next >= (int)queue.size())
			return nullptr;
		
		return &queue[next];
	}
	
	inline Layer2Slot *GetSlotAt(uint32_t player_index, int index)
	{
		std::vector<Layer2Slot> &queue = GetQueue(player_index);
		
		if (index < 0 || index >= (int)queue.size())
			return nullptr;
		
		return &queue[index];
	}
	
	inline void Advance(uint32_t player_index)
	{
		if (player_index == 0)
			next_p1++;
		else
			next_p2++;
	}	
};

struct RandomStageQueue
{
	int mode;
	
	std::vector<Layer2StageSlot> rslots;
	int next;
	
	RandomStageQueue()
	{
		mode = -1;
		next = 0;
	}
	
	void Reset()
	{
		mode = -1;
		next = 0;
		rslots.clear();
	}
	
	inline int GetNext() const
	{
		return next;
	}
	
	inline std::vector<Layer2StageSlot> &GetQueue()
	{
		return rslots;
	}
	
	inline size_t QueueSize() const
	{
		return rslots.size();
	}
	
	inline Layer2StageSlot *GetNextSlot()
	{
		int next = GetNext();
		std::vector<Layer2StageSlot> &queue = GetQueue();
		
		if (next < 0 || next >= (int)queue.size())
			return nullptr;
		
		return &queue[next];
	}
	
	inline Layer2StageSlot *GetSlotAt(int index)
	{
		std::vector<Layer2StageSlot> &queue = GetQueue();
		
		if (index < 0 || index >= (int)queue.size())
			return nullptr;
		
		return &queue[index];
	}
	
	inline void Advance()
	{
		next++;
	}	
};

void layer2_init();
bool layer2_add_mod(const std::string &name, const std::string &ini_path, std::unordered_map<uint32_t, VirtualRdbEntry> *cfiles, std::unordered_map<uint32_t, VirtualRdbEntry> *mfiles,
					std::unordered_map<uint32_t, VirtualRdbEntry> *kfiles, std::unordered_map<uint32_t, VirtualRdbEntry> *ffiles, std::unordered_map<uint32_t, VirtualRdbEntry> *rfiles);
void layer2_finalization();

#endif
