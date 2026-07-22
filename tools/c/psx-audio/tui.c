#include <ctype.h>

#include "cli.h"
#include "psf.h"

#ifndef _WIN32
#include <termios.h>
#include <poll.h>
#include <unistd.h>
#endif

#define TUI_STATE_FILE "out/audio/.tui-state"

static const char* tui_fmt_names[] = {"wav", "ogg", "flac", NULL};
static int         tui_fmt_idx = 0;

static void tui_save_state(const char* track_name, float gain) {
  FILE* f;
  MKDIR("out");
  MKDIR("out/audio");
  f = fopen(TUI_STATE_FILE, "w");
  if (!f)
    return;
  fprintf(f, "%s\n%.2f\n%s\n", track_name, (double)gain,
          tui_fmt_names[tui_fmt_idx]);
  fclose(f);
}

static int tui_load_state(char* track_name, size_t sz, float* gain) {
  char  line[64];
  FILE* f = fopen(TUI_STATE_FILE, "r");
  if (!f)
    return -1;
  if (!fgets(track_name, (int)sz, f)) {
    fclose(f);
    return -1;
  }
  track_name[strcspn(track_name, "\r\n")] = '\0';
  if (!fgets(line, sizeof(line), f))
    *gain = 1.0f;
  else
    *gain = (float)atof(line);
  if (!fgets(line, sizeof(line), f))
    tui_fmt_idx = 0;
  else {
    line[strcspn(line, "\r\n")] = '\0';
    for (int i = 0; tui_fmt_names[i]; i++) {
      if (strcmp(tui_fmt_names[i], line) == 0) {
        tui_fmt_idx = i;
        break;
      }
    }
  }
  fclose(f);
  return 0;
}

typedef struct {
  int16_t*     pcm;
  int64_t      pos, total;
  int          channels, rate;
  float        gain;
  volatile int playing;
} TuiPlayback;

static TuiPlayback tui_pb;

static volatile int tui_render_ready;
static int16_t*     tui_render_result;
static int64_t      tui_render_frames;
static int          tui_render_rate;
static int          tui_load_gen;
static int          tui_pending_render;
static char         tui_render_msg[128];
static int          tui_render_msg_ticks;

#ifndef _WIN32

static pthread_t tui_render_thr;

typedef struct {
  uint8_t *vh, *vb, *sep;
  uint32_t vh_sz, vb_sz, sep_sz;
  int      seq_idx;
  int      gen;
} RenderJob;

static void* tui_render_worker(void* arg) {
  RenderJob*   job = (RenderJob*)arg;
  RenderOutput ro;

  if (render_bgm(job->sep, job->sep_sz, job->vh, job->vh_sz, job->vb,
                 job->vb_sz, job->seq_idx, 44100, &ro) == 0 &&
      job->gen == tui_load_gen) {
    tui_render_result = ro.pcm;
    tui_render_frames = ro.frames;
    tui_render_rate = ro.rate;
    tui_render_ready = 1;
  } else if (ro.pcm) {
    free(ro.pcm);
  }

  free(job->vh);
  free(job->vb);
  free(job->sep);
  free(job);
  return NULL;
}

static void tui_async_start(const AudioSource* s, int seq) {
  tui_load_gen++;
  RenderJob* job = malloc(sizeof(RenderJob));
  if (!job)
    return;
  job->vh = malloc(s->vh_sz);
  if (job->vh)
    memcpy(job->vh, s->vh, s->vh_sz);
  job->vb = malloc(s->vb_sz);
  if (job->vb)
    memcpy(job->vb, s->vb, s->vb_sz);
  job->sep = s->sep_sz ? malloc(s->sep_sz) : NULL;
  if (job->sep && s->sep)
    memcpy(job->sep, s->sep, s->sep_sz);
  job->vh_sz = s->vh_sz;
  job->vb_sz = s->vb_sz;
  job->sep_sz = s->sep_sz;
  job->seq_idx = seq;
  job->gen = tui_load_gen;

  tui_render_ready = 0;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&tui_render_thr, &attr, tui_render_worker, job);
  pthread_attr_destroy(&attr);
}

