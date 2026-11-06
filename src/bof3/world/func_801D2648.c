#include "bof3/world/area03004_internal.h"

/* @source 0x801D2648
 * @behavior AREA030 work-record step handler, entry 4 of the local work-record
 * step table 0x801E1D20 dispatched through scratch byte 1: while the record
 * timer byte 0x0A is nonzero it counts that byte down and, once it reaches zero
 * on the same pass, reloads it from byte 0x10 of the shared limit record
 * D_801E3210 when that record's signed byte 0x0F sits below the record byte
 * 0x07, which it then steps back by one. It then integrates the step words
 * 0x0C/0x10 into the record positions 0x34/0x38, adds the accumulator pair
 * 0x14/0x20 and accumulates that sum into word 0x3C; while that total stays
 * positive with an empty 0x20 accumulator it either restarts the climb
 * (0x14 = 0x40000, 0x20 = -0x8000) and arms the shared 0x80143FC8 entry record
 * picked by func_8019601c with flags 1 and byte 0x05 10 when the record flag
 * byte 0x0B is set, or clears the 0x3C total otherwise. On the exhausted path
 * it drops the 0x20 accumulator once the total has gone negative and publishes
 * whether the total is still positive into record byte 0x0B. It then clears
 * record byte 0x5C, gates bit 5 of record byte 0 from the signed counter 0x3E
 * (clearing byte 0x5D on the live counter), mirrors byte 0x5D into 0x5E and
 * 0x5F, requests the frontend state selected by the sign of word 0x10 (2 when
 * positive, 3 at zero, 0 when negative) through func_8014D8D4, runs
 * func_8014D978, stores 2 in record byte 0x48 and finishes with the per-record
 * scroll updater func_801E084C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D2648(void) {
  u8* work;
  u8* timer;
  u8* entry;
  u8* limit;
  u8* rec;
  u8* state;
  u8* flags;
  u8* level;
  u8* reset;
  u8* mirrorA;
  u8* mirrorB;
  u8* motion;
  u8* tail;
  s32 total;
  s32 word;
  u8 pick;

  work = (u8*)D_1F800044;
  if (work[0x0Au] != 0u) {
    work[0x0Au] = (u8)(work[0x0Au] - 1u);
    timer = (u8*)D_1F800044;
    if (timer[0x0Au] == 0u) {
      limit = D_801E3210;
      if (((s8*)limit)[0x0Fu] < timer[7u]) {
        timer[0x0Au] = limit[0x10u];
        entry = (u8*)D_1F800044;
        entry[7u] = (u8)(entry[7u] - 1u);
      }
    }
  }

  rec = (u8*)D_1F800044;
  *(s32*)(rec + 0x34u) = *(s32*)(rec + 0x34u) + *(s32*)(rec + 0x0Cu);
  *(s32*)(rec + 0x38u) = *(s32*)(rec + 0x38u) + *(s32*)(rec + 0x10u);
  total = *(s32*)(rec + 0x3Cu) +
          (*(s32*)(rec + 0x14u) = *(s32*)(rec + 0x14u) + *(s32*)(rec + 0x20u));
  *(s32*)(rec + 0x3Cu) = total;
  if ((total > 0) && (*(s32*)(rec + 0x20u) == 0)) {
    if (rec[0x0Bu] == 0u) {
      *(s32*)(rec + 0x3Cu) = 0;
    } else {
      *(s32*)(rec + 0x14u) = 0x40000;
      *(s32*)(rec + 0x20u) = -0x8000;
      pick = (u8)func_8019601c();
      if (pick != 0xFFu) {
        D_80143FC8[pick].flags_00 = 1u;
        D_80143FC8[pick].byte_05 = 10u;
      }
    }
  } else {
    state = (u8*)D_1F800044;
    if ((*(s32*)(state + 0x3Cu) < 0) && (*(s32*)(state + 0x20u) != 0)) {
      *(s32*)(state + 0x20u) = 0;
      state[0x0Bu] = 0u;
    } else if (*(s32*)(state + 0x3Cu) > 0) {
      state[0x0Bu] = 1u;
    } else {
      state[0x0Bu] = 0u;
    }
  }

  flags = (u8*)D_1F800044;
  flags[0x5Cu] = 0u;
  level = (u8*)D_1F800044;
  if (*(s16*)(level + 0x3Eu) > 0) {
    level[0u] = (u8)(level[0u] & 0xDFu);
    reset = (u8*)D_1F800044;
    reset[0x5Du] = 0u;
  } else {
    level[0u] = (u8)(level[0u] | 0x20u);
  }
  mirrorA = (u8*)D_1F800044;
  mirrorA[0x5Eu] = mirrorA[0x5Du];
  mirrorB = (u8*)D_1F800044;
  mirrorB[0x5Fu] = mirrorB[0x5Du];
  motion = (u8*)D_1F800044;
  word = *(s32*)(motion + 0x10u);
  if (word > 0) {
    func_8014D8D4(2);
  } else if (word < 0) {
    func_8014D8D4(0);
  } else {
    func_8014D8D4(3);
  }
  func_8014D978();
  tail = (u8*)D_1F800044;
  tail[0x48u] = 2u;
  func_801E084C();
}
