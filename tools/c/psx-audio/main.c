#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MKDIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#include <dirent.h>
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"
#include "util.h"
#include "emi.h"
#include "cli.h"
#include "psf.h"
#include "psx_machine.h"

static char g_root_dir[512] = ".";

int ends_with(const char* s, const char* suffix) {
  size_t sl = strlen(s), xl = strlen(suffix);
  if (xl > sl)
    return 0;
  for (size_t i = 0; i < xl; i++) {
    char a = s[sl - xl + i], b = suffix[i];
    if (a >= 'A' && a <= 'Z')
      a += 32;
    if (b >= 'A' && b <= 'Z')
      b += 32;
    if (a != b)
      return 0;
  }
  return 1;
}

/* ── audio source ──────────────────────────────────────────────────── */

void source_free(AudioSource* s) {
  free(s->owned_vh);
  free(s->owned_vb);
  free(s->owned_sep);
  memset(s, 0, sizeof(*s));
}

int source_from_emi(AudioSource* s, const char* path) {
  size_t  len;
  EmiFile emi;

  memset(s, 0, sizeof(*s));
  s->owned_vh = read_file(path, &len);
  if (!s->owned_vh)
    return -1;
  if (emi_parse(s->owned_vh, len, &emi) != 0) {
    source_free(s);
    return -1;
  }
  s->vh = emi_find_type(&emi, EMI_TYPE_VH, &s->vh_sz);
  s->vb = emi_find_type(&emi, EMI_TYPE_VB, &s->vb_sz);
  s->sep = emi_find_type(&emi, EMI_TYPE_SEQ, &s->sep_sz);
  if (!s->vh || !s->vb) {
    source_free(s);
    return -1;
  }
  return 0;
}

int source_from_raw(AudioSource* s, const char* vh_p, const char* vb_p,
                    const char* sep_p) {
  size_t vl, bl, sl;

  memset(s, 0, sizeof(*s));
  uint8_t* vh = read_file(vh_p, &vl);
  uint8_t* vb = read_file(vb_p, &bl);
  uint8_t* sep = sep_p ? read_file(sep_p, &sl) : NULL;
  if (!vh || !vb) {
    free(vh);
    free(vb);
    free(sep);
    return -1;
  }
  s->owned_vh = vh;
  s->owned_vb = vb;
  s->owned_sep = sep;
  s->vh = vh;
  s->vh_sz = (uint32_t)vl;
  s->vb = vb;
  s->vb_sz = (uint32_t)bl;
  s->sep = sep;
  s->sep_sz = sep ? (uint32_t)sl : 0;
  return 0;
}

int source_from_dir(AudioSource* s, const char* dir) {
  char vh_p[512], vb_p[512], sep_p[512];
  int  found_vh = 0, found_vb = 0, found_sep = 0;
#ifndef _WIN32
  DIR*           d = opendir(dir);
  struct dirent* ent;
  if (!d)
    return -1;
  while ((ent = readdir(d)) != NULL) {
    char    full[600];
    uint8_t hdr[4];
    FILE*   f;
    if (!ends_with(ent->d_name, ".bin"))
      continue;
    snprintf(full, sizeof(full), "%s/%s", dir, ent->d_name);
    f = fopen(full, "rb");
    if (!f)
      continue;
    if (fread(hdr, 1, 4, f) != 4) {
      fclose(f);
      continue;
    }
    fclose(f);
    if (memcmp(hdr, "\x70\x42\x41\x56", 4) == 0) {
      snprintf(vh_p, sizeof(vh_p), "%s", full);
      found_vh = 1;
    } else if (memcmp(hdr, "\x70\x51\x45\x53", 4) == 0) {
      snprintf(sep_p, sizeof(sep_p), "%s", full);
      found_sep = 1;
    } else {
      snprintf(vb_p, sizeof(vb_p), "%s", full);
      found_vb = 1;
    }
  }
  closedir(d);
#endif
  if (!found_vh || !found_vb)
    return -1;
  return source_from_raw(s, vh_p, vb_p, found_sep ? sep_p : NULL);
}

int source_auto(AudioSource* s, const char* path) {
  FILE* f = fopen(path, "rb");
  if (f) {
    uint8_t hdr[16];
    size_t  n = fread(hdr, 1, 16, f);
    fclose(f);
    if (n >= 16 && emi_check_magic(hdr, n))
      return source_from_emi(s, path);
  }
  if (source_from_dir(s, path) == 0)
    return 0;
  return source_from_emi(s, path);
}

/* ── playback ─────────────────────────────────────────────────────── */

