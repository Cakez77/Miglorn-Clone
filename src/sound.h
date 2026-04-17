#pragma once
#include "lib.h"

enum SoundID
{
  SOUND_NONE,
  SOUND_BUTTON_HOVER,

  SOUND_COUNT,
};

// #############################################################################
//                           Sound Constants
// #############################################################################
static constexpr int MAX_CONCURRENT_SOUNDS = 64;
static constexpr int SOUNDS_BUFFER_SIZE = MB(256);
static constexpr int MAX_SOUND_PATH_LENGTH = 256;

// #############################################################################
//                           Sound Structs
// #############################################################################
enum SoundOptionBits
{
	SOUND_OPTION_FADE_OUT = BIT(0),
	SOUND_OPTION_FADE_IN = BIT(1),
	SOUND_OPTION_START = BIT(2),
	SOUND_OPTION_LOOP = BIT(3),
	SOUND_OPTION_SFX = BIT(4),
	SOUND_OPTION_SLOWDOWN = BIT(5),
	SOUND_OPTION_WAKEUP = BIT(6),
	SOUND_OPTION_SPEED_UP = BIT(7),
	SOUND_OPTION_DONE = BIT(8),
	SOUND_OPTION_UI = BIT(9)
};
typedef int SoundOptionFlags;

struct SoundOptions
{
	float pitch;
	float fadeDuration = 2.0f;
	float maxDuration;
  int playBegin;
  int playLength;
  int loopBegin;
  int loopLength;
  int loopCount;
	SoundOptionFlags flags;
};

struct Sound
{
	SoundID ID;
	char path[MAX_SOUND_PATH_LENGTH];
	SoundOptions options;
	float fadeTimer;
	int size;
	char* data;
	void* voice; // win32 voice (XAudio)
	float lastPlayTime;

	bool operator==(Sound& other)
	{
		return 
			str_cmp(path, other.path) &&
			size == other.size &&
			data == other.data;
	}
};

// #############################################################################
//                           Sound Functions
// #############################################################################
void play_sound(SoundID soundID, SoundOptions options = {})
{
  // do nothing for now
}