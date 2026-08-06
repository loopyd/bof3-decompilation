#include "internal.h"

/* PsyQ 4.7 libsnd.h official prototypes (declared directly: libsnd.h
 * conflicts with reviewed declarations in symbols/functions.h). */
extern short SsUtKeyOnV(short voice, short vabId, short prog, short tone,
                        short note, short fine, short voll, short volr);
extern short SsUtSetDetVVol(short, short, short);

/* @behavior Sound cue dispatcher: runs a per-cue handler, eases channel
 * volumes while a fade is active, and keys on/upvolumes cue channels via
 * PsyQ SsUtKeyOnV/SsUtSetDetVVol.
 * @source 0x8015DF18
 */
void sound_dispatch_cue(u32 cue_id) {
  /* MATCHING_AID: original keeps cue_id in a0 through the pre-call
   * computations and copies it to s0 in the jalr delay slot
   * (`jalr v0; move s0,a0`); clean C copied a0 to s0 at function entry and
   * read s0 before the call. The pinned local assigned just before the call
   * reproduces the original copy placement. Exhausted rungs: declarations,
   * statement order, temporaries, compiler-profile search, two bounded
   * permuter runs. Remove if a clean-C shape reproduces the delay-slot copy.
   * Immediately following bin/byte-match was exact (2684 bytes). */
  REGISTER_PIN(u32, saved, "s0");
  s32 hi;
  s32 lo8;
  s32 lo;
  s16 result;
  s32 shifted;
  /* MATCHING_AID: original computes the cue-family mask as `andi v0,s0,0xf000`
   * (duplicated into two jump delay slots) followed by `bnez v0`; with an
   * unpinned local GCC tied the andi destination to the dead pinned s0
   * (`andi s0,s0,0xf000`) and could not fill the delay slots. The v0 pin
   * restores the original destination and delay-slot duplication. Remove if
   * the allocator stops tying the mask temp to s0. Immediately following
   * bin/byte-match was exact. */
  REGISTER_PIN(s32, top, "v0");
  /* MATCHING_AID: in each volume-ease second half the original places the
   * `mult step,target` product in a0 (the dead target register:
   * `mult v0,a0; mflo a0`) and the shifted result in v0; clean C put the
   * product in v1 (shared local) or v0 with the shifted result in a0. The
   * a0 pin on the second-product local restores the original register web in
   * all four channel blocks. Remove if a clean temp arrangement reproduces
   * `mflo a0`/`sra v0,a0,7`. Immediately following bin/byte-match was exact.
   */
  REGISTER_PIN(s32, lo2, "a0");

  hi = (cue_id & 0xF00) >> 8;
  lo8 = cue_id & 0xFF;
  D_8018B408 = (s16)hi;
  D_8018B404 = (s16)lo8;
  saved = cue_id;
  D_8018232C[hi](cue_id);

  if (D_8018B3F4 == 0x80) {
    switch (D_8018B3E4) {
    case 3:
      lo = D_8018B39C * (0x80 - D_8018B3A4);
      if (lo < 0) {
        lo += 0x7F;
      }
      D_8018B39C = (s16)(lo >> 7);
      lo2 = D_8018B3A0 * D_8018B3A4;
      shifted = lo2 >> 7;
      if (lo2 < 0) {
        lo2 += 0x7F;
        shifted = lo2 >> 7;
      }
      D_8018B3A0 = (s16)shifted;
      /* fallthrough */
    case 2:
      lo = D_8018B374 * (0x80 - D_8018B37C);
      if (lo < 0) {
        lo += 0x7F;
      }
      D_8018B374 = (s16)(lo >> 7);
      lo2 = D_8018B378 * D_8018B37C;
      shifted = lo2 >> 7;
      if (lo2 < 0) {
        lo2 += 0x7F;
        shifted = lo2 >> 7;
      }
      D_8018B378 = (s16)shifted;
      /* fallthrough */
    case 1:
      lo = D_8018B34C * (0x80 - D_8018B354);
      if (lo < 0) {
        lo += 0x7F;
      }
      D_8018B34C = (s16)(lo >> 7);
      lo2 = D_8018B350 * D_8018B354;
      shifted = lo2 >> 7;
      if (lo2 < 0) {
        lo2 += 0x7F;
        shifted = lo2 >> 7;
      }
      D_8018B350 = (s16)shifted;
      /* fallthrough */
    case 0:
      lo = D_8018B324 * (0x80 - D_8018B32C);
      if (lo < 0) {
        lo += 0x7F;
      }
      D_8018B324 = (s16)(lo >> 7);
      lo2 = D_8018B328 * D_8018B32C;
      shifted = lo2 >> 7;
      if (lo2 < 0) {
        lo2 += 0x7F;
        shifted = lo2 >> 7;
      }
      D_8018B328 = (s16)shifted;
    }
    top = saved & 0xF000;
    if (top == 0) {
      D_8018B3D8 = 0x17FF;
      D_8018B3DC = 0x17FF;
    }
  }

  if (D_8018232A == 0) {
    switch (D_8018B3E4) {
    case 3:
      result = SsUtKeyOnV(D_8018B390, D_8018B30C, D_8018B380,
                          D_8018B388, D_8018B38C,
                          D_8018B398, D_8018B39C,
                          D_8018B3A0);
      D_8018B394 = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 2:
      result = SsUtKeyOnV(D_8018B368, D_8018B30C, D_8018B358,
                          D_8018B360, D_8018B364,
                          D_8018B370, D_8018B374,
                          D_8018B378);
      D_8018B36C = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 1:
      result = SsUtKeyOnV(D_8018B340, D_8018B30C, D_8018B330,
                          D_8018B338, D_8018B33C,
                          D_8018B348, D_8018B34C,
                          D_8018B350);
      D_8018B344 = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 0:
      result = SsUtKeyOnV(D_8018B318, D_8018B30C, D_8018B308,
                          D_8018B310, D_8018B314,
                          D_8018B320, D_8018B324,
                          D_8018B328);
      D_8018B31C = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      D_8018232A = -1;
      D_8018B3F0 = (u16)D_8018B318;
      D_8018B3EC = D_8018B3E8;
      return;
    }
    return;
  }

  if ((s16)D_8018B3F0 != D_8018B318) {
    switch (D_8018B3E4) {
    case 3:
      result = SsUtKeyOnV(D_8018B390, D_8018B30C, D_8018B380,
                          D_8018B388, D_8018B38C,
                          D_8018B398, D_8018B39C,
                          D_8018B3A0);
      D_8018B394 = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 2:
      result = SsUtKeyOnV(D_8018B368, D_8018B30C, D_8018B358,
                          D_8018B360, D_8018B364,
                          D_8018B370, D_8018B374,
                          D_8018B378);
      D_8018B36C = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 1:
      result = SsUtKeyOnV(D_8018B340, D_8018B30C, D_8018B330,
                          D_8018B338, D_8018B33C,
                          D_8018B348, D_8018B34C,
                          D_8018B350);
      D_8018B344 = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 0:
      result = SsUtKeyOnV(D_8018B318, D_8018B30C, D_8018B308,
                          D_8018B310, D_8018B314,
                          D_8018B320, D_8018B324,
                          D_8018B328);
      D_8018B31C = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      D_8018B3F0 = (u16)D_8018B318;
      D_8018B3EC = D_8018B3E8;
    }
    return;
  }

  if ((s16)D_8018B3EC <= (s16)D_8018B3E8) {
    switch (D_8018B3E4) {
    case 3:
      result = SsUtKeyOnV(D_8018B390, D_8018B30C, D_8018B380,
                          D_8018B388, D_8018B38C,
                          D_8018B398, D_8018B39C,
                          D_8018B3A0);
      D_8018B394 = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 2:
      result = SsUtKeyOnV(D_8018B368, D_8018B30C, D_8018B358,
                          D_8018B360, D_8018B364,
                          D_8018B370, D_8018B374,
                          D_8018B378);
      D_8018B36C = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 1:
      result = SsUtKeyOnV(D_8018B340, D_8018B30C, D_8018B330,
                          D_8018B338, D_8018B33C,
                          D_8018B348, D_8018B34C,
                          D_8018B350);
      D_8018B344 = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 0:
      result = SsUtKeyOnV(D_8018B318, D_8018B30C, D_8018B308,
                          D_8018B310, D_8018B314,
                          D_8018B320, D_8018B324,
                          D_8018B328);
      D_8018B31C = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3DC, D_8018B3DC);
      }
      D_8018B3F0 = (u16)D_8018B318;
      D_8018B3EC = D_8018B3E8;
    }
    return;
  }

  if (D_8018E140[(s16)D_8018B3F0] == 0) {
    switch (D_8018B3E4) {
    case 3:
      result = SsUtKeyOnV(D_8018B390, D_8018B30C, D_8018B380,
                          D_8018B388, D_8018B38C,
                          D_8018B398, D_8018B39C,
                          D_8018B3A0);
      D_8018B394 = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 2:
      result = SsUtKeyOnV(D_8018B368, D_8018B30C, D_8018B358,
                          D_8018B360, D_8018B364,
                          D_8018B370, D_8018B374,
                          D_8018B378);
      D_8018B36C = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 1:
      result = SsUtKeyOnV(D_8018B340, D_8018B30C, D_8018B330,
                          D_8018B338, D_8018B33C,
                          D_8018B348, D_8018B34C,
                          D_8018B350);
      D_8018B344 = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      /* fallthrough */
    case 0:
      result = SsUtKeyOnV(D_8018B318, D_8018B30C, D_8018B308,
                          D_8018B310, D_8018B314,
                          D_8018B320, D_8018B324,
                          D_8018B328);
      D_8018B31C = result;
      if (D_8018B3F4 == 0x80) {
        SsUtSetDetVVol(result, D_8018B3D8, D_8018B3DC);
      }
      D_8018B3F0 = (u16)D_8018B318;
      D_8018B3EC = D_8018B3E8;
    }
  }
}
