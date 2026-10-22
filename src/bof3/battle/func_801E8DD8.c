#include "bof3/battle/battle03_internal.h"

/* @source 0x801E8DD8
 * @behavior Derives the rendering position of the queued work record published
 * at 0x801EB4E0 into the scratch work halfwords: for the first three records the
 * x delta is the signed byte at D_801EB0B0 indexed by record byte +0x08 * 2 plus
 * the +0x79 byte of the 0x140-stride local work record * 8, the y delta is
 * D_801EB108[byte79 * 2] while record byte +0x08 is below two and
 * D_801EB109[byte79 * 2] otherwise, and record halfwords +0x2E/+0x30 become
 * scratch +0x2E/+0x30 with those deltas applied (+0x30 also minus 8) while +0x32
 * is copied across. Enemy records take the x delta from the signed byte at
 * D_801EB712 and the y delta from the class table D_800E40CB row selected by the
 * D_801EB710 class byte of the same 0x118-stride record.
 * @status partial
 * @match 88.79
 * @residual Live asm diff 103/116 instructions, 464 versus 464 bytes. The frame,
 * the instruction count, the two position blocks, the reload asymmetry and the
 * whole body reproduce; the residual is register naming. The two y-delta arms
 * compute the 0x140-stride record base in v1 where the original uses v0 (the
 * original keeps the byte-79 index in v0 and the table byte in v1), and the
 * enemy class-byte read for the y delta does the same with its reduced index.
 * The class-table read emits `addu at,at,v0` where the original emits
 * `addu at,v0,at`.
 * Rungs 4 and 5 are unauthorized in this lane.
 */
void func_801E8DD8(void) {
  Battle03LocalWork* work;
  s8                 dx;
  u8                 dy;
  u16*               scratch;
  s32                enemy;

  work = (Battle03LocalWork*)(void*)D_801EB4E0;
  if (work->unk_05 < 3u) {
    dx = *(s8*)&D_801EB0B0[D_80145E90[work->unk_05].unk_79 * 8 +
                           work->unk_08 * 2];
    scratch = (u16*)(void*)D_1F800044;
    *(u16*)((u8*)scratch + 0x2e) = *(u16*)((u8*)work + 0x2e) + dx;
    if (work->unk_08 < 2u) {
      dy = D_801EB108[D_80145E90[work->unk_05].unk_79 * 2];
    } else {
      dy = D_801EB109[D_80145E90[work->unk_05].unk_79 * 2];
    }
    *(u16*)((u8*)scratch + 0x30) = *(u16*)((u8*)work + 0x30) - dy - 8u;
    *(u16*)((u8*)D_1F800044 + 0x32) = *(u16*)((u8*)D_801EB4E0 + 0x32);

  } else {
    enemy = (s32)work->unk_05 - 3;
    dx = *(s8*)&D_801EB712[enemy * 0x118];
    *(u16*)((u8*)D_1F800044 + 0x2e) = *(u16*)((u8*)work + 0x2e) + dx;
    enemy = (s32)work->unk_05 - 3;
    dy = D_800E40CB[D_801EB710[enemy * 0x118] * 0x88];
    *(u16*)((u8*)D_1F800044 + 0x30) = *(u16*)((u8*)work + 0x30) - dy - 8u;
    *(u16*)((u8*)D_1F800044 + 0x32) = *(u16*)((u8*)work + 0x32);
  }
}
