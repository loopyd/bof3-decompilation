#include "cli.h"
#include "psf.h"

int cmd_play_bgm(int argc, char** argv) {
  AudioSource s;
  int         seq = arg_int(argc, argv, "-s", -1);
  float       gain = arg_gain(argc, argv);
  const char* outpath = arg_str(argc, argv, "-o");
  char        path[512];
  const char* target = argv[2];
  int         rc;

  if (ends_with(target, ".emi") || ends_with(target, ".EMI")) {
    printf("  %s\n", target);
    if (source_from_emi(&s, target) != 0) {
      fprintf(stderr, "error: no audio in %s\n", target);
      return 1;
    }
  } else if (argc >= 5 && ends_with(target, ".bin")) {
    if (source_from_raw(&s, argv[2], argv[3], argv[4]) != 0) {
      fprintf(stderr, "error: failed to read files\n");
      return 1;
    }
    printf("  %s + %s + %s\n", argv[2], argv[3], argv[4]);
  } else {
    if (find_track_path(target, path, sizeof(path)) != 0) {
      fprintf(stderr, "error: track '%s' not found (try: bin/psx-audio list)\n",
              target);
      return 1;
    }
    printf("  %s\n", path);
    if (source_auto(&s, path) != 0) {
      fprintf(stderr, "error: no audio in %s\n", path);
      return 1;
    }
  }

  rc = play_source(&s, seq, gain, outpath);
  source_free(&s);
  return rc != 0 ? 1 : 0;
}

int cmd_play_xa(int argc, char** argv) {
  uint8_t*    data;
  size_t      len;
  int16_t*    pcm = NULL;
  int         rate, nch;
  int         channel = arg_int(argc, argv, "-c", 0);
  float       gain = arg_gain(argc, argv);
  const char* outpath = arg_str(argc, argv, "-o");

  data = read_file(argv[2], &len);
  if (!data) {
    fprintf(stderr, "error: read failed\n");
    return 1;
  }
  int64_t frames = xa_decode_channel(data, len, channel, &pcm, &rate, &nch);
  free(data);
  if (frames <= 0) {
    fprintf(stderr, "error: decode failed (ch %d)\n", channel);
    return 1;
  }
  if (outpath) {
    if (nch == 1)
      wav_write_mono(outpath, pcm, frames, rate);
    else
      wav_write_stereo(outpath, pcm, frames, rate);
    printf("  wrote %s (%.1fs, %dHz, %s)\n", outpath, (double)frames / rate,
           rate, nch == 2 ? "stereo" : "mono");
  } else {
    printf("  %s — ch %d\n", argv[2], channel);
    play_buffer(pcm, frames, nch, rate, gain);
  }
  free(pcm);
  return 0;
}

int cmd_play_vag(int argc, char** argv) {
  AudioSource s;
  VabHdr      hdr;
  int         vag = arg_int(argc, argv, "-v", -1);
  float       gain = arg_gain(argc, argv);

  if (source_from_raw(&s, argv[2], argv[3], NULL) != 0) {
    fprintf(stderr, "error: read failed\n");
    return 1;
  }
  if (vab_parse_vh(s.vh, s.vh_sz, &hdr) != 0) {
    fprintf(stderr, "error: bad VH\n");
    source_free(&s);
    return 1;
  }

  if (vag >= 0) {
    int16_t*    pcm = NULL;
    const char* outpath = arg_str(argc, argv, "-o");
    int         n = vab_decode_vag(s.vb, s.vb_sz, &hdr, vag, &pcm);
    if (n > 0 && pcm) {
      if (outpath) {
        wav_write_mono(outpath, pcm, n, 44100);
        printf("  wrote %s (%d samples)\n", outpath, n);
      } else {
        printf("  VAG %d (%d samples)\n", vag, n);
        play_buffer(pcm, n, 1, 44100, gain);
      }
      free(pcm);
    }
  } else {
    for (int i = 0; i < (int)hdr.tone_count; i++) {
      int16_t* pcm = NULL;
      int      n = vab_decode_vag(s.vb, s.vb_sz, &hdr, i, &pcm);
      if (n > 0 && pcm) {
        printf("  VAG %d/%d\n", i, (int)hdr.tone_count);
        play_buffer(pcm, n, 1, 44100, gain);
        free(pcm);
      }
    }
  }
  source_free(&s);
  return 0;
}

