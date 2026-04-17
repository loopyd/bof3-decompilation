#include "internal.h"

/* does: copies the source quad table into the local work buffer, computes one
 * centroid and normalized local vector per slot, then recenters the copied
 * vertices around that centroid for all 27 entries.
 * @source: 0x801f3314 FUN_801f3314
 */
void func_801f3314(void) {
  volatile u8*       dst;
  const volatile u8* src;
  volatile s16*      state;
  u8                 i;
  VECTOR             center;

  dst = BOF3_WORLD00_AREA024_VERTEX_DST;
  src = BOF3_WORLD00_AREA024_VERTEX_SRC;
  state = BOF3_WORLD00_AREA024_STATE_BASE;
  i = 0u;

  do {
    volatile u32*       dst_words;
    const volatile u32* src_words;

    dst_words = (volatile u32*)dst;
    src_words = (const volatile u32*)src;
    dst_words[0] = src_words[0];
    dst_words[1] = src_words[1];
    dst_words[2] = src_words[2];
    dst_words[3] = src_words[3];
    dst_words[4] = src_words[4];
    dst_words[5] = src_words[5];
    dst_words[6] = src_words[6];
    dst_words[7] = src_words[7];
    dst_words[8] = src_words[8];
    dst_words[9] = src_words[9];

    center.vx = (*(volatile s16*)(dst + 2) + *(volatile s16*)(dst + 8) +
                 *(volatile s16*)(dst + 14) + *(volatile s16*)(dst + 20)) >>
                2;
    center.vy = (*(volatile s16*)(dst + 4) + *(volatile s16*)(dst + 10) +
                 *(volatile s16*)(dst + 16) + *(volatile s16*)(dst + 22)) >>
                2;
    center.vz = (*(volatile s16*)(dst + 6) + *(volatile s16*)(dst + 12) +
                 *(volatile s16*)(dst + 18) + *(volatile s16*)(dst + 24)) >>
                2;
    center.pad = 0;

    state[0] = (s16)center.vx;
    state[1] = (s16)center.vy;
    state[2] = (s16)center.vz;
    VectorNormalS(&center, (SVECTOR*)((u8*)state + 8));
    *(volatile s16*)((u8*)state + 0x10) = 0;
    *(volatile s16*)((u8*)state + 0x12) = 0;
    *(volatile s16*)((u8*)state + 0x14) = 0;
    state[4] = (s16)(s8)state[4];
    state[5] = (s16)(s8)state[5];
    state[6] = (s16)(s8)state[6];

    *(volatile s16*)(dst + 2) -= state[0];
    *(volatile s16*)(dst + 4) -= state[1];
    *(volatile s16*)(dst + 6) -= state[2];
    *(volatile s16*)(dst + 8) -= state[0];
    *(volatile s16*)(dst + 10) -= state[1];
    *(volatile s16*)(dst + 12) -= state[2];
    *(volatile s16*)(dst + 14) -= state[0];
    *(volatile s16*)(dst + 16) -= state[1];
    *(volatile s16*)(dst + 18) -= state[2];
    *(volatile s16*)(dst + 20) -= state[0];
    *(volatile s16*)(dst + 22) -= state[1];
    *(volatile s16*)(dst + 24) -= state[2];

    dst += 0x28;
    src += 0x28;
    state += 0x0c;
    i += 1u;
  } while (i < 0x1bu);
}