static int16_t* g_pcm;
static int64_t  g_pos, g_total;
static int      g_channels;

static void data_callback(ma_device* dev, void* out, const void* in,
                          ma_uint32 frames) {
  int16_t* dst = (int16_t*)out;
  int      needed = (int)(frames * (ma_uint32)g_channels);
  int      avail = (int)(g_total - g_pos);
  int      n = needed < avail ? needed : avail;
  (void)dev;
  (void)in;
  if (n > 0) {
    memcpy(dst, g_pcm + g_pos, (size_t)n * sizeof(int16_t));
    g_pos += n;
  }
  if (n < needed)
    memset(dst + n, 0, (size_t)(needed - n) * sizeof(int16_t));
}

int play_buffer(int16_t* pcm, int64_t frames, int channels, int rate,
                float gain) {
  ma_device_config cfg;
  ma_device        dev;

  if (gain != 1.0f) {
    int64_t total = frames * channels;
    for (int64_t i = 0; i < total; i++) {
      float v = (float)pcm[i] * gain;
      pcm[i] = (int16_t)(v > 32767.0f ? 32767 : v < -32768.0f ? -32768 : v);
    }
  }

  g_pcm = pcm;
  g_pos = 0;
  g_total = frames * channels;
  g_channels = channels;
  cfg = ma_device_config_init(ma_device_type_playback);
  cfg.playback.format = ma_format_s16;
  cfg.playback.channels = (ma_uint32)channels;
  cfg.sampleRate = (ma_uint32)rate;
  cfg.dataCallback = data_callback;

  if (ma_device_init(NULL, &cfg, &dev) != MA_SUCCESS) {
    fprintf(stderr, "error: audio device init failed\n");
    return -1;
  }
  ma_device_start(&dev);
  printf("  \xe2\x96\xb6 %.1fs  %dHz  %s  gain=%.1f  [Ctrl+C]\n",
         (double)frames / rate, rate, channels == 2 ? "stereo" : "mono", gain);
  while (g_pos < g_total)
    ma_sleep(50);
  ma_device_uninit(&dev);
  return 0;
}

int write_stereo_output(const char* path, const int16_t* pcm, int64_t frames,
                        int rate) {
  if (ends_with(path, ".ogg"))
    return ogg_write_stereo(path, pcm, frames, rate);
  if (ends_with(path, ".flac"))
    return flac_write_stereo(path, pcm, frames, rate);
  return wav_write_stereo(path, pcm, frames, rate);
}

int play_source(AudioSource* s, int seq_idx, float gain, const char* outpath) {
  if (s->sep) {
    RenderOutput ro;
    int          rc;

    if (render_bgm(s->sep, s->sep_sz, s->vh, s->vh_sz, s->vb, s->vb_sz, seq_idx,
                   44100, &ro) != 0) {
      fprintf(stderr, "error: render failed\n");
      return -1;
    }
    if (outpath) {
      if (gain != 1.0f) {
        int64_t total = ro.frames * 2;
        for (int64_t i = 0; i < total; i++) {
          float v = (float)ro.pcm[i] * gain;
          ro.pcm[i] = (int16_t)(v > 32767.0f    ? 32767
                                : v < -32768.0f ? -32768
                                                : v);
        }
      }
      if (write_stereo_output(outpath, ro.pcm, ro.frames, ro.rate) != 0) {
        fprintf(stderr, "error: failed to write %s\n", outpath);
        free(ro.pcm);
        return -1;
      }
      printf("  wrote %s (%.1fs, %dHz stereo)\n", outpath,
             (double)ro.frames / ro.rate, ro.rate);
      rc = 0;
    } else {
      rc = play_buffer(ro.pcm, ro.frames, 2, ro.rate, gain);
    }
    free(ro.pcm);
    return rc;
  }
  VabHdr   hdr;
  int16_t* pcm = NULL;
  if (vab_parse_vh(s->vh, s->vh_sz, &hdr) != 0)
    return -1;
  int n = vab_decode_vag(s->vb, s->vb_sz, &hdr, 0, &pcm);
  if (n <= 0 || !pcm)
    return -1;
  int rc;
  if (outpath) {
    wav_write_mono(outpath, pcm, n, 44100);
    printf("  wrote %s (%d samples, 44100Hz mono)\n", outpath, n);
    rc = 0;
  } else {
    rc = play_buffer(pcm, n, 1, 44100, gain);
  }
  free(pcm);
  return rc;
}

/* ── args ─────────────────────────────────────────────────────────── */

