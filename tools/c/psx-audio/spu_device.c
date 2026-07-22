#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "spu_device.h"
#include "spu_reverb.h"
#include "util.h"

#define SPU_BLOCK_SIZE    16u
#define SPU_BLOCK_SAMPLES 28u
#define SPU_PITCH_ONE     0x1000u
#define SPU_PITCH_MAX     0x4000u

typedef struct {
  SpuAdsr       adsr;
  PsxAdpcmState adpcm;
  int16_t       decoded[SPU_BLOCK_SAMPLES];
  int16_t       history[4];
  uint32_t      block_address;
  uint32_t      repeat_address;
  uint32_t      pitch_counter;
  unsigned int  decoded_position;
  uint8_t       block_flags;
  int           active;
  int           releasing;
  int           source_ended;
  int16_t       sweep_vol_left;
  int16_t       sweep_vol_right;
  int16_t       envelope_output;
} PsxSpuVoice;

struct PsxSpu {
  uint16_t     registers[PSX_SPU_REGISTER_SIZE / 2];
  uint8_t      ram[PSX_SPU_RAM_SIZE];
  PsxSpuVoice  voices[PSX_SPU_VOICE_COUNT];
  uint32_t     transfer_address;
  PsxSpuWrite* writes;
  size_t       write_count;
  size_t       write_capacity;
  SpuReverb*   reverb;
  int          reverb_enabled;
  int32_t      noise_level;
  int32_t      noise_timer;
  int32_t      noise_step;
  int32_t      noise_shift;
};

static size_t register_offset(uint32_t address) {
  return (size_t)((address - PSX_SPU_REGISTER_BASE) / 2);
}

static uint16_t voice_register(const PsxSpu* spu, unsigned int voice,
                               unsigned int offset) {
  uint32_t address =
      PSX_SPU_REGISTER_BASE + voice * PSX_SPU_VOICE_STRIDE + offset;
  return spu->registers[register_offset(address)];
}

static void set_endx(PsxSpu* spu, unsigned int voice, int ended) {
  uint32_t address = voice < 16 ? PSX_SPU_ENDX_LOW : PSX_SPU_ENDX_HIGH;
  uint16_t mask = (uint16_t)(1u << (voice & 15u));
  size_t   index = register_offset(address);

  if (ended)
    spu->registers[index] |= mask;
  else
    spu->registers[index] &= (uint16_t)~mask;
}

static void read_ram_block(const PsxSpu* spu, uint32_t address,
                           uint8_t block[SPU_BLOCK_SIZE]) {
  unsigned int i;

  for (i = 0; i < SPU_BLOCK_SIZE; i++)
    block[i] = spu->ram[(address + i) & (PSX_SPU_RAM_SIZE - 1)];
}

static void decode_voice_block(PsxSpu* spu, PsxSpuVoice* voice) {
  uint8_t block[SPU_BLOCK_SIZE];

  read_ram_block(spu, voice->block_address, block);
  psx_adpcm_decode_block(block, voice->decoded, &voice->adpcm);
  voice->decoded_position = 0;
  voice->block_flags = block[1];
  if ((voice->block_flags & 4u) != 0)
    voice->repeat_address = voice->block_address;
}

static int16_t next_voice_sample(PsxSpu* spu, PsxSpuVoice* voice,
                                 unsigned int voice_index) {
  if (voice->source_ended)
    return 0;

  if (voice->decoded_position == SPU_BLOCK_SAMPLES) {
    if ((voice->block_flags & 1u) != 0) {
      set_endx(spu, voice_index, 1);
      if ((voice->block_flags & 2u) != 0) {
        voice->block_address = voice->repeat_address;
      } else {
        voice->source_ended = 1;
        voice->releasing = 1;
        spu_adsr_key_off(&voice->adsr);
        return 0;
      }
    } else {
      voice->block_address =
          (voice->block_address + SPU_BLOCK_SIZE) & (PSX_SPU_RAM_SIZE - 1);
    }
    decode_voice_block(spu, voice);
  }

  return voice->decoded[voice->decoded_position++];
}

