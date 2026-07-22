#include <stdlib.h>
#include <string.h>

#include "spu_reverb.h"

#define REVERB_RAM_SIZE 0x80000
#define REVERB_END_ADDR 0x7FFFE

enum {
  REG_dAPF1 = 0,
  REG_dAPF2,
  REG_vIIR,
  REG_vCOMB1,
  REG_vCOMB2,
  REG_vCOMB3,
  REG_vCOMB4,
  REG_vWALL,
  REG_vAPF1,
  REG_vAPF2,
  REG_mLSAME,
  REG_mRSAME,
  REG_mLCOMB1,
  REG_mRCOMB1,
  REG_mLCOMB2,
  REG_mRCOMB2,
  REG_dLSAME,
  REG_dRSAME,
  REG_mLDIFF,
  REG_mRDIFF,
  REG_mLCOMB3,
  REG_mRCOMB3,
  REG_mLCOMB4,
  REG_mRCOMB4,
  REG_dLDIFF,
  REG_dRDIFF,
  REG_mLAPF1,
  REG_mRAPF1,
  REG_mLAPF2,
  REG_mRAPF2,
  REG_vLIN,
  REG_vRIN,
  REG_COUNT,
  REG_vLOUT,
  REG_vROUT
};

typedef struct {
  SpuReverb base;
  int16_t   ram[REVERB_RAM_SIZE / 2];
  uint32_t  mbase;
  uint32_t  buf_addr;
  int       sample_rate;
  int16_t   regs[REG_COUNT];
  int32_t   left_out;
  int32_t   right_out;
} SpuReverbNocash;

static int16_t saturate16(int32_t v) {
  if (v < -32768)
    return -32768;
  if (v > 32767)
    return 32767;
  return (int16_t)v;
}

static int32_t mulvol(int32_t sample, int16_t vol) {
  return (int32_t)(((int64_t)sample * vol) >> 15);
}

static uint32_t wrap_reverb(SpuReverbNocash* r, uint32_t addr) {
  if (addr > REVERB_END_ADDR)
    addr = r->mbase + (addr - r->mbase) % (REVERB_END_ADDR - r->mbase + 2);
  return addr & ~1u;
}

static int16_t read_buf(SpuReverbNocash* r, uint32_t offset) {
  uint32_t addr = wrap_reverb(r, r->buf_addr + offset * 2);
  if (addr < REVERB_RAM_SIZE)
    return r->ram[addr / 2];
  return 0;
}

static void write_buf(SpuReverbNocash* r, uint32_t offset, int16_t value) {
  uint32_t addr = wrap_reverb(r, r->buf_addr + offset * 2);
  if (addr < REVERB_RAM_SIZE)
    r->ram[addr / 2] = value;
}

static int16_t read_buf_at(SpuReverbNocash* r, uint32_t base, uint32_t offset) {
  uint32_t addr = wrap_reverb(r, base + offset * 2);
  if (addr < REVERB_RAM_SIZE)
    return r->ram[addr / 2];
  return 0;
}

static void write_buf_at(SpuReverbNocash* r, uint32_t base, uint32_t offset,
                         int16_t value) {
  uint32_t addr = wrap_reverb(r, base + offset * 2);
  if (addr < REVERB_RAM_SIZE)
    r->ram[addr / 2] = value;
}

