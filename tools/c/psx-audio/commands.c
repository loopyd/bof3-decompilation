#include "cli.h"
#include "psf.h"
#include "psx_machine.h"

int cmd_list(int argc, char** argv) {
  char        dir[512];
  const char* filter = argc > 2 ? argv[2] : NULL;
  TrackInfo*  tracks;
  int         max = 256;

  if (find_bgm_dir(dir, sizeof(dir)) != 0) {
    fprintf(stderr, "error: out/extracted/BIN/BGM not found\n");
    return 1;
  }
  tracks = calloc(max, sizeof(TrackInfo));
  if (!tracks)
    return 1;
  int count = scan_tracks(dir, tracks, max);

  printf("\n  BGM Tracks (%d)\n", count);
  printf("  %.*s\n", 62,
         "──────────────────────────────────────────────────────────────");
  printf("  %4s  %-20s %7s %6s\n", "#", "Name", "Events", "Tones");
  printf("  %4s  %-20s %7s %6s\n", "───", "────────────────────", "──────",
         "─────");

  int shown = 0;
  for (int i = 0; i < count; i++) {
    if (filter) {
      char un[64], uf[64];
      strncpy(un, tracks[i].name, 63);
      un[63] = '\0';
      strncpy(uf, filter, 63);
      uf[63] = '\0';
      for (char* p = un; *p; p++)
        if (*p >= 'a' && *p <= 'z')
          *p -= 32;
      for (char* p = uf; *p; p++)
        if (*p >= 'a' && *p <= 'z')
          *p -= 32;
      if (!strstr(un, uf))
        continue;
    }
    printf("  %4d  %-20s %6d  %5d\n", shown, tracks[i].name, tracks[i].events,
           tracks[i].tones);
    shown++;
  }
  printf("  %.*s\n", 62,
         "──────────────────────────────────────────────────────────────");
  printf("  bin/psx-audio play <name>    e.g. bin/psx-audio play BGM000\n\n");
  free(tracks);
  return 0;
}

int cmd_emi_inspect(int argc, char** argv) {
  uint8_t* data;
  size_t   len;
  EmiFile  emi;
  data = read_file(argv[2], &len);
  if (!data) {
    fprintf(stderr, "error: read failed\n");
    return 1;
  }
  if (emi_parse(data, len, &emi) != 0) {
    fprintf(stderr, "error: not an EMI file\n");
    free(data);
    return 1;
  }
  printf("  %s: %d entries\n", argv[2], emi.count);
  for (int i = 0; i < emi.count; i++)
    printf("    [%d] type=%2d %-12s size=%-8u offset=0x%X\n", i,
           emi.entries[i].type, emi_type_name(emi.entries[i].type),
           emi.entries[i].size, emi.entries[i].offset);
  free(data);
  return 0;
}

int cmd_psf_inspect(int argc, char** argv) {
  Psf1Image  image;
  Psf1Status status;

  if (argc < 3) {
    fprintf(stderr, "usage: psf-inspect <file.psf>\n");
    return 1;
  }
  status = psf1_load_file(argv[2], &image);
  if (status != PSF1_OK) {
    fprintf(stderr, "error: %s\n", psf1_status_string(status));
    return 1;
  }
  printf("  %s\n", argv[2]);
  printf("    PC:      0x%08X\n", image.initial_pc);
  printf("    SP:      0x%08X\n", image.initial_sp);
  printf("    RAM:     0x%05X-0x%05X\n", image.loaded_min, image.loaded_max);
  printf("    refresh: %dHz\n", image.refresh_rate);
  psf1_image_free(&image);
  return 0;
}

int cmd_psf_pack(int argc, char** argv) {
  const char* outpath = arg_str(argc, argv, "-o");
  uint8_t*    exe;
  size_t      exe_size;
  Psf1Status  status;

  if (argc < 3 || !outpath) {
    fprintf(stderr, "usage: psf-pack <PS-X EXE> -o <file.psflib>\n");
    return 1;
  }
  exe = read_file(argv[2], &exe_size);
  if (!exe) {
    fprintf(stderr, "error: cannot read %s\n", argv[2]);
    return 1;
  }
  status = psf1_write_file(outpath, exe, exe_size, NULL);
  free(exe);
  if (status != PSF1_OK) {
    fprintf(stderr, "error: %s\n", psf1_status_string(status));
    return 1;
  }
  printf("  wrote %s\n", outpath);
  return 0;
}

