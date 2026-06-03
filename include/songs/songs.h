#ifndef SONGS_H
#define SONGS_H

#include "elems/notes.h"
#include "elems/super_mario.h"
#include "elems/always_with_me.h"
#include "elems/dream_wedding.h"
#include "elems/castle_in_the_sky.h"
#include "elems/canon.h"

// 歌曲总数
#define SONG_COUNT 5

// 歌曲信息结构
struct SongInfo {
    const int* notes;
    const int* durations;
    int length;          // 音符数量
};

// 歌曲查找表
static const SongInfo SONG_LIST[SONG_COUNT] = {
    { SuperMario_note,       SuperMario_duration,       sizeof(SuperMario_note)       / sizeof(int) },
    { AlwaysWithMe_note,     AlwaysWithMe_duration,     sizeof(AlwaysWithMe_note)     / sizeof(int) },
    { DreamWedding_note,     DreamWedding_duration,     sizeof(DreamWedding_note)     / sizeof(int) },
    { CastleInTheSky_note,   CastleInTheSky_duration,   sizeof(CastleInTheSky_note)   / sizeof(int) },
    { Canon_note,            Canon_duration,            sizeof(Canon_note)            / sizeof(int) },
};

#endif