static void advance_voice(PsxSpu* spu, PsxSpuVoice* voice,
                          unsigned int voice_index) {
  voice->history[0] = voice->history[1];
  voice->history[1] = voice->history[2];
  voice->history[2] = voice->history[3];
  voice->history[3] = next_voice_sample(spu, voice, voice_index);
}

static void key_on_voice(PsxSpu* spu, unsigned int voice_index) {
  PsxSpuVoice* voice = &spu->voices[voice_index];

  memset(voice, 0, sizeof(*voice));
  voice->block_address =
      ((uint32_t)voice_register(spu, voice_index, PSX_SPU_VOICE_START_ADDRESS)
       << 3) &
      (PSX_SPU_RAM_SIZE - 1);
  voice->repeat_address =
      ((uint32_t)voice_register(spu, voice_index, PSX_SPU_VOICE_REPEAT_ADDRESS)
       << 3) &
      (PSX_SPU_RAM_SIZE - 1);
  voice->active = 1;
  voice->sweep_vol_left = 0;
  voice->sweep_vol_right = 0;
  psx_adpcm_init(&voice->adpcm);
  spu_adsr_key_on(&voice->adsr,
                  voice_register(spu, voice_index, PSX_SPU_VOICE_ADSR1),
                  voice_register(spu, voice_index, PSX_SPU_VOICE_ADSR2));
  set_endx(spu, voice_index, 0);
  decode_voice_block(spu, voice);
  advance_voice(spu, voice, voice_index);
}

static void key_off_voice(PsxSpu* spu, unsigned int voice_index) {
  PsxSpuVoice* voice = &spu->voices[voice_index];

  if (!voice->active)
    return;
  voice->releasing = 1;
  spu_adsr_key_off(&voice->adsr);
}

static void apply_key_mask(PsxSpu* spu, unsigned int first_voice, uint16_t mask,
                           int key_on) {
  unsigned int bit;

  for (bit = 0; bit < 16 && first_voice + bit < PSX_SPU_VOICE_COUNT; bit++) {
    if ((mask & (uint16_t)(1u << bit)) == 0)
      continue;
    if (key_on)
      key_on_voice(spu, first_voice + bit);
    else
      key_off_voice(spu, first_voice + bit);
  }
}

static int32_t fixed_volume(uint16_t value) {
  int32_t volume;

  if ((value & 0x8000u) != 0)
    return 0;
  volume = value & 0x7fffu;
  if ((volume & 0x4000) != 0)
    volume -= 0x8000;
  return volume;
}

static void noise_update_params(PsxSpu* spu) {
  uint16_t cnt = spu->registers[register_offset(PSX_SPU_CONTROL)];
  spu->noise_shift = (cnt >> 10) & 0x0F;
  spu->noise_step = 4 + ((cnt >> 8) & 0x03);
}

static int16_t noise_tick(PsxSpu* spu) {
  int32_t parity;

  spu->noise_timer -= spu->noise_step;
  parity = ((spu->noise_level >> 15) ^ (spu->noise_level >> 12) ^
            (spu->noise_level >> 11) ^ (spu->noise_level >> 10) ^ 1) &
           1;
  if (spu->noise_timer < 0) {
    spu->noise_level = (spu->noise_level << 1) | parity;
    spu->noise_timer += (0x20000 >> spu->noise_shift);
    if (spu->noise_timer < 0)
      spu->noise_timer += (0x20000 >> spu->noise_shift);
  }
  return (int16_t)(spu->noise_level & 0xFFFF);
}

static int16_t sweep_tick(int16_t* level, uint16_t sweep_reg) {
  int32_t cycles, step;
  int     shift, step_val, direction, exponential;

  shift = (sweep_reg >> 2) & 0x1F;
  step_val = sweep_reg & 0x03;
  direction = (sweep_reg >> 13) & 1;
  exponential = (sweep_reg >> 14) & 1;

  cycles = 1 << (shift > 11 ? shift - 11 : 0);
  step = (step_val + 4) << (11 > shift ? 11 - shift : 0);
  if (!direction)
    step = -step;

  if (exponential && direction && *level > 0x6000)
    cycles *= 4;
  if (exponential && !direction)
    step = (step * *level) >> 15;

  *level = (int16_t)(*level + step);
  if (*level > 0x7FFF)
    *level = 0x7FFF;
  if (*level < 0)
    *level = 0;
  return *level;
}