const char* arg_str(int argc, char** argv, const char* flag) {
  for (int i = 1; i < argc - 1; i++)
    if (strcmp(argv[i], flag) == 0)
      return argv[i + 1];
  return NULL;
}
int arg_has(int argc, char** argv, const char* flag) {
  int i;
  for (i = 1; i < argc; i++)
    if (strcmp(argv[i], flag) == 0)
      return 1;
  return 0;
}
int arg_int(int argc, char** argv, const char* flag, int def) {
  const char* v = arg_str(argc, argv, flag);
  return v ? atoi(v) : def;
}
float arg_flt(int argc, char** argv, const char* flag, float def) {
  const char* v = arg_str(argc, argv, flag);
  return v ? (float)atof(v) : def;
}

float arg_gain(int argc, char** argv) {
  const char* v = arg_str(argc, argv, "--gain");
  return v ? (float)atof(v) : arg_flt(argc, argv, "-g", 1.0f);
}

/* ── BGM directory scanning ───────────────────────────────────────── */

int find_bgm_dir(char* out, size_t sz) {
  char        cand[600];
  const char* suffixes[] = {"out/extracted/BIN/BGM", NULL};
  const char* rel_cands[] = {"out/extracted/BIN/BGM",
                             "../out/extracted/BIN/BGM",
                             "../../out/extracted/BIN/BGM", NULL};

  snprintf(cand, sizeof(cand), "%s/%s", g_root_dir, suffixes[0]);
#ifndef _WIN32
  {
    DIR* d = opendir(cand);
    if (d) {
      closedir(d);
      snprintf(out, sz, "%s", cand);
      return 0;
    }
  }
#endif
  for (int i = 0; rel_cands[i]; i++) {
#ifndef _WIN32
    DIR* d = opendir(rel_cands[i]);
    if (d) {
      closedir(d);
      snprintf(out, sz, "%s", rel_cands[i]);
      return 0;
    }
#endif
  }
  return -1;
}

int scan_tracks(const char* dir, TrackInfo* tracks, int max) {
#ifndef _WIN32
  DIR*           d = opendir(dir);
  struct dirent* ent;
  int            count = 0;
  if (!d)
    return 0;
  while ((ent = readdir(d)) != NULL && count < max) {
    char        path[512];
    AudioSource s;
    if (!ends_with(ent->d_name, ".emi"))
      continue;
    snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
    if (source_from_emi(&s, path) != 0)
      continue;
    TrackInfo* t = &tracks[count];
    memset(t, 0, sizeof(*t));
    strncpy(t->name, ent->d_name, sizeof(t->name) - 5);
    char* dot = strrchr(t->name, '.');
    if (dot)
      *dot = '\0';
    strncpy(t->path, path, sizeof(t->path) - 1);
    if (s.vh) {
      VabHdr hdr;
      if (vab_parse_vh(s.vh, s.vh_sz, &hdr) == 0)
        t->tones = (int)hdr.tone_count;
    }
    if (s.sep) {
      SepFile sep;
      if (sep_parse(s.sep, s.sep_sz, &sep) == 0 && sep.sequence_count > 0) {
        t->events = sep.sequences[0].event_count;
        sep_free(&sep);
      }
    }
    source_free(&s);
    count++;
  }
  closedir(d);
  for (int i = 0; i < count - 1; i++)
    for (int j = i + 1; j < count; j++)
      if (strcmp(tracks[i].name, tracks[j].name) > 0) {
        TrackInfo tmp = tracks[i];
        tracks[i] = tracks[j];
        tracks[j] = tmp;
      }
  return count;
#else
  (void)dir;
  (void)tracks;
  (void)max;
  return 0;
#endif
}

int find_track_path(const char* name, char* out, size_t sz) {
  char dir[512], upper[128];
  strncpy(upper, name, sizeof(upper) - 1);
  upper[sizeof(upper) - 1] = '\0';
  for (char* p = upper; *p; p++)
    if (*p >= 'a' && *p <= 'z')
      *p -= 32;
  if (ends_with(upper, ".EMI"))
    upper[strlen(upper) - 4] = '\0';

  if (find_bgm_dir(dir, sizeof(dir)) != 0)
    return -1;

  const char* pats[] = {"%s/%s.EMI", "%s/BGM%s.EMI", "%s/%s", "%s/BGM%s", NULL};
  for (int i = 0; pats[i]; i++) {
    snprintf(out, sz, pats[i], dir, upper);
    FILE* f = fopen(out, "rb");
    if (f) {
      fclose(f);
      return 0;
    }
  }
  return -1;
}

