#include "bof3/scenario/scena00_internal.h"

/* @behavior Runs the secondary SCENA00 state controller selected by the state
 * byte D_80146875: state 0, when the shared mode byte D_80146866 reads 0x30,
 * arms effect bank 0x1F in D_8014832E, requests frontend local mode 1 through
 * func_8014ECAC, stores the object byte func_8019601C() in scratchpad
 * 0x1F800000 and seeds that object's 0x74-byte row at 0x80143FC8 (+0 = 1,
 * +5 = 0x13, +9 = 0xFF and the s16 words D_801492D8 + 0x1C0, D_801492DA and
 * D_801492DC at +0x64/+0x68/+0x6C) unless that byte is 0xFF, then advances the
 * state byte to 1; state 1 advances to state 2 when D_80146866 reads 0x35 and
 * otherwise, while D_80149314 still reads 0x4400, arms 0x6200 in D_80149314,
 * subtracts 0x1E from D_80149322 and rebases D_8014930C, D_80147A90 and
 * D_80143F80 by -0x1E0000 before running func_80154698; state 2 stores 0x37 in
 * D_80146866 and advances the state byte to 3; state 3, when D_80146866 reads
 * 0x38, queues frontend cue func_80161C20(2, 0x64, 0x20), seeds the front
 * selector context (2, 0x340000, 0x430000, 3), requests primary state 4 in
 * D_80146874 and returns the state byte to 0.
 * @source 0x801F9FB8
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the state byte is reached through one
 * address taken into a local pointer, so it lives in $s0 across the calls and
 * every state store goes through that base. The three rebased transform words
 * each need their own pointer local; written as plain scalar globals the
 * compiler folds each access to `lui/%lo(...)`, never fills the `jal` delay
 * slot and matches 150/178 instructions (measured with bin/asm-diff).
 */
void func_801F9FB8(void) {
  u8* state = &D_80146875;
  u8  status = *state;

  switch (status) {
    case 0: {
      u8* scratch;
      u8  object_index;

      if (D_80146866 != 0x30u) {
        break;
      }

      D_8014832E = 0x1fu;
      func_8014ECAC(1u);
      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) =
            (s32)((s16)D_801492D8 + 0x1c0);
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0xffu;
      }

      *state = 1u;
      break;
    }

    case 1:
      if (D_80146866 == 0x35u) {
        *state = 2u;
        break;
      }

      if (D_80149314 == 0x4400u) {
        s32* word_a = &D_8014930C;
        s32* word_b = &D_80147A90;
        s32* word_c = &D_80143F80;

        D_80149314 = 0x6200u;
        D_80149322 = (u16)(D_80149322 - 0x1eu);
        *word_a += -0x1e0000;
        *word_b += -0x1e0000;
        *word_c += -0x1e0000;
        func_80154698();
      }
      break;

    case 2:
      D_80146866 = 0x37u;
      *state = 3u;
      break;

    case 3:
      if (D_80146866 != 0x38u) {
        break;
      }

      func_80161C20(2u, 0x64, 0x20);
      func_8019FA28(2u, 0x340000u, 0x430000u, 3u);
      D_80146874 = 4;
      *state = 0u;
      break;
  }
}