static int16_t clamp_sample(int64_t sample) {
  if (sample < -32768)
    return -32768;
  if (sample > 32767)
    return 32767;
  return (int16_t)sample;
}

static int register_index(uint32_t address, size_t* index) {
  uint32_t offset;
  if (address < PSX_SPU_REGISTER_BASE ||
      address >= PSX_SPU_REGISTER_BASE + PSX_SPU_REGISTER_SIZE ||
      (address & 1u) != 0)
    return -1;
  offset = address - PSX_SPU_REGISTER_BASE;
  *index = (size_t)(offset / 2);
  return 0;
}

PsxSpu* psx_spu_create(void) {
  return (PsxSpu*)calloc(1, sizeof(PsxSpu));
}

void psx_spu_destroy(PsxSpu* spu) {
  if (!spu)
    return;
  spu_reverb_destroy(spu->reverb);
  free(spu->writes);
  free(spu);
}

void psx_spu_reset(PsxSpu* spu) {
  if (!spu)
    return;
  memset(spu->registers, 0, sizeof(spu->registers));
  memset(spu->ram, 0, sizeof(spu->ram));
  memset(spu->voices, 0, sizeof(spu->voices));
  spu->transfer_address = 0;
  spu->write_count = 0;
}

int psx_spu_read16(PsxSpu* spu, uint32_t address, uint16_t* value) {
  size_t index;
  if (!spu || !value || register_index(address, &index) != 0)
    return -1;
  *value = spu->registers[index];
  return 0;
}

int psx_spu_write16(PsxSpu* spu, uint32_t address, uint16_t value,
                    uint64_t cycle) {
  size_t index;
  if (!spu || register_index(address, &index) != 0)
    return -1;
  if (spu->write_count == spu->write_capacity) {
    size_t       capacity = spu->write_capacity ? spu->write_capacity * 2 : 256;
    PsxSpuWrite* writes =
        (PsxSpuWrite*)realloc(spu->writes, capacity * sizeof(*writes));
    if (!writes)
      return -1;
    spu->writes = writes;
    spu->write_capacity = capacity;
  }
  spu->registers[index] = value;
  if (address == PSX_SPU_TRANSFER_ADDRESS) {
    spu->transfer_address = ((uint32_t)value << 3) & (PSX_SPU_RAM_SIZE - 1);
  } else if (address == PSX_SPU_TRANSFER_FIFO) {
    spu->ram[spu->transfer_address] = (uint8_t)value;
    spu->ram[(spu->transfer_address + 1) & (PSX_SPU_RAM_SIZE - 1)] =
        (uint8_t)(value >> 8);
    spu->transfer_address =
        (spu->transfer_address + 2) & (PSX_SPU_RAM_SIZE - 1);
  } else if (address == PSX_SPU_KEY_ON_LOW) {
    apply_key_mask(spu, 0, value, 1);
  } else if (address == PSX_SPU_KEY_ON_HIGH) {
    apply_key_mask(spu, 16, value, 1);
  } else if (address == PSX_SPU_KEY_OFF_LOW) {
    apply_key_mask(spu, 0, value, 0);
  } else if (address == PSX_SPU_KEY_OFF_HIGH) {
    apply_key_mask(spu, 16, value, 0);
  } else if (address == PSX_SPU_CONTROL) {
    spu->reverb_enabled = (value >> 7) & 1;
    noise_update_params(spu);
  }
  if (spu->reverb &&
      (address == PSX_REVERB_VOLL || address == PSX_REVERB_VOLR ||
       address == PSX_REVERB_START ||
       (address >= PSX_REVERB_REG_BASE &&
        address < PSX_REVERB_REG_BASE + PSX_REVERB_REG_COUNT * 2))) {
    spu_reverb_set_register(spu->reverb, address, value);
  }
  spu->writes[spu->write_count].cycle = cycle;
  spu->writes[spu->write_count].address = address;
  spu->writes[spu->write_count].value = value;
  spu->write_count++;
  return 0;
}