/* ── help ─────────────────────────────────────────────────────────── */

static void usage(void) {
  printf(
      "bof3-audio — PSX audio player, decoder, and exporter\n"
      "\n"
      "usage: bof3-audio <command> [args]\n"
      "\n"
      "browse:\n"
      "  list [filter]                         list BGM tracks\n"
      "  emi-inspect <file.EMI>                show EMI contents\n"
      "  psf-inspect <file.psf>                load PSF1/MiniPSF image\n"
      "  psf-run <file.psf> [-n N] [--call A]  run or call into a bounded PSF1 "
      "image\n"
      "\n"
      "play:\n"
      "  play <target> [-s N] [-g GAIN]        play (auto-detects format)\n"
      "  play-bgm <track|EMI|vh vb sep>        play BGM music\n"
      "  play-xa <file.STR> [-c CH]            play XA stream\n"
      "  play-vag <vh> <vb> [-v N]             play VAB samples\n"
      "  tui [-g GAIN]                       interactive TUI player\n"
      "\n"
      "export:\n"
      "  render <target> -o FILE                render BGM to WAV, Ogg Vorbis, "
      "or "
      "FLAC\n"
      "  psf-pack <PS-X EXE> -o FILE            package a PSF1 executable\n"
      "  xa-decode <str> -o out.wav [-c CH]    decode XA to WAV\n"
      "  vab-extract <vh> <vb> -o DIR          extract VAGs to WAV\n"
      "  vab2sf2 <vh> <vb|EMI> -o out.sf2     export SoundFont 2\n"
      "  sep2mid <sep> -o out.mid [-s N]       export to MIDI\n"
      "\n"
      "inspect:\n"
      "  xa-inspect <str>                      list XA streams\n"
      "  vab-inspect <vh>                      show VAB info\n"
      "  bgm-audit <track|EMI|directory> [--details]\n"
      "                                        audit VAB/SEP consistency and "
      "renderer "
      "gaps\n"
      "  sep-inspect <sep> [--programs] [--notes] [--bends] [--controls] "
      "[--events]\n"
      "                                        show SEP events and histograms\n"
      "\n"
      "options:\n"
      "  -s N       sequence index (-1 = all, default)\n"
      "  -g, --gain GAIN  playback gain (default: 1.0)\n"
      "  -c CH      XA channel (default: 0)\n"
      "  -v N       VAG index\n"
      "  -o PATH    output file\n"
      "\n"
      "run 'bof3-audio --examples' for usage examples\n");
}

static void examples(void) {
  printf(
      "examples:\n"
      "\n"
      "  # browse all BGM tracks\n"
      "  bin/psx-audio list\n"
      "  bin/psx-audio list BAT          # filter by name\n"
      "\n"
      "  # play BGM by track name (searches out/extracted/BIN/BGM/)\n"
      "  bin/psx-audio play BGM000\n"
      "  bin/psx-audio play BGMBAT06 -g 0.5\n"
      "  bin/psx-audio play BGMBAT02 --gain 0.7\n"
      "  bin/psx-audio play BGMOPN -s 1  # play sequence 1\n"
      "\n"
      "  # play directly from EMI archive (no extraction needed)\n"
      "  bin/psx-audio play out/extracted/BIN/BGM/BGM000.EMI\n"
      "  bin/psx-audio play-bgm out/extracted/BIN/BGM/BGMBAT06.EMI\n"
      "\n"
      "  # play from extracted directory\n"
      "  bin/psx-audio play out/extracted/BIN/BGM/BGM000\n"
      "\n"
      "  # play from raw files\n"
      "  bin/psx-audio play-bgm 0.bin 2.bin 1.bin\n"
      "\n"
      "  # play XA streaming audio\n"
      "  bin/psx-audio play out/extracted/BIN/SCE_XA/VOICE.STR -c 0\n"
      "  bin/psx-audio play-xa out/extracted/BIN/SCE_XA/S_XA00.STR -c 3\n"
      "\n"
      "  # play individual VAB samples\n"
      "  bin/psx-audio play-vag 0.bin 2.bin -v 0\n"
      "\n"
      "  # render to WAV\n"
      "  bin/psx-audio render BGM000 -o bgm000.wav\n"
      "  bin/psx-audio render BGM000 -o bgm000.ogg\n"
      "  bin/psx-audio render BGM000 -o bgm000.flac\n"
      "  bin/psx-audio render out/extracted/BIN/BGM/BGMOPN.EMI -o opening.wav\n"
      "\n"
      "  # export\n"
      "  bin/psx-audio xa-decode VOICE.STR -o voice_ch0.wav -c 0\n"
      "  bin/psx-audio vab-extract 0.bin 2.bin -o samples/\n"
      "  bin/psx-audio vab2sf2 BGM000.EMI -o bgm000.sf2\n"
      "  bin/psx-audio sep2mid 1.bin -o track.mid\n"
      "\n"
      "  # inspect\n"
      "  bin/psx-audio emi-inspect out/extracted/BIN/BGM/BGM000.EMI\n"
      "  bin/psx-audio xa-inspect out/extracted/BIN/SCE_XA/VOICE.STR\n"
      "  bin/psx-audio vab-inspect 0.bin\n"
      "  bin/psx-audio sep-inspect 1.bin\n");
}

