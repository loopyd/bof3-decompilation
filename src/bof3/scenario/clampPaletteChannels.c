#include "bof3/scenario/scena16_internal.h"

/* @behavior Clamps one 16-entry palette window channel by channel: for every
 * halfword of the source window it splits the low 15 bits into three 5-bit
 * components (arithmetic shift by a running 0/5/10 count, masked with 0x1F),
 * replaces any component above the caller's limit with the limit itself, packs
 * the three components back into 15 bits and stores the packed word into the
 * destination window. A non-zero packed word is re-stored with bit 15 set; a
 * packed word of zero is left as stored. The stage serial D_80145988 is bumped
 * once per call.
 *
 * Instruction evidence for the shapes: the intensity argument is masked with
 * 0xFF and a second copy is masked with 0xFFFF as the unsigned limit compared
 * with `sltu`; the source halfword is loaded with `lhu` and shifted with `srav`,
 * so the colour word is a signed full word with no extra 0xFFFF mask; the inner
 * channel loop accumulates through two full words (`or` into one register and a
 * copy into the carried register), both of which stay live to the end of the
 * outer iteration; the destination is formed as base + byte offset with `addu`,
 * and the source pointer is compared with `slt` (a signed comparison).
 * @source 0x801F83B0
 * @status partial
 * @match 46.51
 * @residual Channel-loop exit (li 3/bne instead of slti 3/bnez) and register allocation differ; both bodies are 172 bytes.
 */
void clampPaletteChannels(u32 intensity) {
  const u16* src;
  const u16* src_end;
  u32        offset;
  u32        limit;
  u32        packed;
  u32        merged;
  s32        color;
  u32        component;
  u32        shift;
  s32        i;
  u16*       dst;

  intensity &= 0xffu;
  limit = intensity & 0xffffu;
  offset = 0u;
  src = (const u16*)SCENA16_PALETTE_SRC;
  src_end = PSX_PTR(const u16, 0x80033820u);

  do {
    packed = 0u;
    i = 0;
    color = *src;
    shift = 0u;
    do {
      component = (u32)(color >> shift) & 0x1fu;
      if (limit < component) {
        component = intensity;
      }
      merged = (packed << 5) | component;
      packed = merged;
      i++;
      shift += 5u;
      if (i == 3) {
        break;
      }
    } while (1);

    dst = (u16*)((u8*)SCENA16_PALETTE_DST + offset);
    *dst = (u16)packed;
    if ((packed & 0xffffu) != 0u) {
      *dst = (u16)(merged | 0x8000u);
    }

    src++;
    offset += 2u;
  } while ((s32)src < (s32)src_end);

  D_80145988 = (u8)(D_80145988 + 1u);
}