int cmd_play(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: play <track|file> [-s N] [-g GAIN]\n");
    return 1;
  }
  if (ends_with(argv[2], ".str") || ends_with(argv[2], ".STR"))
    return cmd_play_xa(argc, argv);
  return cmd_play_bgm(argc, argv);
}

int cmd_render(int argc, char** argv) {
  const char*  outpath = arg_str(argc, argv, "-o");
  int          seq = arg_int(argc, argv, "-s", -1);
  float        gain = arg_gain(argc, argv);
  AudioSource  s;
  RenderOutput ro;
  char         path[512];

  if (!outpath) {
    fprintf(stderr, "error: -o required\n");
    return 1;
  }

  if (argc >= 5 && ends_with(argv[2], ".bin") &&
      source_from_raw(&s, argv[2], argv[3], argv[4]) == 0) {
    /* raw files */
  } else if (source_auto(&s, argv[2]) == 0) {
    /* EMI or directory */
  } else if (find_track_path(argv[2], path, sizeof(path)) == 0 &&
             source_auto(&s, path) == 0) {
    /* resolved track name */
  } else {
    fprintf(stderr, "error: cannot load audio from %s\n", argv[2]);
    return 1;
  }

  if (!s.sep) {
    fprintf(stderr, "error: no sequence data\n");
    source_free(&s);
    return 1;
  }
  if (render_bgm(s.sep, s.sep_sz, s.vh, s.vh_sz, s.vb, s.vb_sz, seq, 44100,
                 &ro) != 0) {
    fprintf(stderr, "error: render failed\n");
    source_free(&s);
    return 1;
  }
  if (gain != 1.0f) {
    int64_t count = ro.frames * 2;
    for (int64_t i = 0; i < count; i++) {
      float value = (float)ro.pcm[i] * gain;
      ro.pcm[i] = (int16_t)(value > 32767.0f    ? 32767
                            : value < -32768.0f ? -32768
                                                : value);
    }
  }
  if (write_stereo_output(outpath, ro.pcm, ro.frames, ro.rate) != 0) {
    fprintf(stderr,
            "error: failed to write %s (compressed output requires its codec "
            "library)\n",
            outpath);
    free(ro.pcm);
    source_free(&s);
    return 1;
  }
  printf("  wrote %s (%.1fs, %dHz stereo)\n", outpath,
         (double)ro.frames / ro.rate, ro.rate);
  free(ro.pcm);
  source_free(&s);
  return 0;
}

int cmd_xa_inspect(int argc, char** argv) {
  uint8_t*     data;
  size_t       len;
  XaStreamInfo streams[32];
  data = read_file(argv[2], &len);
  if (!data) {
    fprintf(stderr, "error: read failed\n");
    return 1;
  }
  int count = xa_inspect(data, len, streams, 32);
  printf("  %s (%zu bytes)\n", argv[2], len);
  for (int i = 0; i < count; i++)
    printf("    ch %d: %dHz %s  %.1fs\n", i, streams[i].rate,
           streams[i].channels == 2 ? "stereo" : "mono  ",
           (double)streams[i].frame_count / streams[i].rate);
  free(data);
  return 0;
}