int cmd_psf_run(int argc, char** argv) {
  int              instructions = arg_int(argc, argv, "-n", 100000);
  const char*      call_value = arg_str(argc, argv, "--call");
  Psf1Image        image;
  Psf1Status       image_status;
  PsxSpu*          spu;
  PsxMachine*      machine;
  PsxMachineStatus machine_status;

  if (argc < 3 || instructions < 0) {
    fprintf(stderr, "usage: psf-run <file.psf> [-n INSTRUCTIONS]\n");
    return 1;
  }
  image_status = psf1_load_file(argv[2], &image);
  if (image_status != PSF1_OK) {
    fprintf(stderr, "error: %s\n", psf1_status_string(image_status));
    return 1;
  }
  spu = psx_spu_create();
  machine = psx_machine_create(&image, spu);
  psf1_image_free(&image);
  if (!spu || !machine) {
    psx_machine_destroy(machine);
    psx_spu_destroy(spu);
    fprintf(stderr, "error: cannot allocate PSX machine\n");
    return 1;
  }
  if (call_value) {
    uint32_t arguments[4] = {0, 0, 0, 0};
    uint32_t address = (uint32_t)strtoul(call_value, NULL, 0);
    machine_status =
        psx_machine_call(machine, address, arguments, (uint64_t)instructions);
  } else {
    machine_status = psx_machine_run(machine, (uint64_t)instructions);
  }
  printf("  cycles:     %llu\n",
         (unsigned long long)psx_machine_cycles(machine));
  printf("  PC:         0x%08X\n", psx_machine_pc(machine));
  printf("  SPU writes: %zu\n", psx_spu_write_count(spu));
  if (machine_status != PSX_MACHINE_OK) {
    const PsxMachineFault* fault = psx_machine_fault(machine);
    fprintf(stderr,
            "error: %s at PC=0x%08X instruction=0x%08X address=0x%08X\n",
            psx_machine_status_string(machine_status), fault->pc,
            fault->instruction, fault->address);
  }
  psx_machine_destroy(machine);
  psx_spu_destroy(spu);
  return machine_status == PSX_MACHINE_OK ? 0 : 1;
}

int cmd_vab_inspect(int argc, char** argv) {
  uint8_t* data;
  size_t   len;
  VabHdr   hdr;
  data = read_file(argv[2], &len);
  if (!data) {
    fprintf(stderr, "error: read failed\n");
    return 1;
  }
  if (vab_parse_vh(data, len, &hdr) != 0) {
    fprintf(stderr, "error: bad VH\n");
    free(data);
    return 1;
  }
  printf("  %s: programs=%u tones=%u vags=%u file=%u bytes\n", argv[2],
         hdr.program_count, hdr.tone_count, hdr.vag_count, hdr.file_size);
  for (int i = 0; i < (int)hdr.tone_count; i++) {
    VagAtr* t = &hdr.tones[i];
    printf(
        "    [%2d] prog=%d block=%d/%d note=%d-%d center=%d shift=%d "
        "bend=%d/%d "
        "vib=%d/%d por=%d/%d mode=%02X vag=%u+%u adsr=%04X/%04X\n",
        i, t->prog, t->storage_block, t->tone_slot, t->min_note, t->max_note,
        t->center_note, t->shift, t->pitch_bend_min, t->pitch_bend_max,
        t->vibrato_width, t->vibrato_time, t->portamento_width,
        t->portamento_time, t->mode, t->vag_offset, t->vag_size, t->adsr1,
        t->adsr2);
  }
  free(data);
  return 0;
}