static void reverb_step(SpuReverbNocash* r, int32_t in_left, int32_t in_right) {
  int16_t* regs = r->regs;
  int32_t  Lin, Rin;
  int32_t  Lout, Rout;
  int32_t  temp;
  uint32_t addr;

  Lin = mulvol(in_left, regs[REG_vLIN]);
  Rin = mulvol(in_right, regs[REG_vRIN]);

  addr = r->buf_addr;

  temp = Lin + mulvol(read_buf_at(r, addr, regs[REG_dLSAME]), regs[REG_vWALL]);
  temp = mulvol(temp - read_buf(r, regs[REG_mLSAME] - 1), regs[REG_vIIR]);
  temp += read_buf(r, regs[REG_mLSAME] - 1);
  write_buf(r, regs[REG_mLSAME], saturate16(temp));

  temp = Rin + mulvol(read_buf_at(r, addr, regs[REG_dRSAME]), regs[REG_vWALL]);
  temp = mulvol(temp - read_buf(r, regs[REG_mRSAME] - 1), regs[REG_vIIR]);
  temp += read_buf(r, regs[REG_mRSAME] - 1);
  write_buf(r, regs[REG_mRSAME], saturate16(temp));

  temp = Lin + mulvol(read_buf_at(r, addr, regs[REG_dRDIFF]), regs[REG_vWALL]);
  temp = mulvol(temp - read_buf(r, regs[REG_mLDIFF] - 1), regs[REG_vIIR]);
  temp += read_buf(r, regs[REG_mLDIFF] - 1);
  write_buf(r, regs[REG_mLDIFF], saturate16(temp));

  temp = Rin + mulvol(read_buf_at(r, addr, regs[REG_dLDIFF]), regs[REG_vWALL]);
  temp = mulvol(temp - read_buf(r, regs[REG_mRDIFF] - 1), regs[REG_vIIR]);
  temp += read_buf(r, regs[REG_mRDIFF] - 1);
  write_buf(r, regs[REG_mRDIFF], saturate16(temp));

  Lout = mulvol(read_buf(r, regs[REG_mLCOMB1]), regs[REG_vCOMB1]);
  Lout += mulvol(read_buf(r, regs[REG_mLCOMB2]), regs[REG_vCOMB2]);
  Lout += mulvol(read_buf(r, regs[REG_mLCOMB3]), regs[REG_vCOMB3]);
  Lout += mulvol(read_buf(r, regs[REG_mLCOMB4]), regs[REG_vCOMB4]);

  Rout = mulvol(read_buf(r, regs[REG_mRCOMB1]), regs[REG_vCOMB1]);
  Rout += mulvol(read_buf(r, regs[REG_mRCOMB2]), regs[REG_vCOMB2]);
  Rout += mulvol(read_buf(r, regs[REG_mRCOMB3]), regs[REG_vCOMB3]);
  Rout += mulvol(read_buf(r, regs[REG_mRCOMB4]), regs[REG_vCOMB4]);

  temp = read_buf(r, regs[REG_mLAPF1] - regs[REG_dAPF1]);
  Lout = Lout - mulvol(temp, regs[REG_vAPF1]);
  write_buf(r, regs[REG_mLAPF1], saturate16(Lout));
  Lout = mulvol(Lout, regs[REG_vAPF1]) + temp;

  temp = read_buf(r, regs[REG_mRAPF1] - regs[REG_dAPF1]);
  Rout = Rout - mulvol(temp, regs[REG_vAPF1]);
  write_buf(r, regs[REG_mRAPF1], saturate16(Rout));
  Rout = mulvol(Rout, regs[REG_vAPF1]) + temp;

  temp = read_buf(r, regs[REG_mLAPF2] - regs[REG_dAPF2]);
  Lout = Lout - mulvol(temp, regs[REG_vAPF2]);
  write_buf(r, regs[REG_mLAPF2], saturate16(Lout));
  Lout = mulvol(Lout, regs[REG_vAPF2]) + temp;

  temp = read_buf(r, regs[REG_mRAPF2] - regs[REG_dAPF2]);
  Rout = Rout - mulvol(temp, regs[REG_vAPF2]);
  write_buf(r, regs[REG_mRAPF2], saturate16(Rout));
  Rout = mulvol(Rout, regs[REG_vAPF2]) + temp;

  r->left_out = saturate16(mulvol(Lout, regs[REG_vLOUT]));
  r->right_out = saturate16(mulvol(Rout, regs[REG_vROUT]));

  r->buf_addr += 2;
  if (r->buf_addr > REVERB_END_ADDR)
    r->buf_addr = r->mbase;
}

static SpuReverb* nocash_create(uint32_t start_addr, int sample_rate) {
  SpuReverbNocash* r = (SpuReverbNocash*)calloc(1, sizeof(*r));
  if (!r)
    return NULL;
  r->base.ops = &spu_reverb_nocash_ops;
  r->mbase = start_addr & ~1u;
  r->buf_addr = r->mbase;
  r->sample_rate = sample_rate;
  return (SpuReverb*)r;
}