int cmd_xa_decode(int argc, char** argv) {
  const char* outpath = arg_str(argc, argv, "-o");
  int         channel = arg_int(argc, argv, "-c", 0);
  uint8_t*    data;
  size_t      len;
  int16_t*    pcm = NULL;
  int         rate, nch;

  if (!outpath) {
    fprintf(stderr, "error: -o required\n");
    return 1;
  }
  data = read_file(argv[2], &len);
  if (!data) {
    fprintf(stderr, "error: read failed\n");
    return 1;
  }
  int64_t frames = xa_decode_channel(data, len, channel, &pcm, &rate, &nch);
  free(data);
  if (frames <= 0) {
    fprintf(stderr, "error: decode failed\n");
    return 1;
  }
  if (nch == 1)
    wav_write_mono(outpath, pcm, frames, rate);
  else
    wav_write_stereo(outpath, pcm, frames, rate);
  printf("  wrote %s (%.1fs, %dHz, %s)\n", outpath, (double)frames / rate, rate,
         nch == 2 ? "stereo" : "mono");
  free(pcm);
  return 0;
}

int cmd_vab_extract(int argc, char** argv) {
  const char* outdir = arg_str(argc, argv, "-o");
  AudioSource s;
  VabHdr      hdr;
  if (!outdir) {
    fprintf(stderr, "error: -o required\n");
    return 1;
  }
  if (source_from_raw(&s, argv[2], argv[3], NULL) != 0) {
    fprintf(stderr, "error: read failed\n");
    return 1;
  }
  if (vab_parse_vh(s.vh, s.vh_sz, &hdr) != 0) {
    fprintf(stderr, "error: bad VH\n");
    source_free(&s);
    return 1;
  }
  MKDIR(outdir);
  int extracted = 0;
  for (int i = 0; i < (int)hdr.tone_count; i++) {
    int16_t* pcm = NULL;
    int      n = vab_decode_vag(s.vb, s.vb_sz, &hdr, i, &pcm);
    if (n > 0 && pcm) {
      char path[512];
      snprintf(path, sizeof(path), "%s/vag_%03d.wav", outdir, i);
      wav_write_mono(path, pcm, n, 44100);
      extracted++;
      free(pcm);
    }
  }
  printf("  extracted %d/%u tones to %s/\n", extracted, hdr.tone_count, outdir);
  source_free(&s);
  return 0;
}

int cmd_sep2mid(int argc, char** argv) {
  const char* outpath = arg_str(argc, argv, "-o");
  int         seq = arg_int(argc, argv, "-s", 0);
  uint8_t*    data;
  size_t      len;
  SepFile     sep;
  if (!outpath) {
    fprintf(stderr, "error: -o required\n");
    return 1;
  }
  data = read_file(argv[2], &len);
  if (!data) {
    fprintf(stderr, "error: read failed\n");
    return 1;
  }
  if (sep_parse(data, len, &sep) != 0) {
    fprintf(stderr, "error: bad SEP\n");
    free(data);
    return 1;
  }
  if (sep_to_midi(&sep, seq, outpath) != 0)
    fprintf(stderr, "error: export failed\n");
  else
    printf("  wrote %s (seq %d)\n", outpath, seq);
  sep_free(&sep);
  free(data);
  return 0;
}

int cmd_vab2sf2(int argc, char** argv) {
  const char* outpath = arg_str(argc, argv, "-o");
  const char* name = arg_str(argc, argv, "--name");
  AudioSource s;

  if (!outpath) {
    fprintf(stderr, "error: -o required\n");
    return 1;
  }
  if (!name)
    name = "BOF3";

  if (argc >= 4 && source_from_raw(&s, argv[2], argv[3], NULL) == 0) {
    /* raw VH + VB files */
  } else if (source_auto(&s, argv[2]) == 0) {
    /* EMI or directory */
  } else {
    fprintf(stderr, "error: cannot load VAB from %s\n", argv[2]);
    return 1;
  }

  if (vab_to_sf2(s.vh, s.vh_sz, s.vb, s.vb_sz, outpath, name) != 0) {
    fprintf(stderr, "error: SF2 export failed\n");
    source_free(&s);
    return 1;
  }
  printf("  wrote %s\n", outpath);
  source_free(&s);
  return 0;
}