/* ── main ─────────────────────────────────────────────────────────── */

int main(int argc, char** argv) {
  if (argc < 2) {
    usage();
    return 0;
  }

  {
    int i = 1;
    while (i < argc - 1) {
      if (strcmp(argv[i], "--dir") == 0) {
        snprintf(g_root_dir, sizeof(g_root_dir), "%s", argv[i + 1]);
        for (int j = i; j < argc - 2; j++)
          argv[j] = argv[j + 2];
        argc -= 2;
        continue;
      }
      i++;
    }
  }

  {
    char* pos[256];
    char* flags[512];
    int   np = 0, nf = 0, n = argc;
    pos[np++] = argv[0];
    for (int i = 1; i < n; i++) {
      const char* a = argv[i];
      if (a[0] == '-') {
        flags[nf++] = argv[i];
        if (strcmp(a, "--gain") == 0 || strcmp(a, "-g") == 0 ||
            strcmp(a, "-s") == 0 || strcmp(a, "-o") == 0 ||
            strcmp(a, "-c") == 0 || strcmp(a, "-v") == 0) {
          if (i + 1 < n)
            flags[nf++] = argv[++i];
        }
        continue;
      }
      pos[np++] = argv[i];
    }
    int k = 0;
    for (int i = 0; i < np; i++)
      argv[k++] = pos[i];
    for (int i = 0; i < nf; i++)
      argv[k++] = flags[i];
    argc = k;
  }

  const char* cmd = argc > 1 ? argv[1] : "";

  if (strcmp(cmd, "list") == 0)
    return cmd_list(argc, argv);
  if (strcmp(cmd, "play") == 0)
    return cmd_play(argc, argv);
  if (strcmp(cmd, "tui") == 0)
    return cmd_tui(argc, argv);
  if (strcmp(cmd, "play-bgm") == 0)
    return cmd_play_bgm(argc, argv);
  if (strcmp(cmd, "play-xa") == 0)
    return cmd_play_xa(argc, argv);
  if (strcmp(cmd, "play-vag") == 0)
    return cmd_play_vag(argc, argv);
  if (strcmp(cmd, "render") == 0)
    return cmd_render(argc, argv);
  if (strcmp(cmd, "xa-decode") == 0)
    return cmd_xa_decode(argc, argv);
  if (strcmp(cmd, "xa-inspect") == 0)
    return cmd_xa_inspect(argc, argv);
  if (strcmp(cmd, "vab-extract") == 0)
    return cmd_vab_extract(argc, argv);
  if (strcmp(cmd, "vab-inspect") == 0)
    return cmd_vab_inspect(argc, argv);
  if (strcmp(cmd, "bgm-audit") == 0)
    return cmd_bgm_audit(argc, argv);
  if (strcmp(cmd, "sep-inspect") == 0)
    return cmd_sep_inspect(argc, argv);
  if (strcmp(cmd, "sep2mid") == 0)
    return cmd_sep2mid(argc, argv);
  if (strcmp(cmd, "emi-inspect") == 0)
    return cmd_emi_inspect(argc, argv);
  if (strcmp(cmd, "psf-inspect") == 0)
    return cmd_psf_inspect(argc, argv);
  if (strcmp(cmd, "psf-pack") == 0)
    return cmd_psf_pack(argc, argv);
  if (strcmp(cmd, "psf-run") == 0)
    return cmd_psf_run(argc, argv);
  if (strcmp(cmd, "vab2sf2") == 0)
    return cmd_vab2sf2(argc, argv);
  if (strcmp(cmd, "--examples") == 0) {
    examples();
    return 0;
  }
  if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0 ||
      strcmp(cmd, "help") == 0) {
    usage();
    return 0;
  }

  fprintf(stderr, "unknown command: %s\n\n", cmd);
  usage();
  return 1;
}