static void nocash_destroy(SpuReverb* rvb) {
  free(rvb);
}

static void nocash_process(SpuReverb* rvb, const int32_t* input_left,
                           const int32_t* input_right, int32_t* output_left,
                           int32_t* output_right, size_t frames) {
  SpuReverbNocash* r = (SpuReverbNocash*)rvb;
  size_t           i;

  for (i = 0; i < frames; i++) {
    int32_t in_l = input_left ? input_left[i] : 0;
    int32_t in_r = input_right ? input_right[i] : 0;
    reverb_step(r, in_l, in_r);
    if (output_left)
      output_left[i] = r->left_out;
    if (output_right)
      output_right[i] = r->right_out;
  }
}

static void nocash_set_register(SpuReverb* rvb, uint32_t address,
                                uint16_t value) {
  SpuReverbNocash* r = (SpuReverbNocash*)rvb;
  size_t           index;

  if (address == PSX_REVERB_START) {
    r->mbase = ((uint32_t)value << 3) & ~1u;
    r->buf_addr = r->mbase;
    return;
  }
  if (address == PSX_REVERB_VOLL) {
    r->regs[REG_vLOUT] = (int16_t)value;
    return;
  }
  if (address == PSX_REVERB_VOLR) {
    r->regs[REG_vROUT] = (int16_t)value;
    return;
  }
  if (address >= PSX_REVERB_REG_BASE &&
      address < PSX_REVERB_REG_BASE + PSX_REVERB_REG_COUNT * 2) {
    index = (address - PSX_REVERB_REG_BASE) / 2;
    if (index < REG_COUNT) {
      r->regs[index] = (int16_t)value;
    }
  }
}

static uint16_t nocash_get_register(const SpuReverb* rvb, uint32_t address) {
  const SpuReverbNocash* r = (const SpuReverbNocash*)rvb;
  size_t                 index;

  if (address == PSX_REVERB_START)
    return (uint16_t)(r->mbase >> 3);
  if (address == PSX_REVERB_VOLL)
    return (uint16_t)r->regs[REG_vLOUT];
  if (address == PSX_REVERB_VOLR)
    return (uint16_t)r->regs[REG_vROUT];
  if (address >= PSX_REVERB_REG_BASE &&
      address < PSX_REVERB_REG_BASE + PSX_REVERB_REG_COUNT * 2) {
    index = (address - PSX_REVERB_REG_BASE) / 2;
    if (index < REG_COUNT)
      return (uint16_t)r->regs[index];
  }
  return 0;
}

const SpuReverbOps spu_reverb_nocash_ops = {
    "nocash",       nocash_create,       nocash_destroy,
    nocash_process, nocash_set_register, nocash_get_register,
};

SpuReverb* spu_reverb_create(const SpuReverbOps* ops, uint32_t start_addr,
                             int sample_rate) {
  if (!ops || !ops->create)
    return NULL;
  return ops->create(start_addr, sample_rate);
}

void spu_reverb_destroy(SpuReverb* rvb) {
  if (rvb && rvb->ops && rvb->ops->destroy)
    rvb->ops->destroy(rvb);
}

void spu_reverb_process(SpuReverb* rvb, const int32_t* input_left,
                        const int32_t* input_right, int32_t* output_left,
                        int32_t* output_right, size_t frames) {
  if (rvb && rvb->ops && rvb->ops->process)
    rvb->ops->process(rvb, input_left, input_right, output_left, output_right,
                      frames);
}

void spu_reverb_set_register(SpuReverb* rvb, uint32_t address, uint16_t value) {
  if (rvb && rvb->ops && rvb->ops->set_register)
    rvb->ops->set_register(rvb, address, value);
}

uint16_t spu_reverb_get_register(const SpuReverb* rvb, uint32_t address) {
  if (rvb && rvb->ops && rvb->ops->get_register)
    return rvb->ops->get_register(rvb, address);
  return 0;
}