int psx_spu_dma_write(PsxSpu* spu, const uint8_t* data, size_t size,
                      uint64_t cycle) {
  size_t i;
  (void)cycle;
  if (!spu || !data || (size & 1u) != 0)
    return -1;
  for (i = 0; i < size; i++) {
    spu->ram[spu->transfer_address] = data[i];
    spu->transfer_address =
        (spu->transfer_address + 1) & (PSX_SPU_RAM_SIZE - 1);
  }
  return 0;
}

size_t psx_spu_write_count(const PsxSpu* spu) {
  return spu ? spu->write_count : 0;
}

const PsxSpuWrite* psx_spu_writes(const PsxSpu* spu) {
  return spu ? spu->writes : NULL;
}

const uint8_t* psx_spu_ram(const PsxSpu* spu) {
  return spu ? spu->ram : NULL;
}

int psx_spu_voice_active(const PsxSpu* spu, unsigned int voice) {
  if (!spu || voice >= PSX_SPU_VOICE_COUNT)
    return 0;
  return spu->voices[voice].active;
}

int psx_spu_render(PsxSpu* spu, int16_t* stereo, size_t frames) {
  size_t   frame;
  int32_t  main_left;
  int32_t  main_right;
  uint16_t rvbon_low, rvbon_high;
  uint16_t non_low, non_high;
  uint16_t pmon_low, pmon_high;

  if (!spu || (!stereo && frames != 0))
    return -1;

  main_left =
      fixed_volume(spu->registers[register_offset(PSX_SPU_MAIN_VOLUME_LEFT)]);
  main_right =
      fixed_volume(spu->registers[register_offset(PSX_SPU_MAIN_VOLUME_RIGHT)]);
  rvbon_low = spu->registers[register_offset(PSX_SPU_REVERB_ON_LOW)];
  rvbon_high = spu->registers[register_offset(PSX_SPU_REVERB_ON_HIGH)];
  non_low = spu->registers[register_offset(PSX_SPU_NOISE_ON_LOW)];
  non_high = spu->registers[register_offset(PSX_SPU_NOISE_ON_HIGH)];
  pmon_low = spu->registers[register_offset(PSX_SPU_PITCH_MOD_ON_LOW)];
  pmon_high = spu->registers[register_offset(PSX_SPU_PITCH_MOD_ON_HIGH)];

  if (!spu->reverb) {
    size_t   k;
    uint16_t reverb_start_reg =
        spu->registers[register_offset(PSX_REVERB_START)];
    uint32_t start_addr = reverb_start_reg ? ((uint32_t)reverb_start_reg << 3) &
                                                 (PSX_SPU_RAM_SIZE - 2)
                                           : PSX_REVERB_AREA_START;
    spu->reverb = spu_reverb_create(&spu_reverb_nocash_ops, start_addr, 44100);
    for (k = 0; k < spu->write_count; k++) {
      uint32_t a = spu->writes[k].address;
      if (a == PSX_REVERB_VOLL || a == PSX_REVERB_VOLR ||
          a == PSX_REVERB_START ||
          (a >= PSX_REVERB_REG_BASE &&
           a < PSX_REVERB_REG_BASE + PSX_REVERB_REG_COUNT * 2))
        spu_reverb_set_register(spu->reverb, a, spu->writes[k].value);
    }
  }

  for (frame = 0; frame < frames; frame++) {
    int32_t      mix_left = 0;
    int32_t      mix_right = 0;
    int32_t      reverb_in_left = 0;
    int32_t      reverb_in_right = 0;
    int16_t      prev_voice_output = 0;
    unsigned int i;

    for (i = 0; i < PSX_SPU_VOICE_COUNT; i++) {
      PsxSpuVoice* voice = &spu->voices[i];
      uint32_t     pitch;
      int32_t      sample;
      int32_t      envelope;
      int          voice_rvb;
      int          voice_noise;
      int          voice_pmon;
      uint16_t     vol_l_reg, vol_r_reg;
      int32_t      vol_l, vol_r;
      int32_t      sl, sr;

      if (!voice->active)
        continue;

      voice_noise =
          (i < 16) ? ((non_low >> i) & 1) : ((non_high >> (i - 16)) & 1);
      voice_pmon = (i > 0) ? ((i < 16) ? ((pmon_low >> i) & 1)
                                       : ((pmon_high >> (i - 16)) & 1))
                           : 0;
      voice_rvb =
          (i < 16) ? ((rvbon_low >> i) & 1) : ((rvbon_high >> (i - 16)) & 1);

      if (voice_noise) {
        sample = (int32_t)noise_tick(spu);
        voice->history[0] = voice->history[1];
        voice->history[1] = voice->history[2];
        voice->history[2] = voice->history[3];
        voice->history[3] = (int16_t)sample;
      } else {
        sample = psx_gauss_interp(voice->history, 4, 3,
                                  (int)(voice->pitch_counter >> 4));
      }

      envelope = spu_adsr_tick(&voice->adsr);
      voice->envelope_output = (int16_t)envelope;
      sample = (sample * envelope) >> 15;

      vol_l_reg = voice_register(spu, i, PSX_SPU_VOICE_VOLUME_LEFT);
      vol_r_reg = voice_register(spu, i, PSX_SPU_VOICE_VOLUME_RIGHT);

      if (vol_l_reg & 0x8000) {
        sweep_tick(&voice->sweep_vol_left, vol_l_reg);
        vol_l = voice->sweep_vol_left;
      } else {
        vol_l = fixed_volume(vol_l_reg);
        voice->sweep_vol_left = (int16_t)vol_l;
      }
      if (vol_r_reg & 0x8000) {
        sweep_tick(&voice->sweep_vol_right, vol_r_reg);
        vol_r = voice->sweep_vol_right;
      } else {
        vol_r = fixed_volume(vol_r_reg);
        voice->sweep_vol_right = (int16_t)vol_r;
      }

      sl = (sample * vol_l) >> 14;
      sr = (sample * vol_r) >> 14;
      mix_left += sl;
      mix_right += sr;
      if (voice_rvb) {
        reverb_in_left += sl;
        reverb_in_right += sr;
      }

      if (voice->releasing && envelope == 0) {
        voice->active = 0;
        continue;
      }

      pitch = voice_register(spu, i, PSX_SPU_VOICE_PITCH);
      if (voice_pmon) {
        int32_t factor = (int32_t)prev_voice_output + 0x8000;
        int32_t step = (int32_t)(int16_t)pitch;
        step = (step * factor) >> 15;
        if (step < 0)
          step = 0;
        if (step > (int32_t)SPU_PITCH_MAX)
          step = SPU_PITCH_MAX;
        pitch = (uint32_t)step;
      }
      if (pitch > SPU_PITCH_MAX)
        pitch = SPU_PITCH_MAX;
      voice->pitch_counter += pitch;
      while (voice->pitch_counter >= SPU_PITCH_ONE) {
        voice->pitch_counter -= SPU_PITCH_ONE;
        advance_voice(spu, voice, i);
      }
      spu->registers[register_offset(
          PSX_SPU_REGISTER_BASE + i * PSX_SPU_VOICE_STRIDE +
          PSX_SPU_VOICE_ADSR_VOLUME)] = (uint16_t)envelope;
      prev_voice_output = (int16_t)envelope;
    }

    if (spu->reverb && spu->reverb_enabled) {
      int32_t rvb_out_l = 0, rvb_out_r = 0;
      spu_reverb_process(spu->reverb, &reverb_in_left, &reverb_in_right,
                         &rvb_out_l, &rvb_out_r, 1);
      mix_left += rvb_out_l;
      mix_right += rvb_out_r;
    }

    stereo[frame * 2] = clamp_sample(((int64_t)mix_left * main_left) >> 14);
    stereo[frame * 2 + 1] =
        clamp_sample(((int64_t)mix_right * main_right) >> 14);
  }

  return 0;
}
