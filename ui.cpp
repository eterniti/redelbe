#include <unordered_set>

#include "Utils.h"
#include "redelbe.h"
#include "ui.h"
#include "game.h"
#include "debug.h"

typedef void (* LSHLayoutType)(void *, uint32_t);
typedef void (*SULType)(void *, uint32_t, void *);
typedef void (*SUMLType)(void *, uint32_t, uint32_t, void *);

static LSHLayoutType layout_show_layout, layout_hide_layout;
//static SULType scene_use_layout;
//static SUMLType scene_use_multiple_layout;

static void **playout_object;

static bool is_layout_hidden = false;

static std::unordered_set<uint32_t> battle_layouts =
{
	0x68c58d27,
	0x44774b37,
	0xa3476d26,
	0x54861cba,
	0xb6873be6, // tutorial_message
	0x5adeac41, // tutorial_qte
	0xfd532010,
	0xd332b554,
	0x2b107044,
	0x4cfc5638,
	0xd7c47612,
	0x1b0565f4,
	0x950e531c, // skill_info
	0x32503255,
	0xc0451ada,
	0x899e7208,
	0xe3ec1bc9,
	0x067e8cce,
	0xaaf4fcbf,	
	// The above ones were the ones the game hide in spectator mode
	// Below are the ones added by me	
	0x134067c6,
	0x2f6cdd26,
	0xb0fb3d01
};

static DWORD beep_thread(LPVOID param)
{
	uint64_t param64 = (uint64_t)(uintptr_t)param;
	DWORD frequency = (DWORD)(param64 >> 32);
	DWORD duration = (DWORD)(param64&0xFFFFFFFF);
	
	Beep(frequency, duration);
	
	return 0;
}

static void AsyncBeep(DWORD frequency, DWORD duration)
{
	CreateThread(nullptr, 0, beep_thread, (void *)(uintptr_t)(duration | ((uint64_t)frequency << 32)), 0, nullptr);
}

void toggle_battle_hud()
{
	if (!playout_object || !(*playout_object))
		return;
	
	//DPRINTF("TOGGLE begin\n");
	AsyncBeep(1250, 50);
	
	bool hidden = is_layout_hidden;
	is_layout_hidden = !is_layout_hidden; // We have to change it now, so that our own hook in layout_show_layout doesn't stop us from showing the hud
	
	for (uint32_t layout: battle_layouts)
	{
		if (hidden)
			layout_show_layout(*playout_object, layout);
		else
			layout_hide_layout(*playout_object, layout);
	}

	//DPRINTF("TOGGLE_END\n");
}

void toggle_battle_hud_reset()
{
	is_layout_hidden = false;
}

// Patch section referenced by the xml (needs to be in C linking)

extern "C"
{
	
PUBLIC void OnLOLocated2(void *addr)
{
	playout_object = (void **)GetAddrFromRel(addr);
}	
	
PUBLIC void SetupLSL(LSHLayoutType orig)
{
	layout_show_layout = orig;
}

PUBLIC void SetupLHL(LSHLayoutType orig)
{
	layout_hide_layout = orig;
}

/*PUBLIC void SetupSUL(SULType orig)
{
	scene_use_layout = orig;
}

PUBLIC void SetupSUML(SUMLType orig)
{
	scene_use_multiple_layout = orig;
}*/

PUBLIC void layout_show_layout_patched(void *pthis, uint32_t layout)
{
	//DPRINTF("Show layout 0x%x (%d)\n", layout, (battle_layouts.find(layout) != battle_layouts.end()));
	
	if (is_layout_hidden && battle_layouts.find(layout) != battle_layouts.end())
	{
		//DPRINTF("^Blocked.\n");
		return;
	}
	
	layout_show_layout(pthis, layout);
}

PUBLIC void layout_hide_layout_patched(void *pthis, uint32_t layout)
{
	//DPRINTF("Hide layout 0x%x (%d)\n", layout, (battle_layouts.find(layout) != battle_layouts.end()));
	layout_hide_layout(pthis, layout);
}

/*PUBLIC void scene_use_layout_patched(void *pthis, uint32_t layout, void *script_object)
{
	scene_use_layout(pthis, layout, script_object);
	
	if (is_layout_hidden && playout_object && *playout_object && battle_layouts.find(layout) != battle_layouts.end())
	{
		layout_hide_layout(*playout_object, layout);
	}
}

PUBLIC void scene_use_multiple_layout_patched(void *pthis, uint32_t layout, uint32_t n, void *script_object)
{
	scene_use_multiple_layout(pthis, layout, n, script_object);
	
	if (is_layout_hidden && playout_object && *playout_object && battle_layouts.find(layout) != battle_layouts.end())
	{
		layout_hide_layout(*playout_object, layout);
	}
}*/
	
} // extern "C"