#else

static void tui_async_start(const AudioSource* s, int seq) {
  (void)s;
  (void)seq;
}

#endif

static void tui_apply_async(void) {
  if (!tui_render_ready)
    return;
  tui_render_ready = 0;
  if (tui_render_result) {
    free(tui_pb.pcm);
    tui_pb.pcm = tui_render_result;
    tui_pb.total = tui_render_frames * 2;
    tui_pb.channels = 2;
    tui_pb.rate = tui_render_rate;
    tui_pb.pos = 0;
    tui_pb.playing = 1;
    tui_render_result = NULL;
  }
}

static void tui_data_callback(ma_device* dev, void* out, const void* in,
                              ma_uint32 frames) {
  int16_t* dst = (int16_t*)out;
  int      needed = (int)(frames * (ma_uint32)tui_pb.channels);
  int      avail, n;
  (void)dev;
  (void)in;

  if (!tui_pb.playing || !tui_pb.pcm) {
    memset(dst, 0, (size_t)needed * sizeof(int16_t));
    return;
  }
  avail = (int)(tui_pb.total - tui_pb.pos);
  n = needed < avail ? needed : avail;
  if (n > 0) {
    if (tui_pb.gain != 1.0f) {
      for (int i = 0; i < n; i++) {
        float v = (float)tui_pb.pcm[tui_pb.pos + i] * tui_pb.gain;
        dst[i] = (int16_t)(v > 32767.0f ? 32767 : v < -32768.0f ? -32768 : v);
      }
    } else {
      memcpy(dst, tui_pb.pcm + tui_pb.pos, (size_t)n * sizeof(int16_t));
    }
    tui_pb.pos += n;
  }
  if (n < needed)
    memset(dst + n, 0, (size_t)(needed - n) * sizeof(int16_t));
  if (tui_pb.pos >= tui_pb.total)
    tui_pb.playing = 0;
}

#ifdef _WIN32

static HANDLE tui_hin, tui_hout;
static DWORD  tui_orig_in_mode, tui_orig_out_mode;

