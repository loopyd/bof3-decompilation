#ifndef PSX_AUDIO_CLI_H
#define PSX_AUDIO_CLI_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MKDIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#include <dirent.h>
#define MKDIR(d) mkdir(d, 0755)
#endif

#include "audio.h"
#include "util.h"
#include "emi.h"

#include "miniaudio.h"

/* ── audio source ──────────────────────────────────────────────────── */

typedef struct {
  uint8_t *      owned_vh, *owned_vb, *owned_sep;
  const uint8_t *vh, *vb, *sep;
  uint32_t       vh_sz, vb_sz, sep_sz;
} AudioSource;

void source_free(AudioSource* s);
int  source_from_emi(AudioSource* s, const char* path);
int  source_from_raw(AudioSource* s, const char* vh_p, const char* vb_p,
                     const char* sep_p);
int  source_from_dir(AudioSource* s, const char* dir);
int  source_auto(AudioSource* s, const char* path);
int  ends_with(const char* s, const char* suffix);

/* ── playback ──────────────────────────────────────────────────────── */

int play_buffer(int16_t* pcm, int64_t frames, int channels, int rate,
                float gain);
int play_source(AudioSource* s, int seq_idx, float gain, const char* outpath);
int write_stereo_output(const char* path, const int16_t* pcm, int64_t frames,
                        int rate);

/* ── args ──────────────────────────────────────────────────────────── */

const char* arg_str(int argc, char** argv, const char* flag);
int         arg_has(int argc, char** argv, const char* flag);
int         arg_int(int argc, char** argv, const char* flag, int def);
float       arg_flt(int argc, char** argv, const char* flag, float def);
float       arg_gain(int argc, char** argv);

/* ── BGM directory scanning ────────────────────────────────────────── */

typedef struct {
  char name[64];
  char path[512];
  int  events;
  int  tones;
} TrackInfo;

int find_bgm_dir(char* out, size_t sz);
int scan_tracks(const char* dir, TrackInfo* tracks, int max);
int find_track_path(const char* name, char* out, size_t sz);

/* ── commands ──────────────────────────────────────────────────────── */

int cmd_list(int argc, char** argv);
int cmd_play(int argc, char** argv);
int cmd_play_bgm(int argc, char** argv);
int cmd_play_xa(int argc, char** argv);
int cmd_play_vag(int argc, char** argv);
int cmd_render(int argc, char** argv);
int cmd_xa_inspect(int argc, char** argv);
int cmd_xa_decode(int argc, char** argv);
int cmd_vab_inspect(int argc, char** argv);
int cmd_bgm_audit(int argc, char** argv);
int cmd_vab_extract(int argc, char** argv);
int cmd_sep_inspect(int argc, char** argv);
int cmd_sep2mid(int argc, char** argv);
int cmd_emi_inspect(int argc, char** argv);
int cmd_psf_inspect(int argc, char** argv);
int cmd_psf_pack(int argc, char** argv);
int cmd_psf_run(int argc, char** argv);
int cmd_vab2sf2(int argc, char** argv);
int cmd_tui(int argc, char** argv);

#endif