int cmd_sep_inspect(int argc, char** argv) {
  uint8_t* data;
  size_t   len;
  SepFile  sep;
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
  printf("  %s: %d sequence(s)\n", argv[2], sep.sequence_count);
  for (int i = 0; i < sep.sequence_count; i++) {
    printf("    [%d] res=%d events=%d\n", i, sep.sequences[i].resolution,
           sep.sequences[i].event_count);
    if (arg_has(argc, argv, "--programs")) {
      int programs[16] = {0};
      int note_count[128] = {0};
      int note_histogram[128][128] = {{0}};
      int min_note[128];
      int max_note[128];
      int event_index;
      int program;

      for (program = 0; program < 128; program++) {
        min_note[program] = 128;
        max_note[program] = -1;
      }
      for (event_index = 0; event_index < sep.sequences[i].event_count;
           event_index++) {
        SepEvent* event = &sep.sequences[i].events[event_index];
        int       channel = event->type & 0x0f;
        if ((event->type & 0xf0) == 0xc0) {
          programs[channel] = event->data1;
        } else if ((event->type & 0xf0) == 0x90 && event->data2 != 0) {
          program = programs[channel];
          note_count[program]++;
          note_histogram[program][event->data1]++;
          if (event->data1 < min_note[program])
            min_note[program] = event->data1;
          if (event->data1 > max_note[program])
            max_note[program] = event->data1;
        }
      }
      for (program = 0; program < 128; program++)
        if (note_count[program] != 0) {
          printf("      program=%d notes=%d range=%d-%d\n", program,
                 note_count[program], min_note[program], max_note[program]);
          if (arg_has(argc, argv, "--notes")) {
            int note;
            printf("        note-counts:");
            for (note = 0; note < 128; note++)
              if (note_histogram[program][note] != 0)
                printf(" %d:%d", note, note_histogram[program][note]);
            printf("\n");
          }
        }
    }
    if (arg_has(argc, argv, "--bends")) {
      int bend_count[16] = {0};
      int min_data1[16], max_data1[16];
      int min_data2[16], max_data2[16];
      int event_index;
      int channel;

      for (channel = 0; channel < 16; channel++) {
        min_data1[channel] = min_data2[channel] = 128;
        max_data1[channel] = max_data2[channel] = -1;
      }
      for (event_index = 0; event_index < sep.sequences[i].event_count;
           event_index++) {
        SepEvent* event = &sep.sequences[i].events[event_index];
        if ((event->type & 0xf0) != 0xe0)
          continue;
        channel = event->type & 0x0f;
        bend_count[channel]++;
        if (event->data1 < min_data1[channel])
          min_data1[channel] = event->data1;
        if (event->data1 > max_data1[channel])
          max_data1[channel] = event->data1;
        if (event->data2 < min_data2[channel])
          min_data2[channel] = event->data2;
        if (event->data2 > max_data2[channel])
          max_data2[channel] = event->data2;
      }
      for (channel = 0; channel < 16; channel++)
        if (bend_count[channel] != 0)
          printf("      bend ch=%d events=%d data1=%d-%d data2=%d-%d\n",
                 channel, bend_count[channel], min_data1[channel],
                 max_data1[channel], min_data2[channel], max_data2[channel]);
      if (arg_has(argc, argv, "--events")) {
        uint32_t tick = 0;
        for (event_index = 0; event_index < sep.sequences[i].event_count;
             event_index++) {
          SepEvent* event = &sep.sequences[i].events[event_index];
          tick += event->delta;
          if ((event->type & 0xf0) == 0xe0)
            printf("        bend-event tick=%u ch=%d data1=%u data2=%u\n", tick,
                   event->type & 0x0f, event->data1, event->data2);
        }
      }
    }
    if (arg_has(argc, argv, "--controls")) {
      int controls[128][128] = {{0}};
      int event_index;
      int control;
      int value;

      for (event_index = 0; event_index < sep.sequences[i].event_count;
           event_index++) {
        SepEvent* event = &sep.sequences[i].events[event_index];
        if ((event->type & 0xf0) == 0xb0)
          controls[event->data1][event->data2]++;
      }
      for (control = 0; control < 128; control++) {
        int count = 0;
        for (value = 0; value < 128; value++)
          count += controls[control][value];
        if (count == 0)
          continue;
        printf("      control=%d events=%d values:", control, count);
        for (value = 0; value < 128; value++)
          if (controls[control][value] != 0)
            printf(" %d:%d", value, controls[control][value]);
        printf("\n");
      }
    }
  }
  sep_free(&sep);
  free(data);
  return 0;
}

int cmd_bgm_audit(int argc, char** argv) {
  AudioSource      source;
  AudioAuditReport report;
  int              program, note;

  if (argc < 3) {
    fprintf(stderr, "usage: bgm-audit <track|EMI|directory>\n");
    return 1;
  }
  memset(&source, 0, sizeof(source));
  if (source_auto(&source, argv[2]) != 0 || !source.sep) {
    fprintf(stderr, "%s: error=missing-or-invalid-EMI/VH/VB/SEP\n", argv[2]);
    source_free(&source);
    return 1;
  }
  if (audio_audit_bgm(source.vh, source.vh_sz, source.vb, source.vb_sz,
                      source.sep, source.sep_sz, &report) != 0) {
    fprintf(stderr, "%s: error=invalid-VH/VB/SEP\n", argv[2]);
    source_free(&source);
    return 1;
  }
  printf(
      "%s: vh=%u+%u/%u programs=%u tones=%u vags=%u seq=%d "
      "remap=%d missing-note-events=%d layered-note-events=%d "
      "bad-vag=%d bad-prefix=%d missing-end=%d reverb-tones=%d "
      "modulation-tones=%d bend-lsb-events=%d ignored-controls=%d "
      "loop-controls=%d\n",
      argv[2], report.vh_size, report.vb_size, report.declared_file_size,
      report.program_count, report.tone_count, report.vag_count,
      report.sequence_count, report.remapped_tones, report.missing_note_events,
      report.layered_note_events, report.bad_vag_ranges,
      report.bad_sample_prefixes, report.samples_without_end,
      report.reverb_tones, report.modulation_tones, report.bend_lsb_events,
      report.ignored_control_events, report.loop_control_events);
  if (arg_has(argc, argv, "--details") && report.missing_note_events) {
    printf("  missing:");
    for (program = 0; program < 128; program++)
      for (note = 0; note < 128; note++)
        if (report.missing_notes[program][note])
          printf(" p%d/n%d:%d", program, note,
                 report.missing_notes[program][note]);
    printf("\n");
  }

  source_free(&source);
  return report.bad_vag_ranges || report.bad_sample_prefixes ||
                 report.samples_without_end ||
                 report.declared_file_size != report.vh_size + report.vb_size
             ? 1
             : 0;
}