static void tui_raw_mode(void) {
  tui_hin = GetStdHandle(STD_INPUT_HANDLE);
  tui_hout = GetStdHandle(STD_OUTPUT_HANDLE);
  GetConsoleMode(tui_hin, &tui_orig_in_mode);
  GetConsoleMode(tui_hout, &tui_orig_out_mode);
  SetConsoleMode(tui_hin, ENABLE_WINDOW_INPUT);
  SetConsoleMode(tui_hout,
                 tui_orig_out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

static void tui_restore_mode(void) {
  SetConsoleMode(tui_hin, tui_orig_in_mode);
  SetConsoleMode(tui_hout, tui_orig_out_mode);
}

static int tui_read_key(void) {
  DWORD wait = WaitForSingleObject(tui_hin, 0);
  if (wait != WAIT_OBJECT_0)
    return -1;

  INPUT_RECORD rec;
  DWORD        nread;
  if (!ReadConsoleInputA(tui_hin, &rec, 1, &nread) || nread == 0)
    return -1;
  if (rec.EventType != KEY_EVENT || !rec.Event.KeyEvent.bKeyDown)
    return -1;

  switch (rec.Event.KeyEvent.wVirtualKeyCode) {
    case VK_UP:
      return 'k';
    case VK_DOWN:
      return 'j';
    case VK_RIGHT:
      return 'l';
    case VK_LEFT:
      return 'h';
  }
  char c = rec.Event.KeyEvent.uChar.AsciiChar;
  if (c == 0x1b)
    return 'q';
  return c ? (unsigned char)c : -1;
}

#else

static struct termios tui_orig_term;
static int            tui_pb_char = -1;

static void tui_raw_mode(void) {
  struct termios raw;
  tcgetattr(STDIN_FILENO, &tui_orig_term);
  raw = tui_orig_term;
  raw.c_lflag &= ~(tcflag_t)(ECHO | ICANON);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void tui_restore_mode(void) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tui_orig_term);
}

static int tui_read_key(void) {
  struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
  unsigned char c;
  if (tui_pb_char >= 0) {
    c = tui_pb_char;
    tui_pb_char = -1;
    return c;
  }
  if (poll(&pfd, 1, 0) <= 0)
    return -1;
  if (read(STDIN_FILENO, &c, 1) != 1)
    return -1;
  if (c == 27) {
    unsigned char seq[2];
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return 27;
    if (seq[0] == '[') {
      if (read(STDIN_FILENO, &seq[1], 1) != 1)
        return 27;
      switch (seq[1]) {
        case 'A':
          return 'k';
        case 'B':
          return 'j';
        case 'C':
          return 'l';
        case 'D':
          return 'h';
      }
      return 27;
    }
    tui_pb_char = seq[0];
    return 27;
  }
  return c;
}

#endif

static char tui_query[64];
static int  tui_search_active;

static int match_substring(const char* name, const char* query) {
  if (!*query)
    return 1;
  while (*name) {
    const char *a = name, *b = query;
    while (*a && *b && tolower(*a) == tolower(*b)) {
      a++;
      b++;
    }
    if (!*b)
      return 1;
    name++;
  }
  return 0;
}

static int next_match(int cur, int count, const TrackInfo* tracks,
                      const char* query, int dir) {
  if (!*query)
    return (cur + dir + count) % count;
  int i = (cur + dir + count) % count;
  while (i != cur) {
    if (match_substring(tracks[i].name, query))
      return i;
    i = (i + dir + count) % count;
  }
  return cur;
}

static int jump_to_first_match(int cur, int count, const TrackInfo* tracks,
                               const char* query) {
  if (match_substring(tracks[cur].name, query))
    return cur;
  return next_match(cur, count, tracks, query, 1);
}

static void tui_draw(const TrackInfo* tracks, int count, int cur, int scroll) {
  double      elapsed = 0, duration = 0;
  int         bar_len = 30, filled;
  int         vis_rows = 12;
  const char* status;

  if (tui_pb.rate > 0 && tui_pb.channels > 0) {
    elapsed = (double)(tui_pb.pos / tui_pb.channels) / tui_pb.rate;
    duration = (double)(tui_pb.total / tui_pb.channels) / tui_pb.rate;
  }
  filled = duration > 0 ? (int)(bar_len * elapsed / duration) : 0;
  if (filled > bar_len)
    filled = bar_len;

  if (!tui_render_ready && tui_load_gen > 0 && !tui_pb.pcm)
    status = "\xe2\x8f\xb3 loading...";
  else
    status = tui_pb.playing ? "\xe2\x96\xb6 playing"
                            : (tui_pb.pos > 0 ? "\xe2\x8f\xb8 paused"
                                              : "\xe2\x8f\xb9 stopped");

  printf("\033[2J\033[H");
  printf("  \033[1mBOF3 BGM Player\033[0m  (%d tracks)\n\n", count);

  for (int i = scroll; i < count && i < scroll + vis_rows; i++) {
    if (i == cur)
      printf("  \033[7m %3d  %-20s %5d ev  %3d tones \033[0m\n", i,
             tracks[i].name, tracks[i].events, tracks[i].tones);
    else
      printf("   %3d  %-20s %5d ev  %3d tones\n", i, tracks[i].name,
             tracks[i].events, tracks[i].tones);
  }
  if (count > vis_rows)
    printf("  ... (%d-%d of %d)\n", scroll,
           scroll + vis_rows - 1 < count ? scroll + vis_rows - 1 : count - 1,
           count);

  printf("\n  \033[1m%-20s\033[0m  %s  %s\n", tracks[cur].name, status,
         tui_pb.pos >= tui_pb.total && tui_pb.total > 0 ? "[end]" : "");
  printf("  [%.*s%.*s] %5.1f / %.1fs\n", filled,
         "================================", bar_len - filled,
         "--------------------------------", elapsed, duration);
  printf("  gain: %.2f  fmt: %s\n", tui_pb.gain, tui_fmt_names[tui_fmt_idx]);
  if (tui_search_active) {
    printf("  \033[7m/: %s\xe2\x96\x88\033[0m\n", tui_query);
    printf("  \033[2mESC:cancel  Enter:accept\033[0m\n");
  } else {
    printf("  \033[2m/:search\033[0m\n");
  }
  if (tui_render_msg_ticks > 0) {
    printf("  %s\n", tui_render_msg);
    tui_render_msg_ticks--;
  }
  printf(
      "\n  \033[2mj/k:\xe2\x86\x91\xe2\x86\x93  space:play/pause  s:stop  "
      "n/p:next/prev  +/-:gain  f:fmt  r:render  q:quit\033[0m\n");
  fflush(stdout);
}

int cmd_tui(int argc, char** argv) {
  char             dir[512];
  TrackInfo*       tracks;
  int              max = 256, count, cur = 0, scroll = 0;
  float            gain = arg_gain(argc, argv);
  ma_device_config cfg;
  ma_device        dev;
  AudioSource      src;
  int              have_src = 0;
  int              running = 1;
  int              vis_rows = 12;

  if (find_bgm_dir(dir, sizeof(dir)) != 0) {
    fprintf(stderr, "error: out/extracted/BIN/BGM not found\n");
    return 1;
  }
  tracks = calloc(max, sizeof(TrackInfo));
  if (!tracks)
    return 1;
  count = scan_tracks(dir, tracks, max);
  if (count == 0) {
    free(tracks);
    fprintf(stderr, "error: no tracks found\n");
    return 1;
  }

  memset(&tui_pb, 0, sizeof(tui_pb));
  tui_pb.gain = gain;

  {
    char  last_name[64];
    float last_gain;
    if (tui_load_state(last_name, sizeof(last_name), &last_gain) == 0) {
      for (int i = 0; i < count; i++) {
        if (strcmp(tracks[i].name, last_name) == 0) {
          cur = i;
          break;
        }
      }
      tui_pb.gain = last_gain;
    }
    if (arg_has(argc, argv, "--gain") || arg_has(argc, argv, "-g"))
      tui_pb.gain = arg_gain(argc, argv);
  }

  tui_pending_render = 0;
  tui_search_active = 0;
  tui_query[0] = 0;
  tui_render_msg[0] = 0;
  tui_render_msg_ticks = 0;
  tui_pb_char = -1;

  cfg = ma_device_config_init(ma_device_type_playback);
  cfg.playback.format = ma_format_s16;
  cfg.playback.channels = 2;
  cfg.sampleRate = 44100;
  cfg.dataCallback = tui_data_callback;
  if (ma_device_init(NULL, &cfg, &dev) != MA_SUCCESS) {
    free(tracks);
    fprintf(stderr, "error: audio device init failed\n");
    return 1;
  }
  ma_device_start(&dev);
  tui_raw_mode();
  tui_draw(tracks, count, cur, scroll);

  while (running) {
    int key = tui_read_key();
    int need_load = 0;

    if (key >= 0) {
      if (tui_search_active) {
        if (key == 27) {
          tui_search_active = 0;
          tui_query[0] = 0;
        } else if (key == '\r' || key == '\n') {
          tui_search_active = 0;
        } else if (key == 127 || key == '\b') {
          int len = (int)strlen(tui_query);
          if (len > 0) {
            tui_query[len - 1] = 0;
          }
        } else if (key >= 32 && key < 127) {
          int len = (int)strlen(tui_query);
          if (len < 63) {
            tui_query[len] = key;
            tui_query[len + 1] = 0;
          }
        }
        if (!tui_search_active && tui_query[0]) {
          cur = jump_to_first_match(0, count, tracks, tui_query);
          need_load = 1;
        }
        if (tui_search_active && tui_query[0]) {
          cur = jump_to_first_match(cur, count, tracks, tui_query);
          need_load = 1;
        }
      } else {
        switch (key) {
          case 'q':
          case 3:
            tui_save_state(tracks[cur].name, tui_pb.gain);
            running = 0;
            break;
          case '/':
            tui_search_active = 1;
            tui_query[0] = 0;
            break;
          case 'j':
          case 'n':
            if (tui_query[0]) {
              cur = next_match(cur, count, tracks, tui_query, 1);
            } else {
              cur = (cur + 1) % count;
            }
            need_load = 1;
            break;
          case 'k':
          case 'p':
            if (tui_query[0]) {
              cur = next_match(cur, count, tracks, tui_query, -1);
            } else {
              cur = (cur - 1 + count) % count;
            }
            need_load = 1;
            break;
          case ' ':
            if (tui_pb.playing) {
              tui_pb.playing = 0;
            } else if (tui_pb.pcm && tui_pb.pos < tui_pb.total) {
              tui_pb.playing = 1;
            } else {
              need_load = 1;
            }
            break;
          case 's':
            tui_pb.playing = 0;
            tui_pb.pos = 0;
            break;
          case '+':
          case '=':
            tui_pb.gain += 0.1f;
            if (tui_pb.gain > 3.0f)
              tui_pb.gain = 3.0f;
            break;
          case '-':
          case '_':
            tui_pb.gain -= 0.1f;
            if (tui_pb.gain < 0.0f)
              tui_pb.gain = 0.0f;
            break;
          case 'f':
          case 'F':
            tui_fmt_idx = (tui_fmt_idx + 1) % 3;
            break;
          case 'r':
          case 'R':
            if (have_src) {
              if (tui_pb.pcm) {
                char outpath[512];
                snprintf(outpath, sizeof(outpath), "%s.%s", tracks[cur].name,
                         tui_fmt_names[tui_fmt_idx]);
                write_stereo_output(outpath, tui_pb.pcm, tui_pb.total / 2,
                                    tui_pb.rate);
                snprintf(tui_render_msg, sizeof(tui_render_msg),
                         "wrote %s (%.1fs)", outpath,
                         (double)(tui_pb.total / 2) / tui_pb.rate);
                tui_render_msg_ticks = 120;
              } else {
                tui_pending_render = 1;
              }
            }
            break;
          case '\r':
          case '\n':
            need_load = 1;
            break;
        }
      }
    }

    if (need_load) {
      tui_pb.playing = 0;
      tui_pending_render = 0;
      if (have_src) {
        source_free(&src);
        have_src = 0;
      }
      if (source_from_emi(&src, tracks[cur].path) == 0) {
        have_src = 1;
        tui_async_start(&src, 0);
      }
    }

    tui_apply_async();

    if (tui_pending_render && tui_pb.pcm) {
      tui_pending_render = 0;
      char outpath[512];
      snprintf(outpath, sizeof(outpath), "%s.%s", tracks[cur].name,
               tui_fmt_names[tui_fmt_idx]);
      write_stereo_output(outpath, tui_pb.pcm, tui_pb.total / 2, tui_pb.rate);
      snprintf(tui_render_msg, sizeof(tui_render_msg), "wrote %s (%.1fs)",
               outpath, (double)(tui_pb.total / 2) / tui_pb.rate);
      tui_render_msg_ticks = 120;
    }

    if (cur < scroll)
      scroll = cur;
    if (cur >= scroll + vis_rows)
      scroll = cur - vis_rows + 1;

    tui_draw(tracks, count, cur, scroll);
    usleep(16000);
  }

  tui_restore_mode();
  printf("\033[2J\033[H");
  ma_device_uninit(&dev);
  if (have_src)
    source_free(&src);
  free(tui_pb.pcm);
  free(tracks);
  return 0;
}
