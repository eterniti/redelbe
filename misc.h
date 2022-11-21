#ifndef __MISC__
#define __MISC__

#include <stdint.h>

enum RandomMusicMode
{
	RM_NONE,
	RM_MIX,
	RM_FULL,
	RM_FULLMIX,
};

uint32_t misc_process_bgm();

#endif
