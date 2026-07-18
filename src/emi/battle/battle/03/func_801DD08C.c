#include "internal.h"

/* @behavior copies the current local battler templates selected by byte `0x13c`
 * into each active local work record's inline block at offset `0x74`.
 * @source 0x801DD08C
 */
void func_801DD08C(void) {
  u8 index;

  index = 0u;
  if (*(volatile u8*)(0x80140000u + 0x62f0u) != 0) {
    do {
      u32           work_offset;
      u8            template_index;
      const u32*    src;
      volatile u32* dst;
      const u32*    end;

      work_offset = (u32)index * 0x140u;
      template_index = *(volatile u8*)(0x80140000u + 0x5fccu + work_offset);
      src = (const u32*)(0x80140000u + 0x4968u + ((u32)template_index * 0xa4u));
      dst = (volatile u32*)(0x80140000u + 0x5f04u + work_offset);
      end = (const u32*)((u32)src + 0xa0u);
      do {
        u32 word0;
        u32 word1;
        u32 word2;
        u32 word3;

        word0 = src[0];
        word1 = src[1];
        word2 = src[2];
        word3 = src[3];
        dst[0] = word0;
        dst[1] = word1;
        dst[2] = word2;
        dst[3] = word3;
        src += 4;
        dst += 4;
      } while (src != end);
      *dst = *src;
      index += 1u;
    } while (index < *(volatile u8*)(0x80140000u + 0x62f0u));
  }
}
