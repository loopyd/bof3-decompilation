#include <string.h>

#include "audio.h"

static int tone_matches(const VabHeader *vab, int program, int note)
{
    int i;
    int matches = 0;

    for (i = 0; i < vab->tone_count; i++) {
        const VabTone *tone = &vab->tones[i];
        if (tone->prog == program && note >= tone->min_note &&
            note <= tone->max_note)
            matches++;
    }
    return matches;
}

static void audit_samples(const uint8_t *vb, size_t vb_len,
                          const VabHeader *vab, AudioAuditReport *report)
{
    uint32_t seen_offsets[2048];
    int seen_count = 0;
    int i, j;

    for (i = 0; i < vab->tone_count; i++) {
        const VabTone *tone = &vab->tones[i];
        int already_seen = 0;
        uint32_t block;
        int has_end = 0;

        if (tone->prog != tone->storage_block)
            report->remapped_tones++;
        if (tone->mode & 4)
            report->reverb_tones++;
        if (tone->vibrato_width || tone->vibrato_time ||
            tone->portamento_width || tone->portamento_time)
            report->modulation_tones++;

        for (j = 0; j < seen_count; j++)
            if (seen_offsets[j] == tone->vag_offset)
                already_seen = 1;
        if (already_seen)
            continue;
        seen_offsets[seen_count++] = tone->vag_offset;

        if (tone->vag_size < 16 || tone->vag_offset > vb_len ||
            tone->vag_size > vb_len - tone->vag_offset) {
            report->bad_vag_ranges++;
            continue;
        }
        for (j = 0; j < 16; j++)
            if (vb[tone->vag_offset + j] != 0) {
                report->bad_sample_prefixes++;
                break;
            }
        for (block = tone->vag_offset + 16;
             block + 16 <= tone->vag_offset + tone->vag_size;
             block += 16) {
            if (vb[block + 1] & 1) {
                has_end = 1;
                break;
            }
        }
        if (!has_end)
            report->samples_without_end++;
    }
}

static void audit_sequence(const SepSequence *sequence, const VabHeader *vab,
                           AudioAuditReport *report)
{
    int programs[16] = { 0 };
    int i;

    for (i = 0; i < sequence->event_count; i++) {
        const SepEvent *event = &sequence->events[i];
        int kind = event->type & 0xf0;
        int channel = event->type & 0x0f;

        if (kind == 0xc0) {
            programs[channel] = event->data1;
        } else if (kind == 0x90 && event->data2 != 0) {
            int matches = tone_matches(vab, programs[channel], event->data1);
            if (matches == 0) {
                report->missing_notes[programs[channel]][event->data1]++;
                report->missing_note_events++;
            } else if (matches > 1) {
                report->layered_note_events++;
            }
        } else if (kind == 0xe0 && event->data1 != 0) {
            report->bend_lsb_events++;
        } else if (kind == 0xb0) {
            if (event->data1 == 98 || event->data1 == 99)
                report->loop_control_events++;
            if (event->data1 != 7 && event->data1 != 10)
                report->ignored_control_events++;
        }
    }
}

int audio_audit_bgm(const uint8_t *vh_data, size_t vh_len,
                    const uint8_t *vb_data, size_t vb_len,
                    const uint8_t *sep_data, size_t sep_len,
                    AudioAuditReport *report)
{
    VabHeader vab;
    SepFile sep;
    int i;

    if (!vh_data || !vb_data || !sep_data || !report)
        return -1;
    memset(report, 0, sizeof(*report));
    if (vab_parse_vh(vh_data, vh_len, &vab) != 0)
        return -1;
    if (sep_parse(sep_data, sep_len, &sep) != 0)
        return -1;

    report->vh_size = (uint32_t)vh_len;
    report->vb_size = (uint32_t)vb_len;
    report->declared_file_size = vab.file_size;
    report->program_count = vab.program_count;
    report->tone_count = vab.tone_count;
    report->vag_count = vab.vag_count;
    report->sequence_count = (uint16_t)sep.sequence_count;
    audit_samples(vb_data, vb_len, &vab, report);
    for (i = 0; i < sep.sequence_count; i++)
        audit_sequence(&sep.sequences[i], &vab, report);

    sep_free(&sep);
    return 0;
}
