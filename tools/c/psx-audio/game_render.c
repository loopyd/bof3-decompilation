#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "psx_machine.h"
#include "spu_device.h"

#define GAME_VH_ADDRESS 0x80020000u
#define GAME_SEP_ADDRESS 0x80024000u
#define GAME_VB_ADDRESS 0x80030000u
#define GAME_TABLE_ADDRESS 0x80078000u

#define SS_INIT 0x8016b2acu
#define SS_SEP_OPEN_J 0x8016b38cu
#define SS_SEP_PLAY 0x8016b9ccu
#define SS_SEQ_CALLED_T_BY_T 0x8016c548u
#define SS_SET_TABLE_SIZE 0x8016d7ecu
#define SS_VAB_OPEN_HEAD 0x80173c20u
#define SS_VAB_TRANS_BODY_PARTLY 0x80174354u

#define CALL_LIMIT 2000000u
#define TICK_RATE 60
#define SPU_RATE 44100

static int call(PsxMachine *machine, uint32_t address,
                uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    uint32_t arguments[4];
    PsxMachineStatus status;
    arguments[0] = a0;
    arguments[1] = a1;
    arguments[2] = a2;
    arguments[3] = a3;
    status = psx_machine_call(machine, address, arguments, CALL_LIMIT);
    if (status != PSX_MACHINE_OK) {
        const PsxMachineFault *fault = psx_machine_fault(machine);
        fprintf(stderr,
                "game call 0x%08X failed: %s at PC=0x%08X instruction=0x%08X address=0x%08X\n",
                address, psx_machine_status_string(status), fault->pc,
                fault->instruction, fault->address);
        return -1;
    }
    return 0;
}

static int64_t sequence_frames(const uint8_t *data, size_t size,
                               int sequence_index)
{
    SepFile sep;
    SepSequence *sequence;
    int64_t tick = 0;
    int64_t tempo_tick = 0;
    double seconds = 0.0;
    double tempo;
    int resolution;
    int i;

    if (sep_parse(data, size, &sep) != 0)
        return -1;
    if (sequence_index < 0 || sequence_index >= sep.sequence_count) {
        sep_free(&sep);
        return -1;
    }
    sequence = &sep.sequences[sequence_index];
    resolution = sequence->resolution > 0 ? sequence->resolution : 48;
    tempo = sequence->tempo_us > 0 ? sequence->tempo_us : 500000.0;
    for (i = 0; i < sequence->event_count; i++) {
        SepEvent *event = &sequence->events[i];
        tick += event->delta;
        if (event->type == 0xff && event->meta_type == 0x51 &&
            event->meta_len >= 3) {
            seconds += (double)(tick - tempo_tick) * tempo /
                       (1000000.0 * resolution);
            tempo_tick = tick;
            tempo = (double)((event->meta[0] << 16) |
                             (event->meta[1] << 8) | event->meta[2]);
        }
    }
    seconds += (double)(tick - tempo_tick) * tempo /
               (1000000.0 * resolution);
    sep_free(&sep);
    return (int64_t)((seconds + 2.0) * SPU_RATE);
}

int render_game_bgm(const AudioRenderRequest *request, RenderOutput *out)
{
    PsxSpu *spu = NULL;
    PsxMachine *machine = NULL;
    int16_t *pcm = NULL;
    uint8_t table[176 * 4];
    int64_t frames;
    int64_t rendered = 0;
    int vab_id;
    int sep_id;
    int result = -1;

    if (!request || !out || !request->game_image ||
        request->output_rate != SPU_RATE)
        return -1;
    if (request->vh_len > GAME_SEP_ADDRESS - GAME_VH_ADDRESS ||
        request->sep_len > GAME_VB_ADDRESS - GAME_SEP_ADDRESS ||
        request->vb_len > GAME_TABLE_ADDRESS - GAME_VB_ADDRESS)
        return -1;
    frames = sequence_frames(request->sep_data, request->sep_len,
                             request->sequence);
    if (frames <= 0 || (uint64_t)frames > SIZE_MAX / (2 * sizeof(*pcm)))
        return -1;

    spu = psx_spu_create();
    machine = psx_machine_create(request->game_image, spu);
    pcm = (int16_t *)calloc((size_t)frames * 2, sizeof(*pcm));
    memset(table, 0, sizeof(table));
    if (!spu || !machine || !pcm)
        goto done;
    if (psx_machine_write_ram(machine, GAME_VH_ADDRESS, request->vh_data,
                              request->vh_len) != PSX_MACHINE_OK ||
        psx_machine_write_ram(machine, GAME_SEP_ADDRESS, request->sep_data,
                              request->sep_len) != PSX_MACHINE_OK ||
        psx_machine_write_ram(machine, GAME_VB_ADDRESS, request->vb_data,
                              request->vb_len) != PSX_MACHINE_OK ||
        psx_machine_write_ram(machine, GAME_TABLE_ADDRESS, table,
                              sizeof(table)) != PSX_MACHINE_OK)
        goto done;

    if (call(machine, SS_INIT, 0, 0, 0, 0) != 0 ||
        call(machine, SS_SET_TABLE_SIZE, GAME_TABLE_ADDRESS, 1, 1, 0) != 0 ||
        call(machine, SS_VAB_OPEN_HEAD, GAME_VH_ADDRESS, 0xffffu, 0, 0) != 0)
        goto done;
    vab_id = (int16_t)psx_machine_register(machine, 2);
    if (vab_id < 0 ||
        call(machine, SS_VAB_TRANS_BODY_PARTLY, GAME_VB_ADDRESS,
             (uint32_t)request->vb_len, (uint32_t)vab_id, 0) != 0 ||
        call(machine, SS_SEP_OPEN_J, GAME_SEP_ADDRESS,
             (uint32_t)vab_id, 4, 0) != 0)
        goto done;
    sep_id = (int16_t)psx_machine_register(machine, 2);
    if (sep_id < 0 ||
        call(machine, SS_SEP_PLAY, (uint32_t)sep_id,
             (uint32_t)request->sequence, 1, 0) != 0)
        goto done;

    while (rendered < frames) {
        size_t count = (size_t)(frames - rendered);
        if (count > SPU_RATE / TICK_RATE)
            count = SPU_RATE / TICK_RATE;
        if (call(machine, SS_SEQ_CALLED_T_BY_T, 0, 0, 0, 0) != 0 ||
            psx_spu_render(spu, pcm + rendered * 2, count) != 0)
            goto done;
        rendered += (int64_t)count;
    }

    {
        int heard = 0;
        int64_t i;
        for (i = 0; i < frames * 2; i++)
            if (pcm[i] != 0) {
                heard = 1;
                break;
            }
        if (!heard) {
            fprintf(stderr,
                    "game scheduler produced no audible voice-register state\n");
            goto done;
        }
    }

    out->pcm = pcm;
    out->frames = frames;
    out->rate = SPU_RATE;
    pcm = NULL;
    result = 0;

done:
    free(pcm);
    psx_machine_destroy(machine);
    psx_spu_destroy(spu);
    return result;
}
